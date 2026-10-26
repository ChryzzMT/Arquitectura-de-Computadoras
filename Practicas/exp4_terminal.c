/*
 * Lab: Port I/O keyboard + circular FIFO queue + VRAM terminal
 * C with GCC inline assembly (x86-64, AT&T syntax).
 *
 * The lab machine (RAM, ports, VRAM) is simulated with arrays.
 * Core operations (memory access, compare, modular pointer update)
 * are written in inline assembly.
 *
 * Build: gcc -O1 -Wall -o exp4_terminal exp4_terminal.c
 * Run:   ./exp4_terminal            (live: type keys, Ctrl-D quits)
 *        printf 'HOLA\bX' | ./exp4_terminal [burst] [-v]
 *   burst: keys polled per main-loop pass (default 1). Values > 8 can
 *          overflow the queue and demonstrate discarding.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

/* ---- Memory map ---- */
#define KB_QUEUE   0x70
#define KB_HEAD    0x78
#define KB_TAIL    0x79
#define KB_COUNT   0x7A
#define CURSOR     0x60
#define VRAM_BASE  0xF0
#define PORT_STATUS 0x01
#define PORT_DATA   0x02
#define ASCII_BS    0x08
#define ASCII_SPACE 0x20

static uint8_t mem[256];       /* RAM + VRAM address space (0x00-0xFF) */
static uint8_t io_ports[256];  /* isolated I/O address space           */
static int input_eof = 0;
static int verbose = 0;
static unsigned discarded = 0;
static int live = 0;                 /* 1 when stdin is a terminal */
static struct termios old_tio;
static void restore_tty(void) { if (live) tcsetattr(0, TCSANOW, &old_tio); }

/* ------------------------------------------------------------------
 * Inline-assembly primitives
 * ------------------------------------------------------------------ */

/* MEM_READ: value = mem[addr] */
static inline uint8_t mem_read(uint8_t addr) {
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0"
                     : "=q"(v)
                     : "r"(mem), "r"((uint64_t)addr)
                     : "memory");
    return v;
}

/* MEM_WRITE: mem[addr] = value */
static inline void mem_write(uint8_t addr, uint8_t v) {
    __asm__ volatile("movb %0, (%1,%2)"
                     :
                     : "q"(v), "r"(mem), "r"((uint64_t)addr)
                     : "memory");
}

/* IO_READ: isolated I/O, never touches mem[]. Reading the data port
 * clears the status port (hardware behaviour described in the lab). */
static inline uint8_t io_in(uint8_t port) {
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0"
                     : "=q"(v)
                     : "r"(io_ports), "r"((uint64_t)port)
                     : "memory");
    if (port == PORT_DATA)
        io_ports[PORT_STATUS] = 0;
    return v;
}

/* CMP a,b -> 1 if equal */
static inline int cmp_eq(uint8_t a, uint8_t b) {
    uint8_t r;
    __asm__("cmpb %2, %1\n\t"
            "sete %0"
            : "=q"(r)
            : "q"(a), "q"(b)
            : "cc");
    return r;
}

/* (v + 1) mod 8 */
static inline uint8_t inc_mod8(uint8_t v) {
    __asm__("incb %0\n\t"
            "andb $7, %0"
            : "+q"(v)
            :
            : "cc");
    return v;
}

/* v + 1 / v - 1 on 8-bit registers (wraps at 0xFF <-> 0x00) */
static inline uint8_t inc8(uint8_t v) { __asm__("incb %0" : "+q"(v) : : "cc"); return v; }
static inline uint8_t dec8(uint8_t v) { __asm__("decb %0" : "+q"(v) : : "cc"); return v; }

/* ------------------------------------------------------------------
 * Simulated keyboard hardware (reads stdin)
 * ------------------------------------------------------------------ */
static void hw_keyboard_tick(void) {
    if (io_ports[PORT_STATUS]) return;      /* previous key not read yet */
    int c;
    do { c = getchar(); } while (c == '\n' || c == '\r');
    if (c == EOF) { input_eof = 1; return; }
    if (live && c == 4) { input_eof = 1; return; }   /* Ctrl-D quits */
    if (c == 127) c = ASCII_BS;
    io_ports[PORT_DATA] = (uint8_t)c;
    io_ports[PORT_STATUS] = 1;
}

/* ------------------------------------------------------------------
 * Experiments 1 + 2: producer (port -> queue)
 * ------------------------------------------------------------------ */
