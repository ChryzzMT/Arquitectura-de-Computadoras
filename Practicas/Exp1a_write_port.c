/*
 * Experiment 1 - VERSION A: WRITE to the port, read the keyboard with C.
 * C with GCC inline assembly (x86-64, AT&T syntax).
 *
 * The key is read from the real keyboard with a normal C function
 * (getchar by default, scanf with -DUSE_SCANF; cin is the C++ equivalent).
 * The program then acts as the keyboard controller: it WRITES the key into
 * the data port (0x02) and sets the status port (0x01) using OUT
 * (IO_WRITE line). Finally it checks the ports with IN and shows that RAM
 * address 0x02 is untouched (separate address spaces).
 *
 * Build: gcc -O1 -Wall -o exp1a_write_port exp1a_write_port.c
 *        gcc -O1 -Wall -DUSE_SCANF -o exp1a_write_port exp1a_write_port.c
 * Run:   printf 'HI' | ./exp1a_write_port      (or type keys + Ctrl-D)
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PORT_STATUS 0x01
#define PORT_DATA   0x02

static uint8_t mem[256];        /* RAM address space  */
static uint8_t io_ports[256];   /* port address space */

static void OUT(uint8_t port, uint8_t v) {              /* IO_WRITE */
    __asm__ volatile("movb %0, (%1,%2)" : : "q"(v), "r"(io_ports),
                     "r"((uint64_t)port) : "memory");
    printf("  BUS: IO_READ=0  MEM_READ=0  IO_WRITE=1 MEM_WRITE=0 | addr=0x%02X <- data=0x%02X\n", port, v);
}
static uint8_t IN(uint8_t port) {                       /* IO_READ */
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0" : "=q"(v)
                     : "r"(io_ports), "r"((uint64_t)port) : "memory");
    printf("  BUS: IO_READ=1  MEM_READ=0  IO_WRITE=0 MEM_WRITE=0 | addr=0x%02X -> data=0x%02X\n", port, v);
    if (port == PORT_DATA) io_ports[PORT_STATUS] = 0;   /* hw clears status */
    return v;
}
static uint8_t LOAD(uint8_t addr) {                     /* MEM_READ */
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0" : "=q"(v)
                     : "r"(mem), "r"((uint64_t)addr) : "memory");
    printf("  BUS: IO_READ=0  MEM_READ=1  IO_WRITE=0 MEM_WRITE=0 | addr=0x%02X -> data=0x%02X\n", addr, v);
    return v;
}
static void STORE(uint8_t addr, uint8_t v) {            /* MEM_WRITE */
    __asm__ volatile("movb %0, (%1,%2)" : : "q"(v), "r"(mem),
                     "r"((uint64_t)addr) : "memory");
    printf("  BUS: IO_READ=0  MEM_READ=0  IO_WRITE=0 MEM_WRITE=1 | addr=0x%02X <- data=0x%02X\n", addr, v);
}

/* Read ONE key with a standard C function */
static int read_key_c(void) {
#ifdef USE_SCANF
    char c;
    if (scanf(" %c", &c) != 1) return EOF;   /* skips whitespace/newlines */
    return (unsigned char)c;
#else
    int c;
    do { c = getchar(); } while (c == '\n' || c == '\r');
    return c;
#endif
}

int main(void) {
    memset(mem, 0, sizeof mem);
    memset(io_ports, 0, sizeof io_ports);

    printf("== Setup: RAM[0x02] holds a sentinel (0xAA), distinct from Port 0x02 ==\n");
    STORE(0x02, 0xAA);

    int n = 0, c;
    while ((c = read_key_c()) != EOF) {
        printf("\n== Key #%d read with %s: '%c' (0x%02X) ==\n", ++n,
#ifdef USE_SCANF
               "scanf",
#else
               "getchar",
#endif
               c >= 0x20 ? c : '.', c);

        printf("OUT [0x02],R2  ; write ASCII into the DATA port\n");
        OUT(PORT_DATA, (uint8_t)c);
        printf("OUT [0x01],1   ; write STATUS port: key available\n");
        OUT(PORT_STATUS, 1);

        printf("-- verify: read both ports back with IN --\n");
        printf("IN R1,[0x01]\n");
        IN(PORT_STATUS);
        printf("IN R2,[0x02]   ; hardware clears status\n");
        uint8_t k = IN(PORT_DATA);
        printf("  R2 = '%c'\n", k >= 0x20 ? k : '.');

        printf("LOAD R3,[0x02] ; same address in the MEMORY space\n");
        uint8_t ram = LOAD(0x02);
        printf("  RAM[0x02] = 0x%02X -> unchanged: IO_WRITE never reached RAM\n", ram);
    }
    if (n == 0) printf("\n(no keys; try: printf 'HI' | ./exp1a_write_port)\n");
    return 0;
}
