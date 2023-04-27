#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
#define TRUE 1
#define GREEN "\x1B[32m"
#define CYAN "\x1B[36m"
#define RESET "\x1B[0m"


void read_command(char command[]){

    fgets(command, MAX_INPUT_SIZE, stdin);
    command[strcspn(command, "\n")] = 0;
}


void split_command(char command[], char *args[]){

    int i = 0;
    char *token;
    token = strtok(command, " ");
    while(token != NULL){
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = '\0';
}


void print_command_prompt(){

    char *user = getlogin();
    printf(CYAN);
    printf("[%s:", user );
    char path[MAX_INPUT_SIZE];
    getcwd(path, sizeof(path));
    printf(GREEN);
    printf("%s]$ ",path);
    printf(RESET);
}


void touch(const char *filename){
    int fd = open(filename, O_CREAT, 0666);
    if(fd == -1){
        perror("open");
        return;
    }
    close(fd);

}


int main()
{
    char command[MAX_INPUT_SIZE];
    char *args[MAX_INPUT_SIZE];
    char history[MAX_INPUT_SIZE][MAX_INPUT_SIZE];
    int history_count = 0;

    while (TRUE){

        print_command_prompt();
        read_command(command);

        strcpy(history[history_count % MAX_INPUT_SIZE], command);
        history_count++;

        split_command(command, args);

        if (strcmp(args[0], "help") == 0){

            printf("Projekt Microshell\nAutor: Agata Parzybut\n");
            printf("\nProgram obsluguje polecenia\ncd\ncp\necho\nexit\nhelp\nhistory\ntouch\noraz polecenia z fork() i exec*()\n");

        }
        else if(strcmp(args[0], "exit") == 0){

            exit(0);

        }
        else if(strcmp(args[0], "echo") == 0){

            int i = 1;
            while(args[i] != NULL){
                printf("%s ", args[i]);
                i++;
            }
            printf("\n");

        }

        else if(strcmp(args[0], "cd") == 0){

            if (args[1] == NULL ){
                chdir(getenv( "HOME" ));
            }
            else if(strcmp(args[1], "..") == 0){
                chdir("..");
            }
            else if(strcmp(args[1], ".") == 0 ){

            }
            else if(strcmp(args[1], "~") == 0){
                chdir(getenv( "HOME" ));
            }
            else{
                if(chdir(args[1]) == -1){
                    perror(0);
                }else{
                    chdir(args[1]);
                }
            }

        }
        else if(strcmp(args[0], "history") == 0){

            int i;
            for (i = 0; i < history_count; i++){
                printf("%d %s\n", i, history[i % MAX_INPUT_SIZE]);
            }

        }
        else if (strcmp(args[0], "touch") == 0){

            if(args[1] != NULL){
                touch(args[1]);
            }else{
                fprintf(stderr, "touch: missing operand\n");
            }

        }
        else if (strcmp(args[0], "cp") == 0) {

            if (args[1] == NULL || args[2] == NULL) {
                printf("usage: cp <source> <destination>\n");
            } else {
                FILE *source_file = fopen(args[1], "rb");
                if (source_file == NULL) {
                    printf("cp: cannot open '%s': %s\n", args[1], strerror(errno));
                    continue;
                }
                FILE *destination_file = fopen(args[2], "wb");
                if (destination_file == NULL) {
                    printf("cp: cannot create '%s': %s\n", args[2], strerror(errno));
                    fclose(source_file);
                    continue;
                }
                char buffer[MAX_INPUT_SIZE];
                size_t nread;
                while ((nread = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
                    if (fwrite(buffer, 1, nread, destination_file) != nread) {
                        printf("cp: write error\n");
                        break;
                    }
                }
                if (ferror(source_file)) {
                    printf("cp: read error\n");
                }
                fclose(source_file);
                fclose(destination_file);
            }

        }
        else{

            pid_t pid = fork();
            if (pid == 0){
                execvp(args[0], args);
                perror("invalid command");
                exit(EXIT_FAILURE);
            }
            else{
                waitpid(-1, NULL, 0);
            }
        }
    }

    return 0;
}
