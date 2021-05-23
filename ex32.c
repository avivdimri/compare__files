// Aviv Dimri 206322406
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

//the function run the a.out file and return 1 if succeed ,otherwise -1
int run() {
    char *arr[] = {"./a.out", NULL};
    pid_t pid;
    int stat;
    time_t start_t, end_t;
    //start the stopper
    if(time(&start_t)<0){
        write(1, "Error in: time\n", strlen("Error in: time\n"));
        return -1;
    }
    if ((pid = fork()) == 0) {
        //the son runs file
        execvp("./a.out", arr);
        write(1, "Error in: execvp\n", strlen("Error in: execvp\n"));
        return -1;
    }
    else if (pid>0) {
        if (waitpid(pid, &stat, 0) < 0) {
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            return -1;
        }
        time(&end_t); //stops the stopper
        double diff_t = difftime(end_t, start_t);
        remove("a.out");
        if (diff_t > 5) {
            //too much time
            return -1;
        }
        // succeed
        return 1;
    }
    write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    return -1;
}
// the function gets 2 paths to files and try run another program to compare them.
int run_ex31(char *file1, char *file2) {
    char *arr[] = {"./comp.out", file1, file2, NULL};
    pid_t pid;
    int stat;
    if ((pid = fork()) == 0) {
        //the son run the program
        execvp("./comp.out", arr);
        write(1, "Error in: execvp\n", strlen("Error in: execvp\n"));
        return -1;

    }
    else if (pid>0) {
        if (waitpid(pid, &stat, 0) < 0) {
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n"));
            return -1;
        }
        int ret = WEXITSTATUS(stat);
        switch (ret) {
            case 1:
                //the files are identify
                return 100;
            case 2:
                //the files are different
                return 50;
            case 3:
                //the files are same
                return 75;
            default:
                //error
                return -1;
        }
    }
    write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    return -1;
}


// the function gets path to c-file and try to compile it.
// the function return 0 if succeed, otherwise -1
int compile_file(char *path) {
    char *arr[] = {"gcc", path, NULL};
    pid_t pid;
    int stat;
    if ((pid = fork()) == 0) {
        //the son make the compile
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
            // the compilation failed
            return -1;
        }
        // the compilation succeed
        return 0;
    }
    write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    return -1;
}

// the function gets fd of file and pointer to line, and copy a row of the file in line
int readLine(int fd, char *line) {
    char buffer[2];
    int i;
    while ((i = read(fd, buffer, 1)) > 0) {
        if (strcmp(buffer, "\n") == 0) {
            //end of the line so finish
            *line = '\0';
            return 0;
        }
        *line++ = buffer[0]; //copy the char to the line
    }
    if (i<0) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    return -1; //error
}
// the function gets path of directory an looking for c file to compile.
// the function return 0 if didn't find c file,10 if found but the file didn't compile, and 20 if compiled
int find_file_c(char *path) {
    DIR *pDir;
    char name_file[150];
    struct dirent *pDirent;
    // open the directory
    if ((pDir = opendir(path)) == NULL) {
        write(1, "Error in: opendir\n", strlen("Error in: opendir\n"));
        exit(-1);
    }
    //go over the files in the directory
    while ((pDirent = readdir(pDir)) != NULL) {
        strcpy(name_file, pDirent->d_name);
        if (pDirent->d_type == DT_DIR) {
            //skip on the inner directory
            continue;
        }
        unsigned length = strlen(name_file);
        if ((name_file[length - 1] == 'c') && (name_file[length - 2] == '.')) {
            //found .c file
            strcat(path, "/");
            strcat(path, name_file);
            if (compile_file(path) == 0) {
                // the file is compiled
                closedir(pDir);
                return 20;
            }
            // the file isn't compiled
            closedir(pDir);
            return 10;
        }
    }
    //didn't find any c filein the directory
    closedir(pDir);
    return 0;
}
//the function gets grade, name of directory and fd of result csv file.
//the function write in the file the name, grade he got and message according to the grade
void write_result(int grade, char dir_name[], int res) {
    strcat(dir_name, ",");
    switch (grade) {
        case 0:
            strcat(dir_name, "0,NO_C_FILE\n");
            break;
        case 10:
            strcat(dir_name, "10,COMPILATION_ERROR\n");
            break;
        case 20:
            strcat(dir_name, "20,TIMEOUT\n");
            break;
        case 50:
            strcat(dir_name, "50,WRONG\n");
            break;
        case 75:
            strcat(dir_name, "75,SIMILAR\n");
            break;
        case 100:
            strcat(dir_name, "100,EXCELLENT\n");
            break;
        default:
            return;
    }
    write(res, dir_name, strlen(dir_name));

}

