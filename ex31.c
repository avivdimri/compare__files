// Aviv Dimri 206322406
#include <fcntl.h>
#include <zconf.h>
#include <memory.h>
#include <stdlib.h>

//The function gets file and buffer and read all the 
//chars while they are space to skip them
int skip_space(int file, char buf[]) {
    int  i;
    while ((i=read(file, buf, 1)) > 0) {
        if (strcmp(buf, " ") != 0) {
            return 0;
        }
    }
    if (i < 0) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    return -1;
}
//the function gets 2 chars and check if they are case sesetive
int IsCaseSenstive(char a, char b) {
    if (a > b) {
        return (a - 32 == b);
    }
    return (b - 32 == a);
}

int compare(int fd, int fd2, char buf[], char buf2[], int *sim) {
    if (strcmp(buf, buf2) != 0) {
        *sim = 1; //the files aren't identify
        if ((strcmp(buf, " ") == 0) || (strcmp(buf, "\n") == 0)) {
            // the chars aren't different, just space
            if (!skip_space(fd, buf)) {
                //after skip all the spaces continue to compare the next chars
                return compare(fd, fd2, buf, buf2, sim);
            }
            return -1;
        }
        if ((strcmp(buf2, " ") == 0) || (strcmp(buf2, "\n") == 0)) {
            // the chars aren't different, just space
            if (!skip_space(fd2, buf2)) {
                //after skip all the spaces continue to compare the next chars
                return compare(fd, fd2, buf, buf2, sim);
            }
            return -1;
        }
        //check if the chars are case sensitive
        if (!IsCaseSenstive(buf[0], buf2[0])) {
            //the files are different
            return -1;
        }
    }
    // the chars are same
    return 1;
}

// the function gets 2 paths of files from the command line and return 1 if the are identify,
// 3 if they are same ,and 2 id they are different
int main(int argc, char **argv) {
    int sim = 0; //flag if the files are not identify
    int fd, fd2;
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }
    fd2 = open(argv[2], O_RDONLY);
    if (fd2 < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }
    char buf[20],buf2[20];
    int i, j;
    i = read(fd, buf, 1);
    if (i < 0) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    j = read(fd2, buf2, 1);
    if (j < 0) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    while ((i > 0) && (j > 0)) {
        if (compare(fd, fd2, buf, buf2, &sim) < 0) {
            //the chars are different
            return 2;
        }
        //read the next char
        i = read(fd, buf, 1);
        if (i < 0) {
            write(1, "Error in: read\n", strlen("Error in: read\n"));
            exit(-1);
        }
        j = read(fd2, buf2, 1);
        if (j < 0) {
            write(1, "Error in: read\n", strlen("Error in: read\n"));
            exit(-1);
        }
    }
    if (i) {
        //there are more chars to read so the files are not identify
        sim = 1;
        //read the rest and check if there is only space chars
        while (i > 0) {
            if ((strcmp(buf, " ") != 0) && (strcmp(buf, "\n") != 0)) {
                //the files are different
                return 2;
            }
            i = read(fd, buf, 1);
        }
    }
    if (j) {
        //there are more chars to read so the files are not identify
        sim = 1;
        //read the rest and check if there is only space chars
        while (j > 0) {
            if ((strcmp(buf2, " ") != 0) && (strcmp(buf2, "\n") != 0)) {
                //the files are different
                return 2;
            }
            j = read(fd2, buf2, 1);
        }
    }

    if (sim) {
        // the flag is turn on so the files aren't identify but are same
        return 3;
    }
    // the files are identify
    return 1;
}

