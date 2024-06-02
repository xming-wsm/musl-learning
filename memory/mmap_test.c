#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>


void static inline errExit(const char* msg) {
    printf("%s failed. Exiting the process\n", msg);
    exit(-1);
}


int main(int argc, char *argv[])
{
    int ret = -1;
    printf("Process pid: %d\n", getpid());
    printf("Before mmap\n");
    getchar();

    char* addr = NULL;
    addr = mmap(NULL, (size_t)132*1024, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        errExit("mmap");
    }
    printf("After mmap\n");
    getchar();


    /* unmap mapped region. */
    ret = munmap(addr, (size_t)132*1024);
    if (ret == -1) {
        errExit("mumap");
    }
    printf("After mumap\n");
    getchar();
    return EXIT_SUCCESS;
}
