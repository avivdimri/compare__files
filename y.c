#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
..this function runs the ex31.c - the program that compare between 2 files.
void runningEX31(struct dirent * ditOut,int fdCSV,char pathLine3[]){
    int status,fdOutput;
    pid_t pid = fork();

    if(pid < 0){
        write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    }
    //in the child process
    if(pid == 0){
        //check if the outputfile is exist.
        if((fdOutput=open(pathLine3,O_RDONLY))==-1){
            write(1, "Output file not exist\n", strlen("Error in: open file\n"));
            exit(-1);
        }
        char *argv[] = {"./comp.out",pathLine3,"outputC.txt",NULL};
        //run the ex31.c file with arguments
        if(execvp("./comp.out",argv)==-1){
            write(1, "Error in: exec\n", strlen("Error in: exce\n"));
        }
    }
    //in the father process
    else{
        //wait to the child process
        if(waitpid(pid,&status,0)<0){
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n")); 
        }
        //the files are equal, write in the csvFile
        if(WEXITSTATUS(status) == 1){
            char text[150]={0};
            strcpy(text,ditOut->d_name);
            strcat(text,",100,EXCELLENT\n");
            write(fdCSV,text,sizeof(text));
        }
        //the files are similar, write in the csvFile
        if(WEXITSTATUS(status) == 3){
            char text[150]={0};
            strcpy(text,ditOut->d_name);
            strcat(text,",75,SIMILAR\n");
            write(fdCSV,text,sizeof(text));
        }
       //the files are different, write in the csvFile
        if(WEXITSTATUS(status) == 2){
            char text[150]={0};
            strcpy(text,ditOut->d_name);
            strcat(text,",50,WRONG\n");
            write(fdCSV,text,sizeof(text));
        }
        
    }
    
}
//this function run the c file we found in the folder.
void runningC(char* programName,struct dirent * ditOut,struct dirent * ditIn,int status,int fdCSV,char pathLine3[],char pathLine2[]){
    int fdTXT,fdInput;
    //open outputfile for the program.
    if((fdTXT = open("outputC.txt", O_CREAT | O_TRUNC | O_WRONLY, 0777))==-1){
        write(1, "Error in: open file\n", strlen("Error in: open file\n"));
        exit(-1);
    }
    //check if the inputfile is exist.
    if((fdInput=open(pathLine2,O_RDONLY))==-1){
        write(1, "Input file not exist\n", strlen("Error in: open file\n"));
        exit(-1);
    }
    //I/O redirection
    dup2(fdInput,0);
    dup2(fdTXT,1);
    
    pid_t pid = fork();
    if(pid < 0){
        write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    }
    //in the child process
    if(pid == 0){
        strcat(programName,"./a.out");
        char *argv[] = {"./a.out",programName,NULL};
        //run the c file with the input from the inputfile.
        if(execvp("./a.out",argv)==-1){
            write(1, "Error in: exec\n", strlen("Error in: exce\n"));
        }
    }
    //in the father process
    else{
        long now1 = time(NULL);
        if(waitpid(pid,&status,0)<0){
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n")); 
        }
        long now2 = time(NULL);
        //check if the program runns more than 5 seconds, and write in the csv file.
        if(now2 - now1 >5){
            char text[150]={0};
            strcpy(text,ditOut->d_name);
            strcat(text,",20,TIMEOUT\n");
            write(fdCSV,text,sizeof(text));
            
        }
        else{
            runningEX31(ditOut,fdCSV,pathLine3);
        }
    } 
    close(fdInput);
    close(fdTXT);
    remove("a.out");
}
//this program compile the c file we found in the folder
void compilation(char* programName,struct dirent * ditOut,struct dirent * ditIn,int status,int fdCSV,char pathLine3[],char pathLine2[]){
    pid_t pid;
    pid = fork();
    if(pid < 0){
        write(1, "Error in: fork\n", strlen("Error in: fork\n"));
    }
    //in the child process
    if(pid == 0){
        //לקמפל
        char *argv[] = {"gcc","-o","a.out",strcat(programName,ditIn->d_name),NULL};
        if(execvp("gcc",argv)==-1){
            write(1, "Error in: exec\n", strlen("Error in: exce\n"));
        }
    }
    //in the father process
    else{
        if(waitpid(pid,&status,0)<0){
            write(1, "Error in: waitpid\n", strlen("Error in: waitpid\n")); 
        }
        //if the compile failed, write in the csv file
        if(WEXITSTATUS(status) != 0){
            //printf("compilation error\n");
            char text[150]={0};
            strcpy(text,ditOut->d_name);
            strcat(text,",10,COMPILATION_ERROR\n");
            write(fdCSV,text,sizeof(text));
        } 
        else{
            runningC(programName,ditOut,ditIn,status,fdCSV,pathLine3,pathLine2);
            
        }  
    }
}

