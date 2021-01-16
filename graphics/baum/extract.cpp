#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char ** argv)
{
    if (argc != 3) {
        fputs("args\n", stderr);
        return 1;
    }
    char * name = argv[1];
    char * p = strchr(name, '.');
    if (p == NULL) {
        perror("name-ext");
        return 1;
    }
    int n = atoi(p + 1);
    if (n <= 0) {
        perror("ext-width");
        return 1;
    }
    int bits = atoi(argv[2]);
    if (bits <= 0) {
        perror("ext-width");
        return 1;
    }

    FILE * in = fopen(argv[1], "r");
    struct stat sb;
    if (fstat(fileno(in), &sb) == -1) {
        perror("fstat");
        return 1;
    }
    int t = sb.st_size / (n << 2);

    printf("P5\n%d %d\n255\n", n, t);
    uint8_t buffer[2];
    for (int i = 0; i < n * t; i++) {
        uint32_t sample;
        if (1 != fread(&sample, sizeof sample, 1, in)) break;
        buffer[0] = (sample >> bits) & 255;
        if (1 != fwrite(buffer, 1, 1, stdout)) break;
    }
}
