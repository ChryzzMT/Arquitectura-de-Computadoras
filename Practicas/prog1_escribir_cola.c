#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define BDA_FILE "/tmp/bios_keyboard_bda.bin"

#pragma pack(push, 1)
typedef struct {
    uint16_t head;          /* Offset 0x001A: read pointer */
    uint16_t tail;          /* Offset 0x001C: write pointer */
    uint16_t buffer[16];    /* Offset 0x001E-0x003D: scan-code (MSB) + ASCII (LSB) */
    uint8_t  port_60_data;  /* Port 0x60 data register */
    uint8_t  port_64_status;/* Port 0x64 status register (bit 0: key ready) */
} BiosKeyboardMemory;
#pragma pack(pop)

void cargar_bda(BiosKeyboardMemory *bda) {
    FILE *f = fopen(BDA_FILE, "rb");
    if (!f) {
        bda->head = 0x001E;
        bda->tail = 0x001E;
        memset(bda->buffer, 0, sizeof(bda->buffer));
        bda->port_60_data = 0;
        bda->port_64_status = 0;
    } else {
        if (fread(bda, sizeof(BiosKeyboardMemory), 1, f) != 1) {
            bda->head = 0x001E;
            bda->tail = 0x001E;
        }
        fclose(f);
    }
}

void guardar_bda(const BiosKeyboardMemory *bda) {
    FILE *f = fopen(BDA_FILE, "wb");
    if (f) {
        fwrite(bda, sizeof(BiosKeyboardMemory), 1, f);
        fclose(f);
    }
}

void metodo_inyectado(const char *texto) {
    printf("\n--- [OPTION 1: INJECTING INTO TTY INPUT QUEUE] ---\n");
#if defined(__unix__) || defined(__APPLE__)
    int fd = open("/dev/tty", O_RDWR);
    if (fd < 0) fd = STDIN_FILENO;
    int i = 0;
    while (texto[i] != '\0') {
        if (ioctl(fd, TIOCSTI, &texto[i]) < 0) {
            perror("ioctl TIOCSTI error");
            if (fd != STDIN_FILENO) close(fd);
            return;
        }
        i++;
    }
    if (fd != STDIN_FILENO) close(fd);
    printf(">> Success! %d characters injected into the terminal input queue.\n", i);
#else
    printf("TIOCSTI is only available on POSIX / Unix.\n");
#endif
}

void metodo_asm_int16(char ascii, uint8_t scan_code) {
    printf("\n--- [OPTION 2: ASM WITH INT 16h (SERVICE 05h)] ---\n");
    printf("Equivalent x86 assembly:\n");
    printf("  mov ah, 05h      ; AH = 05h (insert keystroke in buffer)\n");
    printf("  mov ch, 0x%02X    ; CH = Scan-code (hardware)\n", scan_code);
    printf("  mov cl, '%c' (0x%02X); CL = ASCII code\n", ascii, (uint8_t)ascii);
    printf("  int 16h          ; BIOS interrupt vector call\n");

    BiosKeyboardMemory bda;
    cargar_bda(&bda);

    uint16_t next_tail = (bda.tail >= 0x003C) ? 0x001E : bda.tail + 2;
    if (next_tail == bda.head) {
        printf(">> [BIOS INT 16h] AL = 1: BUFFER OVERFLOW. Buffer full.\n");
        return;
    }

    int idx = (bda.tail - 0x001E) / 2;
    bda.buffer[idx] = ((uint16_t)scan_code << 8) | (uint8_t)ascii;
    bda.tail = next_tail;
    bda.port_60_data = scan_code;
    bda.port_64_status |= 0x01;
    guardar_bda(&bda);

    printf(">> [BIOS INT 16h] Key '%c' inserted. AL = 0.\n", ascii);
}

void metodo_directo_memoria(char ascii, uint8_t scan_code) {
    printf("\n--- [OPTION 3: DIRECT TO RAM (BDA 0x0040:0x001A)] ---\n");
    BiosKeyboardMemory bda;
    cargar_bda(&bda);

    uint16_t next_tail = (bda.tail >= 0x003C) ? 0x001E : bda.tail + 2;
    if (next_tail == bda.head) {
        printf(">> [RAM ERROR] Circular buffer full in physical memory.\n");
        return;
    }

    int idx = (bda.tail - 0x001E) / 2;
    bda.buffer[idx] = ((uint16_t)scan_code << 8) | (uint8_t)ascii;
    bda.tail = next_tail;
    bda.port_60_data = scan_code;
    bda.port_64_status |= 0x01;
    guardar_bda(&bda);

    printf(">> [RAM BDA] Writing word 0x%04X at cell %d\n", bda.buffer[idx], idx);
    printf(">> [RAM BDA] New TAIL = 0x%04X | HEAD = 0x%04X\n", bda.tail, bda.head);
}

int main(void) {
    int opcion;
    char texto[100];
    char c;

    printf("====================================================================\n");
    printf("   PROGRAM 1: KEYBOARD QUEUE / MEMORY WRITER\n");
    printf("====================================================================\n");
    printf("Select the write method:\n");
    printf("  [1] INJECTED (OS input queue via ioctl TIOCSTI)\n");
    printf("  [2] ASM WITH INT 16h (BIOS x86 service 05h)\n");
    printf("  [3] DIRECT TO MEMORY (BIOS segment 0x0040:0x001A in RAM)\n");
    printf("Enter option (1-3): ");

    if (scanf("%d", &opcion) != 1) return 1;
    while ((c = getchar()) != '\n' && c != EOF);

    switch (opcion) {
        case 1:
            printf("\nEnter text to inject: ");
            if (fgets(texto, sizeof(texto) - 1, stdin)) {
                if (texto[strlen(texto) - 1] != '\n') strcat(texto, "\n");
                metodo_inyectado(texto);
            }
            break;
        case 2:
            printf("\nEnter a character to send via INT 16h: ");
            c = getchar();
            if (c != '\n' && c != EOF) {
                uint8_t sc = (c >= 'A' && c <= 'Z') ? (uint8_t)(0x1E + (c - 'A')) : 0x1E;
                metodo_asm_int16(c, sc);
            }
            break;
        case 3:
            printf("\nEnter a character to write directly to RAM: ");
            c = getchar();
            if (c != '\n' && c != EOF) {
                metodo_directo_memoria(c, 0x1E);
            }
            break;
        default:
            printf("Invalid option.\n");
            return 1;
    }
    return 0;
}
