#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int a = 5;
    int* b = malloc(sizeof(int));
    memcpy(b, &a, sizeof(int));
    printf("a = %d, b = %d\n", a, *b);
    return 1;
}
