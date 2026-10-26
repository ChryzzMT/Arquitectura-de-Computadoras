/*
 * Experiment 3 - Video memory (VRAM 0xF0..0xFF, 4x4 characters)
 * C with GCC inline assembly (x86-64, AT&T syntax).
 *
 * Address formula:  addr = 0xF0 + (row * 4) + col
 * Demonstrates: 2D addressing, write, read-back/inspection and scroll.
 *
 * Build: gcc -O1 -Wall -o exp3_vram exp3_vram.c
 * Run:   ./exp3_vram
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define VRAM_BASE 0xF0
#define SPACE     0x20

static uint8_t mem[256];

static inline uint8_t mem_read(uint8_t a) {              /* LOAD  R,[a] */
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0" : "=q"(v)
                     : "r"(mem), "r"((uint64_t)a) : "memory");
    return v;
}
static inline void mem_write(uint8_t a, uint8_t v) {     /* STORE R,[a] */
    __asm__ volatile("movb %0, (%1,%2)" : : "q"(v), "r"(mem),
                     "r"((uint64_t)a) : "memory");
}
/* 0xF0 + (row << 2) + col  (row*4 done with a shift) */
static inline uint8_t vram_addr(uint8_t row, uint8_t col) {
    uint8_t r = row;
    __asm__("shlb $2, %0\n\taddb %1, %0\n\taddb $0xF0, %0"
            : "+q"(r) : "q"(col) : "cc");
    return r;
}

static void put_at(uint8_t row, uint8_t col, char ch) {
    uint8_t a = vram_addr(row, col);
    mem_write(a, (uint8_t)ch);
    printf("STORE '%c',[0x%02X]   ; (row %u, col %u) -> 0xF0 + %u*4 + %u\n",
           ch, a, row, col, row, col);
}
static uint8_t get_at(uint8_t row, uint8_t col) {
    uint8_t a = vram_addr(row, col);
    uint8_t v = mem_read(a);
    printf("LOAD  R,[0x%02X]    ; (row %u, col %u) -> '%c'\n",
           a, row, col, v >= 0x20 && v < 0x7F ? v : '?');
    return v;
}

static void render(const char *title) {
    printf("%s\n+----+\n", title);
    for (int r = 0; r < 4; r++) {
        putchar('|');
        for (int c = 0; c < 4; c++) {
            uint8_t ch = mem_read(vram_addr((uint8_t)r, (uint8_t)c));
            putchar(ch >= 0x20 && ch < 0x7F ? ch : ' ');
        }
        puts("|");
    }
    puts("+----+");
}

/* Scroll up one line: read the cell below, overwrite the cell above */
static void scroll_up(void) {
    for (uint8_t a = VRAM_BASE; a < 0xFC; a++) {
        uint8_t below = mem_read((uint8_t)(a + 4));      /* LOAD  R1,[a+4] */
        mem_write(a, below);                             /* STORE R1,[a]   */
    }
    for (uint8_t a = 0xFC; a >= 0xFC; a++) {             /* blank last row */
        mem_write(a, SPACE);
        if (a == 0xFF) break;
    }
}

int main(void) {
    memset(mem, 0, sizeof mem);
    for (int a = VRAM_BASE; a <= 0xFF; a++) mem[a] = SPACE;

    printf("== Write by coordinates ==\n");
    const char *l0 = "HOLA", *l1 = "RISC", *l2 = "VRAM", *l3 = "FIFO";
    for (uint8_t c = 0; c < 4; c++) put_at(0, c, l0[c]);
    for (uint8_t c = 0; c < 4; c++) put_at(1, c, l1[c]);
    for (uint8_t c = 0; c < 4; c++) put_at(2, c, l2[c]);
    for (uint8_t c = 0; c < 4; c++) put_at(3, c, l3[c]);
    printf("\nExample from the guide: (row 2, col 1) = 0xF0 + 8 + 1 = 0xF9\n");
    put_at(2, 1, 'X');
    printf("\n");
    render("Screen after writes:");

    printf("\n== Read back / inspect ==\n");
    uint8_t top_left = get_at(0, 0);
    uint8_t cell     = get_at(2, 1);
    printf("check: top-left=='H' -> %s, (2,1)=='X' -> %s\n",
           top_left == 'H' ? "OK" : "FAIL", cell == 'X' ? "OK" : "FAIL");

    printf("\n== Scroll up one line ==\n");
    scroll_up();
    render("Screen after scroll (row1->row0, row2->row1, row3->row2, row3 blank):");
    return 0;
}