static void poll_keyboard(void) {
    hw_keyboard_tick();
    if (cmp_eq(io_in(PORT_STATUS), 0)) return;   /* no key ready */

    uint8_t key   = io_in(PORT_DATA);            /* IN R2,[0x02] */
    uint8_t count = mem_read(KB_COUNT);
    if (cmp_eq(count, 8)) { discarded++; return; } /* overflow: discard */

    uint8_t tail = mem_read(KB_TAIL);
    mem_write((uint8_t)(KB_QUEUE + tail), key);  /* enqueue at 0x70+TAIL */
    mem_write(KB_TAIL, inc_mod8(tail));          /* TAIL=(TAIL+1) mod 8  */
    mem_write(KB_COUNT, inc8(count));            /* COUNT++              */
}

/* ------------------------------------------------------------------
 * Experiment 3: scroll one row up in VRAM
 * ------------------------------------------------------------------ */
static void scroll(void) {
    for (uint8_t a = VRAM_BASE; a < 0xFC; a = inc8(a))
        mem_write(a, mem_read((uint8_t)(a + 4)));
    for (uint8_t a = 0xFC; ; a = inc8(a)) {
        mem_write(a, ASCII_SPACE);
        if (cmp_eq(a, 0xFF)) break;
    }
}

/* ------------------------------------------------------------------
 * Consumer (queue -> VRAM) with Backspace support
 * ------------------------------------------------------------------ */
static void process_queue(void) {
    uint8_t count = mem_read(KB_COUNT);
    if (cmp_eq(count, 0)) return;                /* queue empty */

    uint8_t head = mem_read(KB_HEAD);
    uint8_t ch   = mem_read((uint8_t)(KB_QUEUE + head)); /* dequeue */
    mem_write(KB_HEAD, inc_mod8(head));          /* HEAD=(HEAD+1) mod 8 */
    mem_write(KB_COUNT, dec8(count));            /* COUNT--             */

    uint8_t cur = mem_read(CURSOR);
    if (cmp_eq(ch, ASCII_BS)) {
        if (cur > VRAM_BASE) {                   /* cursor > 0xF0 */
            cur = dec8(cur);
            mem_write(cur, ASCII_SPACE);         /* erase visually */
        }
    } else {
        mem_write(cur, ch);                      /* echo */
        cur = inc8(cur);
        if (cmp_eq(cur, 0)) {                    /* ran past 0xFF */
            scroll();
            cur = 0xFC;
        }
    }
    mem_write(CURSOR, cur);
}

static void render(void) {
    puts("+----+");
    for (int r = 0; r < 4; r++) {
        putchar('|');
        for (int c = 0; c < 4; c++) {
            uint8_t ch = mem_read((uint8_t)(VRAM_BASE + r * 4 + c));
            putchar(ch >= 0x20 && ch < 0x7F ? ch : ' ');
        }
        puts("|");
    }
    puts("+----+");
}

int main(int argc, char **argv) {
    int burst = 1;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) verbose = 1;
        else burst = atoi(argv[i]) > 0 ? atoi(argv[i]) : 1;
    }

    if (isatty(0)) {                 /* live mode: key-by-key, no Enter */
        live = 1;
        tcgetattr(0, &old_tio);
        struct termios raw = old_tio;
        raw.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(0, TCSANOW, &raw);
        atexit(restore_tty);
        printf("Live terminal: type keys, Backspace works, Ctrl-D to quit.\n");
    }

    /* INIT */
    memset(mem, 0, sizeof mem);
    memset(io_ports, 0, sizeof io_ports);
    for (int a = VRAM_BASE; a <= 0xFF; a++) mem[a] = ASCII_SPACE;
    mem_write(CURSOR, VRAM_BASE);

    /* MAIN LOOP */
    while (!input_eof || mem_read(KB_COUNT) > 0) {
        for (int i = 0; i < burst; i++) poll_keyboard();
        process_queue();
        if (live) { printf("\033[H\033[J"); render(); fflush(stdout); }
        if (verbose)
            fprintf(stderr, "HEAD=%u TAIL=%u COUNT=%u CURSOR=0x%02X\n",
                    mem[KB_HEAD], mem[KB_TAIL], mem[KB_COUNT], mem[CURSOR]);
    }

    if (!live) render();
    if (discarded) printf("Keys discarded (queue full): %u\n", discarded);
    return 0;
}
