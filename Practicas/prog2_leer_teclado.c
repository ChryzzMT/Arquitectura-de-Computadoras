#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

#define BDA_FILE "/tmp/bios_keyboard_bda.bin"

#pragma pack(push, 1)
typedef struct {
    uint16_t head;          /* Offset 0x001A: read pointer */
    uint16_t tail;          /* Offset 0x001C: write pointer */
    uint16_t buffer[16];    /* Offset 0x001E-0x003D: 16 words */
    uint8_t  port_60_data;  /* Port 0x60 data register */
    uint8_t  port_64_status;/* Port 0x64 status register */
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

void lectura_con_scanf(void) {
    char palabra[100];
    printf("\n--- [OPTION 1: READ WITH scanf()] ---\n");
    printf(">> Enter or wait for input: ");
    if (scanf("%99s", palabra) == 1) {
        printf("\n======================================================\n");
        printf("[scanf RESULT]: \"%s\"\n", palabra);
        printf("======================================================\n");
    }
}

void lectura_con_getc(void) {
    int c;
    printf("\n--- [OPTION 2: READ WITH getc(stdin)] ---\n");
    printf(">> Enter or wait for a character: ");
    c = getc(stdin);
    if (c != EOF) {
        printf("\n======================================================\n");
        printf("[getc RESULT]: Character '%c' | ASCII: %d (0x%02X)\n", c, c, (uint8_t)c);
        printf("======================================================\n");
    }
}

void lectura_desde_puerto(void) {
    printf("\n--- [OPTION 3: READ FROM HARDWARE PORT (0x60)] ---\n");
    printf("Physical x86 instruction: in al, 60h\n");
    BiosKeyboardMemory bda;
    cargar_bda(&bda);

    printf(">> Port 0x64 status: 0x%02X\n", bda.port_64_status);
    if ((bda.port_64_status & 0x01) == 0 && bda.port_60_data == 0) {
        printf(">> [PORT 0x60] No pending data on the port.\n");
    } else {
        uint8_t scan_code = bda.port_60_data;
        bda.port_64_status &= ~0x01;
        guardar_bda(&bda);
        printf("======================================================\n");
        printf("[PORT 0x60 RESULT]: Scan-code = 0x%02X\n", scan_code);
        printf("======================================================\n");
    }
}

void lectura_directa_memoria(void) {
    printf("\n--- [OPTION 4: DIRECT READ FROM RAM (BDA)] ---\n");
    BiosKeyboardMemory bda;
    cargar_bda(&bda);

    printf(">> HEAD pointer: 0x%04X | TAIL pointer: 0x%04X\n", bda.head, bda.tail);
    if (bda.head == bda.tail) {
        printf(">> [RAM] Buffer empty in memory (HEAD == TAIL).\n");
        return;
    }

    int idx = (bda.head - 0x001E) / 2;
    uint16_t palabra = bda.buffer[idx];
    uint8_t ascii = (uint8_t)(palabra & 0xFF);
    uint8_t scan = (uint8_t)(palabra >> 8);

    bda.head = (bda.head >= 0x003C) ? 0x001E : bda.head + 2;
    guardar_bda(&bda);

    printf("======================================================\n");
    printf("[DIRECT RAM RESULT]:\n");
    printf("  16-bit word in cell: 0x%04X\n", palabra);
    printf("  -> Low byte  (ASCII)    : '%c' (0x%02X)\n", ascii, ascii);
    printf("  -> High byte (Scan-Code): 0x%02X\n", scan);
    printf("  -> New HEAD pointer     : 0x%04X\n", bda.head);
    printf("======================================================\n");
}

void lectura_con_int16(void) {
    printf("\n--- [OPTION 5: WITH INT 16h (SERVICE 00h IN ASM)] ---\n");
    printf("Physical x86 instruction: mov ah, 00h; int 16h\n");
    BiosKeyboardMemory bda;
    cargar_bda(&bda);

    if (bda.head == bda.tail) {
        printf(">> [BIOS INT 16h] Buffer currently empty.\n");
        return;
    }

    int idx = (bda.head - 0x001E) / 2;
    uint16_t palabra = bda.buffer[idx];
    uint8_t ascii = (uint8_t)(palabra & 0xFF);
    uint8_t scan = (uint8_t)(palabra >> 8);

    bda.head = (bda.head >= 0x003C) ? 0x001E : bda.head + 2;
    guardar_bda(&bda);

    printf("======================================================\n");
    printf(" [CPU REGISTERS AFTER INT 16h]:\n");
    printf("   Register AL (ASCII code) : '%c' (0x%02X)\n", ascii, ascii);
    printf("   Register AH (Scan-code)  : 0x%02X\n", scan);
    printf("   Full register AX         : 0x%04X\n", palabra);
    printf("======================================================\n");
}

int main(void) {
    int opcion;
    char c;

    printf("====================================================================\n");
    printf("   PROGRAM 2: KEYBOARD / QUEUE / MEMORY / PORT READER\n");
    printf("====================================================================\n");
    printf("Select the read method:\n");
    printf("  [1] WITH scanf() (formatted word from stdin)\n");
    printf("  [2] WITH getc() (character by character from stdin)\n");
    printf("  [3] READING FROM THE PORT (Port 0x60 / IN AL, 60h)\n");
    printf("  [4] READING DIRECTLY FROM MEMORY (BDA segment 0x0040:0x001A)\n");
    printf("  [5] WITH INT 16h (BIOS service 00h in ASM)\n");
    printf("Enter option (1-5): ");

    if (scanf("%d", &opcion) != 1) return 1;
    while ((c = getchar()) != '\n' && c != EOF);

    switch (opcion) {
        case 1: lectura_con_scanf(); break;
        case 2: lectura_con_getc(); break;
        case 3: lectura_desde_puerto(); break;
        case 4: lectura_directa_memoria(); break;
        case 5: lectura_con_int16(); break;
        default: printf("Invalid option.\n"); return 1;
    }
    return 0;
}
