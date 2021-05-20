
// Created by אביב דמרי on 10/05/2021.
//

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <dirent.h>
#include <zconf.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
int run() {
    char *arr[] = {"./a.out", NULL};
    pid_t pid;
    int stat;
    time_t start_t, end_t;
    if(time(&start_t)<0){
        write(1, "Error in: time\n", strlen("Error in: time\n"));
        return -1;
    }
    if ((pid = fork()) == 0) {
        //the son make the action
        execvp("./a.out", arr);
        write(1, "Error in: execvp\n", strlen("Error in: execvp\n"));
        return -1;
    }
    else if (pid>0) {
        if (waitpid(pid, &stat, 0) < 0) {
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            return -1;
        }
        time(&end_t);
        double diff_t = difftime(end_t, start_t);
        remove("a.out");
        if (diff_t > 5) {
            return -1;
        }
        return 1;
    }
    write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    return -1;
}

int run2(char file1[150], char file2[150]) {
    char d[150];
    getcwd(d, 150);
    char *arr[] = {"./comp.out", file1, file2, NULL};
    pid_t pid;
    int stat;
    if ((pid = fork()) == 0) {
        //the son make the action
        execvp("./comp.out", arr);
        write(1, "Error in: execvp\n", strlen("Error in: execvp\n"));
        return -1;

    }
    else if (pid>0) {
        if (waitpid(pid, &stat, 0) < 0) {
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            return -1;
        }
        int v = WEXITSTATUS(stat);
        return v;
    }
    write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    return -1;
}



int compile_file(char *namefile) {
    char *arr[] = {"gcc", namefile, NULL};
    pid_t pid;
    int stat;
    if ((pid = fork()) == 0) {
        //the son make the action
        stat = execvp("gcc", arr);
        write(1, "Error in: execvp\n", strlen("Error in: execvp\n"));
        return -1;

    } else if (pid > 0) {
        if (waitpid(pid, &stat, 0) <0){
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            return -1;
        }
        int val = WEXITSTATUS(stat);
        if (val != 0) {
            return -1;
        }
        return 0;
    }
    write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    return -1;
}

int readLine(int fd, char *line) {
    char buffer[2];
    char *p = line;
    while (read(fd, buffer, 1) > 0) {
        if (strcmp(buffer, "\n") == 0) {
            *p = '\0';
            return 0;
        }
        *p++ = buffer[0];
    }
    return -1;
}

int find_file_c(char *path) {
    DIR *pDir;
    char name_file[150];
    struct dirent *pDirent;
    if ((pDir = opendir(path)) == NULL) {
        write(1, "Error in: opendir\n", strlen("Error in: opendir\n"));
        exit(-1);
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        strcpy(name_file, pDirent->d_name);
        if (pDirent->d_type == DT_DIR) {
            continue;
        }
        unsigned length = strlen(name_file);
        if ((name_file[length - 1] == 'c') && (name_file[length - 2] == '.')) {
            strncat(path, "/", 1);
            strncat(path, name_file, strlen(name_file));
            if (compile_file(path) == 0) {
                closedir(pDir);
                return 20;
            }
            closedir(pDir);
            return 10;
        }
    }
    closedir(pDir);
    return 0;
}

void write_result(int grade, char name[], int res) {
    strcat(name, ",");
    switch (grade) {
        case 0:
            strcat(name, "0,NO_C_FILE\n");
            break;
        case 10:
            strcat(name, "10,COMPILATION_ERROR\n");
            break;
        case 20:
            strcat(name, "20,TIMEOUT\n");
            break;
        case 50:
            strcat(name, "50,WRONG\n");
            break;
        case 75:
            strcat(name, "75,SIMILAR\n");
            break;
        case 100:
            strcat(name, "100,EXCELLENT\n");
            break;
        default:
            strcat(name, "errorrrrr\n");
            break;
    }
    write(res, name, strlen(name));

}
void check_access(char *path, char *massage) {
    int acc = access(path, F_OK);
    if (errno == ENOENT) {
        write(1, massage, strlen(massage));
        exit(-1);
    }
    if (acc < 0) {
        write(1, "Error in: access\n", strlen("Error in: access\n"));
        exit(-1);
    }
}

int main(int argc, char **argv) {
    char dir_path[150], in_path[150], out_path[150];
    int fdin = open(argv[1], O_RDONLY);
    if (fdin < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }
    if (readLine(fdin, dir_path)) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    check_access(dir_path,"Not a valid directory\n");

    if (readLine(fdin, in_path)) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    check_access(in_path, "Input file not exist\n");
    if (readLine(fdin, out_path)) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    check_access(out_path,"Output file not exist\n");
    close(fdin);


    int res, error;
    error = open("errors.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (error < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }

    if (dup2(error, 2) < 0){
        write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
        exit(-1);
    }
    res = open("results.csv", O_CREAT | O_RDWR, 0666);
    if (res < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }

    DIR *pDir;
    struct dirent *pDirent;
    if ((pDir = opendir(dir_path)) == NULL) {
        write(1, "Error in: opendir\n", strlen("Error in: opendir\n"));
        exit(-1);
    }
    char main_path[150];
    char inner_path[150];
    getcwd(main_path, 150);
    strcpy(inner_path, main_path);
    strncat(inner_path, "/", 1);
    strncat(inner_path, dir_path, 150);
    char c[150];
    char b[150];
    int grade = 0;

    while ((pDirent = readdir(pDir)) != NULL) {
        strcpy(c, pDirent->d_name);
        if ((pDirent->d_type != DT_DIR) || (!strcmp(c, "..")) || (!strcmp(c, "."))) {
            continue;
        }
        strcpy(b, dir_path);
        strncat(b, "/", 1);
        strncat(b, c, strlen(c));
        grade = find_file_c(b);

        if (grade > 10) {
            int input, output;
            if ((input = open(in_path, O_RDONLY)) < 0) {
                write(1, "Error in: open44\n", strlen("Error in: open44\n"));
                exit(-1);
            }

            int save_in = dup(0);
            if(save_in<0) {
                write(1, "Error in: dup\n", strlen("Error in: dup\n"));
                exit(-1);
            }

            if (dup2(input, 0) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            if ((output = open("dug.txt", O_WRONLY | O_TRUNC | O_CREAT, 0777)) < 0) {
                write(1, "Error in: open55\n", strlen("Error in: open55\n"));
                exit(-1);
            }
            int save_out = dup(1);
            if(save_out<0) {
                write(1, "Error in: dup\n", strlen("Error in: dup\n"));
                exit(-1);
            }

            if (dup2(output, 1) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            int r = run();
            close(input);
            if ( dup2(save_in, 0) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            if (dup2(save_out, 1) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }

            if (r > 0) {
                int v = run2(out_path, "dug.txt");
                close(output);
                switch (v) {
                    case 1:
                        grade = 100;
                        break;
                    case 2:
                        grade = 50;
                        break;
                    case 3:
                        grade = 75;
                        break;
                    default:
                        grade = -1;
                        break;
                }

            }
        }
        write_result(grade, c, res);

    }
    remove("dug.txt");
    close(error);
    close(res);
    closedir(pDir);
}
//void closes(){}