#include <stdio.h>
#include <stdint.h>
#include <endian.h>
#include <string.h>
#include <time.h>

typedef struct binary{
    char file_type[12];
    uint32_t version;
    uint32_t size;
    uint8_t xPC;
    uint8_t yPC;
    uint8_t hardness[21][80];
}binary;

struct binary bin;
int main(int argc, char *argv[]){
}