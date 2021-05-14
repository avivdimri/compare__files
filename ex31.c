#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <memory.h>

int sim = 0;
int fdin;
int f2;

int skip_space(int file, char buf[]) {
    //char buf[20];
    while (read(file, buf, 1) > 0) {
        if (strcmp(buf, " ") != 0) {
            return 0;
        }
    }
    return -1;
}

int caseSenstive(char a, char b) {
    if (a > b) {
        return (a - 32 == b);
    }
    return (b - 32 == a);
}

int compare(char buf[], char buf2[]) {
    if (strcmp(buf, buf2) != 0) {
        if ((strcmp(buf, " ") == 0) || (strcmp(buf, "\n") == 0)) {
            if (!skip_space(fdin, buf)) {
                return compare(buf, buf2);
            }
            return -1;
        }
        if ((strcmp(buf2, " ") == 0) || (strcmp(buf2, "\n") == 0)) {
            if (!skip_space(f2, buf2)) {
                return compare(buf, buf2);
            }
            return -1;
        }
        if (!caseSenstive(buf[0], buf2[0])) {
            return -1;
        }
        sim = 1;
    }
    return 1;
}

int main(int argc, char **argv) {
    fdin = open(argv[1], O_RDONLY);
    f2 = open(argv[2], O_RDONLY);

    if ((fdin < 0) || (f2 < 0)) {
        perror("can'tttt open");
        return -1;
    }
    char buf[20];
    char buf2[20];
    int i, j;
    i = read(fdin, buf, 1);
    j = read(f2, buf2, 1);
    while ((i > 0) && (j > 0)) {
        if (compare(buf, buf2) < 0) {
            return 2;
        }
        i = read(fdin, buf, 1);
        j = read(f2, buf2, 1);
    }
    if (i) {
        sim = 1;
        while (i > 0) {
            if ((strcmp(buf, " ") != 0) && (strcmp(buf, "\n") != 0)) {
                return 2;
            }
            i = read(fdin,buf,1);
        }
    }
    if (j) {
        sim = 1;
        while (j > 0) {
            if ((strcmp(buf2, " ") != 0) && (strcmp(buf2, "\n") != 0)) {
                return 2;
            }
            j = read(f2,buf2,1);
        }
    }

    if (sim) {
        return 3;
    }
    return 1;
}