int main(int argc, char *argv[]){
    int openFile,readFile;
    DIR *pDir;
    char line1[150]={0};
    char line2[150]={0};
    char line3[150]={0};
    char buffer[1];
    char text[150];
    int i=0;
    //open the configuration file.
    if((openFile=open("conf.txt",O_RDONLY))==-1){
        write(1, "Error in: open file\n", strlen("Error in: open file\n"));   /* text explaining why */ 
		exit(-1); 
    }
    //read the file and split it to 3 buffers - line1,2,3.
    readFile = read(openFile,buffer,sizeof(buffer));
    while(readFile>0){
        if (buffer[0] == '\n'){
            break;
        }
        line1[i] = buffer[0];
        i++;
        readFile = read(openFile,buffer,sizeof(buffer));
    }
    //read line2
    readFile = read(openFile,buffer,sizeof(buffer));
    i=0;
    while(readFile>0){
        if (buffer[0] == '\n'){
            break;
        }
        line2[i] = buffer[0];
        i++;
        readFile = read(openFile,buffer,sizeof(buffer));
    }
    //read line3
    readFile = read(openFile,buffer,sizeof(buffer));
    i=0;
    while(readFile>0){
        if (buffer[0] == '\n'){
            break;
        }
        line3[i] = buffer[0];
        i++;
        readFile = read(openFile,buffer,sizeof(buffer));
    }
    if(readFile==-1){
        write(1, "Error in: read file\n", strlen("Error in: read file\n"));   /* text explaining why */ 
		exit(-1); 
    }
    //open folder from line 1
    if ((pDir = opendir (line1)) == NULL) {
        perror ("Not a valid directory\n");
        exit(-1);
    }
   
    struct dirent * ditOut;
    struct dirent * ditIn;
    char pathLine2[150];
    char pathLine3[150];
    getcwd(pathLine2,150);
    strcat(pathLine2,"/");
    strcat(pathLine2,line2);
    getcwd(pathLine3,150);
    strcat(pathLine3,"/");
    strcat(pathLine3,line3);
    
    DIR* newPDir;
    int fdCSV,status,flagC=0,fdError;
    //open CSV file for the results.
    if((fdCSV = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0777))==-1){
        write(1, "Error in: open file\n", strlen("Error in: open file\n"));
        exit(-1);
    }
    //open txt file for the errors.
    if (fdError=open("errors.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644) == -1) {
        write(1, "Error in: open file\n", strlen("Error in: open file\n"));
        exit(-1);
    }
    //change the direct of the errors
    dup2(fdError, 2);
    //skip the '.','..' files.
    ditOut = readdir(pDir);
    ditOut = readdir(pDir);
    //go over all the files in the folder.
    while((ditOut = readdir(pDir))!=NULL){
        
        if(ditOut->d_type == DT_DIR){
            char* name = ditOut->d_name;
            char *temp = strcat(line1 , "/");
            char* innerFolder = strcat(temp , name);
            //get into the file if it is a folder
            if((newPDir = opendir(innerFolder)) != NULL){
                //go over all the files in the innerfolder.
                while((ditIn= readdir(newPDir))!=NULL){
                    int size = strlen(ditIn->d_name);
                    char* file = ditIn->d_name;
                    //get into the file if it is a c file
                    if ( ditIn->d_type == DT_REG && file[size-1] == 'c' && file[size-2] == '.'){
                        flagC =1;
                        char* programName = strcat(innerFolder,"/");
                        compilation(programName,ditOut,ditIn,status,fdCSV,pathLine3,pathLine2);
                      
                    }
                }
                //if there is not a c file in the folder, write in the CSV file
                if(!flagC){
                    char text[150]={0};
                    strcpy(text,ditOut->d_name);
                    strcat(text,",0,NO_C_FILE\n");
                    write(fdCSV,text,sizeof(text));
                }
                strtok(line1,"/");
                flagC =0;
            }
            else{
                //printf ("Cannot open directory");
            }
        }
    }
    //closing all the files and folder and remove not useful files
    remove("outputC.txt");
    closedir(newPDir);
    closedir(pDir);
    close(openFile);
    close(fdError);
    close(fdCSV);
    
    return 0;
}