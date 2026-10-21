#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define SHM_NAME "/bus_shm_opt"
#define NUM_PACKETS 10000000
#define PKT_SIZE 64
#define BATCH_SIZE 500
#define TOTAL_BATCHES (NUM_PACKETS / BATCH_SIZE)

typedef struct {
    int id;
    char payload[PKT_SIZE - sizeof(int)];
} Packet;

typedef struct {
    sem_t sem_read;
    sem_t sem_write;
    Packet batch[BATCH_SIZE];
} SharedBusBuffer;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[Prod_OPT] Error creating shared memory");
        return 1;
    }

    if (ftruncate(shm_fd, sizeof(SharedBusBuffer)) == -1) {
        perror("[Prod_OPT] Error in ftruncate");
        return 1;
    }

    SharedBusBuffer *shm = mmap(0, sizeof(SharedBusBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("[Prod_OPT] Error in mmap");
        return 1;
    }

    sem_init(&shm->sem_read, 1, 0);
    sem_init(&shm->sem_write, 1, 1);

    printf("[Prod_OPT] Starting optimized transfer (Zero-Copy Batching)...\n");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int b = 0; b < TOTAL_BATCHES; b++) {
        sem_wait(&shm->sem_write);

        // Fill the batch directly in shared memory (Zero-Copy)
        for (int i = 0; i < BATCH_SIZE; i++) {
            shm->batch[i].id = b * BATCH_SIZE + i;
            memset(shm->batch[i].payload, 'B', sizeof(shm->batch[i].payload));
        }

        sem_post(&shm->sem_read);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double pps = NUM_PACKETS / elapsed;
    double mbps = ((double)NUM_PACKETS * PKT_SIZE) / (1024.0 * 1024.0 * elapsed);

    printf("\n=== OPTIMIZED PRODUCER METRICS (Prod_OPT) ===\n");
    printf("Packets sent    : %d\n", NUM_PACKETS);
    printf("Total time      : %.4f seconds\n", elapsed);
    printf("Speed (PPS)     : %.0f packets/sec\n", pps);
    printf("Throughput      : %.2f MB/s\n", mbps);

    munmap(shm, sizeof(SharedBusBuffer));
    close(shm_fd);
    return 0;
}