// the function gets path error message anf if there is no access to it the path
// the function write the error she get in 2nd parameter and exit from the program
void check_access(char *path, char *error_str) {
    int acc = access(path, F_OK);
    if (errno == ENOENT) {
        write(1, error_str, strlen(error_str));
        exit(-1);
    }
    if (acc < 0) {
        write(1, "Error in: access\n", strlen("Error in: access\n"));
        exit(-1);
    }
}

int main(int argc, char **argv) {
    char dir_path[150], in_path[150], out_path[150];
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }
    //read the first row which contain the path of the directory
    if (readLine(fd, dir_path)) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    check_access(dir_path,"Not a valid directory\n");

    //read the second row which contain the path of the input file
    if (readLine(fd, in_path)) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    check_access(in_path, "Input file not exist\n");

    //read the third row which contain the path of the output file
    if (readLine(fd, out_path)) {
        write(1, "Error in: read\n", strlen("Error in: read\n"));
        exit(-1);
    }
    check_access(out_path,"Output file not exist\n");
    close(fd);

    int res, error;
    error = open("errors.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (error < 0) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        exit(-1);
    }
    //change the destination of the errors
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
    // open the directory
    if ((pDir = opendir(dir_path)) == NULL) {
        write(1, "Error in: opendir\n", strlen("Error in: opendir\n"));
        exit(-1);
    }
    char main_path[150],inner_path[150];
    getcwd(main_path, 150);
    strcpy(inner_path, main_path);
    strcat(inner_path, "/");
    strcat(inner_path, dir_path);
    char dir_name[150],inner_dir_path[150];
    int grade;
    //go over the files in the directory
    while ((pDirent = readdir(pDir)) != NULL) {
        strcpy(dir_name, pDirent->d_name); //name of the inner directory
        if ((pDirent->d_type != DT_DIR) || (!strcmp(dir_name, "..")) || (!strcmp(dir_name, "."))) {
            //it's not directory so continue to the next one
            continue;
        }
        strcpy(inner_dir_path, dir_path);
        strcat(inner_dir_path, "/");
        strcat(inner_dir_path, dir_name);
        grade = find_file_c(inner_dir_path);
        if (grade > 10) {
            // found the c file and it's compiled
            int input, output;
            //open the file, which contains the input for the c file
            if ((input = open(in_path, O_RDONLY)) < 0) {
                write(1, "Error in: open\n", strlen("Error in: open\n"));
                exit(-1);
            }
            int save_in = dup(0); //save the src fd of the input
            if(save_in<0) {
                write(1, "Error in: dup\n", strlen("Error in: dup\n"));
                exit(-1);
            }
            //change to read the input from the file
            if (dup2(input, 0) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            //open  file, which contains the output of the c file
            if ((output = open("output.txt", O_WRONLY | O_TRUNC | O_CREAT, 0777)) < 0) {
                write(1, "Error in: open55\n", strlen("Error in: open55\n"));
                exit(-1);
            }
            int save_out = dup(1); //save the src fd of the output
            if(save_out<0) {
                write(1, "Error in: dup\n", strlen("Error in: dup\n"));
                exit(-1);
            }
            //change to write the output to the file
            if (dup2(output, 1) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            int is_run = run(); //run the c file
            close(input);
            //return to the source input
            if ( dup2(save_in, 0) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            //return to the source output
            if (dup2(save_out, 1) < 0){
                write(1, "Error in: dup2\n", strlen("Error in: dup2\n"));
                exit(-1);
            }
            if (is_run > 0) {
                // the run succeed,so compare to the expected output
                if((grade = run_ex31(out_path, "output.txt") < 0)){
                    continue;
                }
                close(output);
            }
        }
        write_result(grade, dir_name, res);

    }
    remove("output.txt");
    close(error);
    close(res);
    closedir(pDir);
}
