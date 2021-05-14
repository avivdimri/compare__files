
// Created by אביב דמרי on 10/05/2021.
//

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <dirent.h>
#include <zconf.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
int run(){
    char* arr[] = {"./a.out",NULL};
    pid_t pid;
    int stat;
    time_t start_t,end_t;
    time(&start_t);
    if ((pid = fork()) == 0) {
        //the son make the action
        execvp("./a.out", arr);
        printf("%s", "exec failed\n");
        return -1;

    } else{
        waitpid(pid,&stat,0);
        time(&end_t);
        double diff_t = difftime(end_t,start_t);
        remove("a.out");
        if (diff_t>5){
            return -1;
        }
        return 1;
    }
}

int run2(char file1[150],char file2[150]){
    char d[150];
    getcwd(d,150);
    char* arr[] = {"./comp.out",file1,file2,NULL};
    pid_t pid;
    int status;
    if ((pid = fork()) == 0) {
        //the son make the action
        execvp("./comp.out", arr);
        printf("%s", "exec failed\n");
        return -1;

    }
    waitpid(pid,&status,0);
    int v = WEXITSTATUS(status);
    return v;
}


int compile_file(char* namefile){
    char* arr[] = {"gcc",namefile,NULL};
    pid_t pid;
    int stat;
    if ((pid =fork()) == 0) {
        //the son make the action
        stat = execvp("gcc", arr);
        printf("%s", "exec failed\n");
        return -1;

    } else if (pid > 0) {
        waitpid(pid,&stat,0);
        int val = WEXITSTATUS(stat);
        if(val!=0){
            printf("didnt compile");
            return -1;
        }
        return 0;
    }
    printf("%s", "fork failed\n");
    return -1;
}
int readLine(int fd,char* line){
    char buffer[2];
    char* p = line;
    while(read(fd,buffer,1)>0){
        if(strcmp(buffer,"\n")==0){
            *p = '\0';
            return 0;
        }
        *p++ = buffer[0];
    }
    return -1;
}
int find_file_c(char *path){
    DIR *pDir;
    char name_file[150];
    struct dirent *pDirent;
    if((pDir=opendir(path))==NULL) {
        printf("error dir");
        return -1;
    }
    while ((pDirent=readdir(pDir))!= NULL) {
        strcpy(name_file,pDirent->d_name);
        if (pDirent->d_type == DT_DIR) {
            continue;
        }
        unsigned length = strlen(name_file);
        if((name_file[length-1]=='c')&&(name_file[length-2]=='.')){
            strncat(path, "/", 1);
            strncat(path, name_file, strlen(name_file));
            if(compile_file(path)==0){
                return 20;
            }
            return 10;
        }
    }
    return 0;
}
void write_result(int grade,char name[],int res){
    strncat(name, ",", 1);
    switch (grade) {
        case 0:
            strncat(name, "0,NO_C_FILE\n",150);
            break;
        case 10:
            strncat(name,"10,COMPILATION_ERROR\n",150);
            break;
        case 20:
            strncat(name,"20,TIMEOUT\n ",150);
            break;
        case 50:
            strncat(name,"50,WRONG\n",150);
            break;
        case 75:
            strncat(name,"75,SIMILAR\n",150);
            break;
        case 100:
            strncat(name,"100,EXCELLENT\n",150);
            break;
        default:
            strncat(name,"errorrrrr\n",150);
            break;
    }
    write(res,name,strlen(name));

}


int main() {
    char dir_path[150],in_path[150],out_path[150];
    int fdin = open("conf.txt", O_RDONLY);
    if (fdin<0){
        printf("cant open file");
    }
    if(readLine(fdin,dir_path)){
        printf("error line");
    }
    if(readLine(fdin,in_path)){
        printf("error line");
    }
    if(readLine(fdin,out_path)){
        printf("error line");
    }
    DIR *pDir;
    struct dirent *pDirent;
    if((pDir=opendir(dir_path))==NULL){
        printf("Not a valid directory\n");
        return -1;
    }
    char main_path[150];
    char inner_path[150];
    getcwd(main_path, 150);
    strcpy(inner_path,main_path);
    strncat(inner_path, "/", 1);
    strncat(inner_path, dir_path, 150);
    char c[150];
    char b[150];
    int res,error;
    int grade=0;
    error = open("errors.txt", O_CREAT|O_TRUNC|O_WRONLY, 0644);
    dup2(error, 2);
    res = open("resulttt.csv", O_CREAT|O_TRUNC|O_WRONLY, 0644);
    //res = open("result.csv", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR );
    if (res<0) {
        printf("open failedddd\n");
    }
    int i=0;
    while ((pDirent=readdir(pDir))!= NULL) {
        strcpy(c, pDirent->d_name);
        if ((pDirent->d_type != DT_DIR) || (!strcmp(c, "..")) || (!strcmp(c, "."))) {
            continue;
        }
        printf("%s\n", c);
        strcpy(b, dir_path);
        strncat(b, "/", 1);
        strncat(b, c, strlen(c));
        grade = find_file_c(b);

        if (grade>10) {

            char tmp[150];
            getcwd(tmp, 150);
            int input, output;

            if ((input = open(in_path, O_RDONLY)) < 0) {
                printf("Input file not exist\n");
                return -1;
            }

            int save_in = dup(0);
            dup2(input, 0);
            if ((output = open("dug.txt",  O_WRONLY | O_TRUNC | O_CREAT,0777))<0) {
                printf("Output file not exist\n");
                return 1;
            }

            int save_out = dup(1);
            dup2(output, 1);

            int r = run();
            dup2(save_in, 0);
            dup2(save_out, 1);

            if (r > 0) {

                int v = run2(out_path, "dug.txt");
                switch (v) {
                    case 1:
                        printf("1 is the king");
                        grade = 100;
                        break;
                    case 2:
                        printf("2 is the king");
                        grade = 50;
                        break;
                    case 3:
                        printf("3 is the king");
                        grade = 75;
                        break;
                    default:
                        printf("-1 is the king");
                        grade = -1;
                        break;
                }

            }
        }
        write_result(grade,c,res);

    }
    close(res);
    closedir(pDir);
    close(fdin);
}