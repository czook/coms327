#include <stdio.h>
#include <stdlib.h>

void genBorder();

int main(int argc, char* argv[]) {
    genBorder();
}

void genBorder() {
    for (int i = 0; i < 80; i++) {
        printf("-");
    }
    printf("\n");
    for (int i = 0; i < 19; i++) {
        printf("|                                                                              |\n");
    }
    for (int i = 0; i < 80; i++) {
        printf("-");
    }
    printf("\n");
}