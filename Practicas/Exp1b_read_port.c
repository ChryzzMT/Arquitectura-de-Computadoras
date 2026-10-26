/*
 * Experiment 1 - VERSION B: type normally on the keyboard, READ the port with IN.
 * C with GCC inline assembly (x86-64, AT&T syntax).
 *
 * Shows the control-bus lines for every access:
 *   IN  / OUT  -> IO_READ / IO_WRITE   (only the keyboard responds)
 *   LOAD/STORE -> MEM_READ / MEM_WRITE (only RAM responds)
 * Address 0x02 exists in BOTH spaces with different content: no collision.
 *
 * Build: gcc -O1 -Wall -o exp1b_read_port exp1b_read_port.c
 * Run:   printf 'HI!' | ./exp1b_read_port
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PORT_STATUS 0x01
#define PORT_DATA   0x02
#define PORT_LEDS   0x10   /* extra output port to demo IO_WRITE */

static uint8_t mem[256];        /* RAM address space  */
static uint8_t io_ports[256];   /* port address space */

/* ---- bus-traced primitives (inline asm does the data transfer) ---- */
static uint8_t IN(uint8_t port) {                       /* IO_READ */
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0" : "=q"(v)
                     : "r"(io_ports), "r"((uint64_t)port) : "memory");
    printf("  BUS: IO_READ=1  MEM_READ=0  IO_WRITE=0 MEM_WRITE=0 | addr=0x%02X -> data=0x%02X\n", port, v);
    if (port == PORT_DATA) io_ports[PORT_STATUS] = 0;   /* hw clears status */
    return v;
}
static void OUT(uint8_t port, uint8_t v) {              /* IO_WRITE */
    __asm__ volatile("movb %0, (%1,%2)" : : "q"(v), "r"(io_ports),
                     "r"((uint64_t)port) : "memory");
    printf("  BUS: IO_READ=0  MEM_READ=0  IO_WRITE=1 MEM_WRITE=0 | addr=0x%02X <- data=0x%02X\n", port, v);
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
static int is_zero(uint8_t v) {                          /* CMP R1,0 ; JZ */
    uint8_t r;
    __asm__("cmpb $0, %1\n\tsete %0" : "=q"(r) : "q"(v) : "cc");
    return r;
}

/* Simulated keyboard hardware: presents the next stdin byte */
static int hw_key_available(void) {
    if (io_ports[PORT_STATUS]) return 1;
    int c = getchar();
    if (c == EOF) return 0;
    io_ports[PORT_DATA] = (uint8_t)c;
    io_ports[PORT_STATUS] = 1;
    return 1;
}

int main(void) {
    memset(mem, 0, sizeof mem);
    memset(io_ports, 0, sizeof io_ports);

    printf("== Setup: RAM[0x02] holds a sentinel (0xAA), distinct from Port 0x02 ==\n");
    STORE(0x02, 0xAA);

    int n = 0;
    while (hw_key_available()) {
        printf("\n== Key #%d: polling loop ==\n", ++n);
        printf("IN R1,[0x01]   ; status\n");
        uint8_t st = IN(PORT_STATUS);
        if (is_zero(st)) continue;                       /* JZ ESPERAR_TECLA */

        printf("IN R2,[0x02]   ; data -> hardware clears status\n");
        uint8_t key = IN(PORT_DATA);
        printf("  R2 = '%c' (0x%02X)\n", key >= 0x20 ? key : '.', key);

        printf("IN R1,[0x01]   ; status after reading data\n");
        IN(PORT_STATUS);

        printf("LOAD R3,[0x02] ; SAME address, memory space -> RAM answers\n");
        uint8_t ram = LOAD(0x02);
        printf("  RAM[0x02] = 0x%02X (untouched by the port read) -> no collision\n", ram);

        printf("OUT [0x10],R2  ; IO_WRITE demo (echo key to LED port)\n");
        OUT(PORT_LEDS, key);
    }
    if (n == 0) printf("\n(no keys given; try: printf 'HI!' | ./exp1b_read_port)\n");

    printf("\nANSWER: during IN R2,[0x02] the control bus has IO_READ=1 and "
           "MEM_READ=0.\nRAM only reacts to MEM_* lines, so it ignores the cycle; "
           "only the keyboard drives the data bus.\n");
    return 0;
}
