#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define FIFO_PATH "/tmp/bus_fifo_no_opt"
#define NUM_PACKETS 10000000
#define PKT_SIZE 64

typedef struct {
    int id;
    char payload[PKT_SIZE - sizeof(int)];
} Packet;

int main() {
    mkfifo(FIFO_PATH, 0666);
    printf("[Prod_NO] Waiting for the consumer...\n");
    int fd = open(FIFO_PATH, O_WRONLY);

    Packet pkt;
    memset(pkt.payload, 'A', sizeof(pkt.payload));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Send packet by packet (10,000,000 write syscalls)
    for (int i = 0; i < NUM_PACKETS; i++) {
        pkt.id = i;
        if (write(fd, &pkt, sizeof(Packet)) <= 0) {
            perror("write error");
            break;
        }
    }

    close(fd);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double pps = NUM_PACKETS / elapsed;
    double mbps = ((double)NUM_PACKETS * PKT_SIZE) / (1024.0 * 1024.0 * elapsed);

    printf("\n=== NON-OPTIMIZED PRODUCER METRICS (Prod_NO) ===\n");
    printf("Packets sent    : %d\n", NUM_PACKETS);
    printf("Total time      : %.4f seconds\n", elapsed);
    printf("Speed (PPS)     : %.0f packets/sec\n", pps);
    printf("Throughput      : %.2f MB/s\n", mbps);
    return 0;
}
