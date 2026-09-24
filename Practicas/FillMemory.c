#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLOCK_SIZE (1024 * 1024 * 100) // Allocate in 100 MB blocks

int main(void) {
    size_t total_allocated = 0;
    int block_count = 0;

    printf("=== RAM & Swap Stress Test Initiated ===\n");
    printf("Allocating memory in %d MB increments. Press Ctrl+C to abort.\n\n", BLOCK_SIZE / (1024 * 1024));

    while (1) {
        // Allocate a new memory block
        char *block = (char *)malloc(BLOCK_SIZE);
        if (block == NULL) {
            fprintf(stderr, "\n[!] Memory allocation failed at %zu MB. System limit reached.\n", 
                    total_allocated / (1024 * 1024));
            break;
        }

        // Force the OS to map physical RAM/Swap by writing to the block (prevents lazy allocation)
        memset(block, 0, BLOCK_SIZE);

        total_allocated += BLOCK_SIZE;
        block_count++;

        printf("Allocated block %d | Total Memory: %zu MB\n", block_count, total_allocated / (1024 * 1024));

        // Short delay to allow monitoring tools (like htop) to capture the change
        usleep(150000); // 150 milliseconds
    }

    printf("Program finished. Press Enter to release memory and exit.\n");
    getchar(); // Keep memory allocated until user input, if desired

    return 0;
}
