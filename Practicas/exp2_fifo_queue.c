/*
 * Experiment 2 - Circular FIFO keyboard queue in RAM (0x70..0x77)
 * C with GCC inline assembly (x86-64, AT&T syntax).
 *
 * KB_HEAD=0x78  KB_TAIL=0x79  KB_COUNT=0x7A
 * Demonstrates: enqueue, dequeue, wrap-around, overflow (full) and
 * underflow (empty) with a scripted scenario.
 *
 * Build: gcc -O1 -Wall -o exp2_fifo_queue exp2_fifo_queue.c
 * Run:   ./exp2_fifo_queue
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define KB_QUEUE 0x70
#define KB_HEAD  0x78
#define KB_TAIL  0x79
#define KB_COUNT 0x7A

static uint8_t mem[256];

static inline uint8_t mem_read(uint8_t a) {
    uint8_t v;
    __asm__ volatile("movb (%1,%2), %0" : "=q"(v)
                     : "r"(mem), "r"((uint64_t)a) : "memory");
    return v;
}
static inline void mem_write(uint8_t a, uint8_t v) {
    __asm__ volatile("movb %0, (%1,%2)" : : "q"(v), "r"(mem),
                     "r"((uint64_t)a) : "memory");
}
static inline int cmp_eq(uint8_t a, uint8_t b) {
    uint8_t r;
    __asm__("cmpb %2, %1\n\tsete %0" : "=q"(r) : "q"(a), "q"(b) : "cc");
    return r;
}
static inline uint8_t inc_mod8(uint8_t v) {   /* (v+1) mod 8 */
    __asm__("incb %0\n\tandb $7, %0" : "+q"(v) : : "cc");
    return v;
}
static inline uint8_t inc8(uint8_t v) { __asm__("incb %0" : "+q"(v) : : "cc"); return v; }
static inline uint8_t dec8(uint8_t v) { __asm__("decb %0" : "+q"(v) : : "cc"); return v; }

/* Producer. Returns 1 on success, 0 if the queue is full (key discarded). */
static int enqueue(uint8_t key) {
    uint8_t count = mem_read(KB_COUNT);
    if (cmp_eq(count, 8)) return 0;
    uint8_t tail = mem_read(KB_TAIL);
    mem_write((uint8_t)(KB_QUEUE + tail), key);
    mem_write(KB_TAIL, inc_mod8(tail));
    mem_write(KB_COUNT, inc8(count));
    return 1;
}

/* Consumer. Returns 1 and sets *key, or 0 if the queue is empty. */
static int dequeue(uint8_t *key) {
    uint8_t count = mem_read(KB_COUNT);
    if (cmp_eq(count, 0)) return 0;
    uint8_t head = mem_read(KB_HEAD);
    *key = mem_read((uint8_t)(KB_QUEUE + head));
    mem_write(KB_HEAD, inc_mod8(head));
    mem_write(KB_COUNT, dec8(count));
    return 1;
}

static void dump(const char *what) {
    printf("%-26s HEAD=%u TAIL=%u COUNT=%u | RAM[0x70..0x77]=",
           what, mem[KB_HEAD], mem[KB_TAIL], mem[KB_COUNT]);
    for (int i = 0; i < 8; i++) {
        uint8_t c = mem[KB_QUEUE + i];
        if (c >= 0x20 && c < 0x7F) printf("%c ", c); else printf(". ");
    }
    printf("\n");
}

int main(void) {
    memset(mem, 0, sizeof mem);
    char what[64];
    uint8_t k;

    dump("initial");

    printf("\n-- 1) Fill beyond capacity: enqueue 'A'..'J' (10 keys, capacity 8) --\n");
    for (char c = 'A'; c <= 'J'; c++) {
        int ok = enqueue((uint8_t)c);
        snprintf(what, sizeof what, "enqueue '%c' %s", c, ok ? "ok" : "DISCARDED (full)");
        dump(what);
    }

    printf("\n-- 2) Dequeue 5 keys (FIFO order: A B C D E) --\n");
    for (int i = 0; i < 5; i++) {
        dequeue(&k);
        snprintf(what, sizeof what, "dequeue -> '%c'", k);
        dump(what);
    }

    printf("\n-- 3) Wrap-around: enqueue 'K','L','M' (TAIL goes 0 -> ...) --\n");
    for (char c = 'K'; c <= 'M'; c++) {
        enqueue((uint8_t)c);
        snprintf(what, sizeof what, "enqueue '%c'", c);
        dump(what);
    }

    printf("\n-- 4) Drain the queue and try one extra dequeue (empty) --\n");
    for (int i = 0; i < 9; i++) {
        if (dequeue(&k)) snprintf(what, sizeof what, "dequeue -> '%c'", k);
        else             snprintf(what, sizeof what, "dequeue -> EMPTY (no-op)");
        dump(what);
    }
    return 0;
}
