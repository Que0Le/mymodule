/* https://cirosantilli.com/linux-kernel-module-cheat#mmap */

#define _XOPEN_SOURCE 700
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uintmax_t */
#include <string.h>
#include <sys/mman.h>
#include <unistd.h> /* sysconf */
#include <time.h>

#include "lkmc/pagemap.h" /* lkmc_pagemap_virt_to_phys_user */

#include "common.h"
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

static unsigned long get_nsecs(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000UL + ts.tv_nsec;
}

int main(int argc, char **argv) {
    int fd;
    long page_size;
    char *address1, *address2;
    char buf2[BUFFER_SIZE];
    uintptr_t paddr;

    /* Allocate mem for log */
    unsigned long *log_time_stamps = (unsigned long *) malloc(MAX_LOG_ENTRY*8);
    if (!log_time_stamps) {
        printf("Malloc log_time_stamps failed\n!");
        return -1;
    }
    memset(log_time_stamps, 0, MAX_LOG_ENTRY*8);
    for (int i=0; i<MAX_LOG_ENTRY; i++) {
        if (log_time_stamps[i] != 0) {
            printf("memset log_time_stamps failed\n!");
            return -1;
        }
    }

    /* Open proc file */
    char name_buff[128];
    memset(name_buff, '\0', 100);
    memcpy(name_buff, path_prefix, strlen(path_prefix));
    memcpy(name_buff+strlen(path_prefix), proc_filename, strlen(proc_filename));
    printf("open pathname = %s\n", name_buff);
    fd = open(name_buff, O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open");
        assert(0);
    }
    printf("fd = %d\n", fd);
    /* Map mem */
    page_size = sysconf(_SC_PAGE_SIZE);
    address1 = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (address1 == MAP_FAILED) {
        perror("mmap");
        assert(0);
    }

    signal(SIGINT, sig_handler);
    while(keep_running) {
        // memset(buf, '\0', BUFFER_SIZE);
        // read(fd, buf, BUFFER_SIZE);
        // printf("buf     =%s\n", buf);

        memset(buf2, '\0', BUFFER_SIZE);
        ssize_t r = pread(fd, buf2, BUFFER_SIZE, 0);
        uint64_t now = get_nsecs();
        for (int i=0; i<PKTS_PER_BUFFER; i++) {
            if (buf2[i*PKT_BUFFER_SIZE] == '\0')
                continue;
            // Extract data from packet
            // printf("buf2[%zu] =%s us[%lu]\n", r, buf2+i*PKT_BUFFER_SIZE, now);
            struct Payload pl;
            memcpy(&pl, buf2+i*PKT_BUFFER_SIZE, sizeof(struct Payload));
            pl.us_time_arrival_1 = now;
            printf("-------------------------------------------------------\n");
            printf("Client_id[%lu] uid[%lu] type[%lu] create_time[%lu] delta[%lu usec]\n",
                pl.client_uid, pl.uid, pl.type, pl.created_time, (now-pl.ks_time_arrival_2)/1000);
            printf("             ks_1[%lu] ks_2[%lu] us_1[%lu] us_2[%lu]\n", 
                pl.ks_time_arrival_1, pl.ks_time_arrival_2,
                pl.us_time_arrival_1, pl.us_time_arrival_2);
            // Add timestamp to log at uid
            if (pl.uid <MAX_LOG_ENTRY && pl.uid >= 0) {
                log_time_stamps[pl.uid] = now;
            }
        }

        // strncpy(buf2, address1, BUFFER_SIZE);
        // printf("buf2    =%s\n", buf2);
        // usleep(10);
        // break;
    }
    printf("\nExporting log file ...\n");   // enter new line to avoid the Ctrl+C (^C) char
    // for (int i=0; i<100; i++)
    //     printf("uid[%d] time[%lu]\n", i, log_time_stamps[i]);

    /* Export log to text file */
    FILE *fp;
    char buf[128];
    fp = fopen(path_log_export_up, "w");
    for (unsigned long i=0; i<MAX_LOG_ENTRY; i++) {
        memset(buf, '\0', 128);
        snprintf(buf, 100, "%lu\n", log_time_stamps[i]);
        fprintf(fp, buf);
    }
    fclose(fp);

    close(fd);


    // puts("mmap 2");
    // address2 = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // if (address2 == MAP_FAILED) {
    //     perror("mmap");
    //     return EXIT_FAILURE;
    // }
    // assert(address1 != address2);

    // /* Read and modify memory. */
    // puts("access 1");
    // puts(address1);
    // assert(!strcmp(address1, "asdf"));
    // /* vm_fault */
    // puts("access 2");
    // assert(!strcmp(address2, "asdf"));
    // /* vm_fault */
    // strcpy(address1, "qwer");
    // /* Also modified. So both virtual addresses point to the same physical address. */
    // assert(!strcmp(address2, "qwer"));

    // /* Check that the physical addresses are the same.
    //  * They are, but TODO why virt_to_phys on kernel gives a different value? */
    // assert(!lkmc_pagemap_virt_to_phys_user(&paddr, getpid(), (uintptr_t)address1));
    // printf("paddr1 = 0x%jx\n", (uintmax_t)paddr);
    // assert(!lkmc_pagemap_virt_to_phys_user(&paddr, getpid(), (uintptr_t)address2));
    // printf("paddr2 = 0x%jx\n", (uintmax_t)paddr);

    // /* Check that modifications made from userland are also visible from the kernel. */
    // read(fd, buf, BUFFER_SIZE);
    // assert(!memcmp(buf, "qwer", BUFFER_SIZE));

    // /* Modify the data from the kernel, and check that the change is visible from userland. */
    // write(fd, "zxcv", 4);
    // assert(!strcmp(address1, "zxcv"));
    // assert(!strcmp(address2, "zxcv"));

    // /* Cleanup. */
    // puts("munmap 1");
    // if (munmap(address1, page_size)) {
    //     perror("munmap");
    //     assert(0);
    // }
    // puts("munmap 2");
    // if (munmap(address2, page_size)) {
    //     perror("munmap");
    //     assert(0);
    // }
    // puts("close");
    // close(fd);
    return EXIT_SUCCESS;
}
