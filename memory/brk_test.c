#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    fprintf(stdout, "process id: %d\n", getpid());
    void* curr_brk = NULL;
    void* temp_brk = NULL;

    // 获取当前brk指针地址
    temp_brk = curr_brk = sbrk(0);
    fprintf(stdout, "Current location of brk: %p\n", curr_brk);

    // 申请堆内存
    int* a = malloc(sizeof(uint32_t) * 129);
    curr_brk = sbrk(0);
    fprintf(stdout, "Current location of brk after malloc: %p\n", curr_brk);

    getchar();
    return EXIT_SUCCESS;
}
