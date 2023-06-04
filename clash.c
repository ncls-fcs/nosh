#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#include "plist.c"

#define MAX_INPUT_SIZE 100
#define DELIMITER_CHARS "   "
const size_t MAX_DIRECTORY_LENGTH_CONST = 100;

static int checkBackgroundProcess(pid_t pid, const char *cmd) {

    int exitstatus = 0;
    pid_t status = waitpid(pid, &exitstatus, WNOHANG);
    if(status == -1) {  
        perror("waitpid");
    }else if(status != 0) {
        if(WIFEXITED(exitstatus)) {                                 //if process specified by pid stopped, report exitstatus and remove process from list of background tasks
            printf("Exitstatus [%s] = %d\n", cmd, WEXITSTATUS(exitstatus));
            char *buf = "";     //buffer that needs to be given to removeElement
            removeElement(pid, buf, 0);
        }
    }
    return 0;   //traverse through whole list (return value of 0 indicates going to next element in list)
}

static int printElements(pid_t pid, const char *cmd) {
    printf("%i [%s]\n", pid, cmd);
    return 0;
}

static void changeDirectory(char **args, int number_of_elements){
    if(number_of_elements == 3) {
        chdir(args[1]);
    }else{
        printf("usage: cd <directory>");
    }
}

static void printcwd(){
    char current_directory[PATH_MAX];
    if(getcwd(current_directory, PATH_MAX) == NULL) {
        //error handling
        perror("getcwd");
    }else {
        printf("%s: ", current_directory);
    }
}

static int readstdin(char *line){
    /*reads line of MAX_INPUT_SIZE length from stdin and saves content to *line parameter. If input on stdin is overlength, flushes stdin and returns -1, returns 0 on success*/
    if(fgets(line, MAX_INPUT_SIZE+1, stdin) == NULL){
        /*fgets error handling*/
        if(ferror(stdin)) {     //real error -> terminate with EXIT_FAILURE
            perror("fgets");
            exit(EXIT_FAILURE);
        }else{                  //end of file -> "normal" termination
            exit(EXIT_SUCCESS);
        }
    }
    /*handling of overlength input:*/
    //fgets reads a maximum of MAX_INPUT_SIZE chars so if MAX_INPUT_SIZE chars are read and the last char is not a newline it means that the input must be longer than MAX_INPUT_SIZE chars (excluding the newline)
    if(strlen(line) == MAX_INPUT_SIZE && line[MAX_INPUT_SIZE] != '\n') {
        fprintf(stderr, "Your input was too long (> %d chars, including newline)\n", MAX_INPUT_SIZE);

        //flushing stdin if input was longer than MAX_INPUT_SIZE chars
        int character;
        while((character = fgetc(stdin)) != '\n') {
            //error handling for fgetc
            if(character == EOF) {
                //if there is an error, terminate.
                if(ferror(stdin)) {
                    perror("fgetc");
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
        //when stdin is flushed, continue reading the next line
        return -1;
    }
    return 0;
}


int main(int argc, char const *argv[]) {
    
    char *args[MAX_INPUT_SIZE];      //array to hold arguments as *char (Strings) (there cant be more arguments than input characters so MAX_INPUT_SIZE is the upper bound for number of arguments)

    char line[MAX_INPUT_SIZE+1 * sizeof(char)];   //reserving memory for the contents of stdin that will be written to *line
    
    int (*checkBackgroundProcessPtr) (pid_t, const char *) = &checkBackgroundProcess;
    int isBackgroundProcess = 0;    //flag to indicate if '&' symbol was typed after command

    while(!feof(stdin)) {
        /*collecting zombies of background tasks:*/
        walkList(checkBackgroundProcessPtr);

        /*printing current working directory:*/
        printcwd();

        /*reading arguments:*/
        if(readstdin(line) == -1){   //reading MAX_INPUT_SIZE chars from stdin and saving them to *line
            continue;   //if input was overlength, skip to new command line
        }

        /*parsing input:*/
        char *current_argument;     //buffer in which each parsed argument will reside until it´s added to args array

        if(line[strlen(line)-2] == '&') {
            isBackgroundProcess = 1;
            line[strlen(line)-2] = '\0';
        } else {
            line[strlen(line)-1] = '\0';       //removes "\n"
        }

        current_argument = strtok(line, DELIMITER_CHARS);    
        if(!current_argument) {
            //line was empty
            fprintf(stderr, "No argument was passed\n");
            continue;
        }

        args[0] = current_argument;     //adding command name to first position in arguments array

        int number_of_args = 1;
        do{
            current_argument = strtok(NULL, DELIMITER_CHARS);
            args[number_of_args] = current_argument;
            number_of_args++;
        }while (current_argument);

        /*own commands*/
        if(strcmp(args[0], "jobs") == 0) {
            if(number_of_args > 2) {
                printf("usage: jobs\n");
            }else{
                walkList(printElements);
            }
            continue;
        }else if(strcmp(args[0], "cd") == 0) {
            changeDirectory(args, number_of_args);
        }


        
        //make command string out of args array (without '&')
        int i = 0;
        char *cmd = malloc(MAX_INPUT_SIZE);       //gets free´d in plist.c when removing element associated with cmd
        if(cmd == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        while(args[i]){
            strcat(cmd, args[i]);
            strcat(cmd, " ");
            i++;
        }
        cmd[strlen(cmd)-1] = '\0';  //remove last space     

        /*starting new process:*/
        pid_t pid = fork();

        if(pid == -1) {
            //forking failed
            perror("fork");
            exit(EXIT_FAILURE);
        }else if (pid == 0) {
            //in child proccess -> executing command
            execvp(args[0], args);
            //exec only returns on error
            perror("exec");
            exit(EXIT_FAILURE);
        }else {
            //in parent proccess

            //differentiate between fore- and background tasks
            if(isBackgroundProcess) {
                printf("background process initiated\n");
                //background task
                printf("%s", cmd);   

                insertElement(pid, cmd);       //add background task to list of background tasks
            }else{
                //foreground task -> suspend thread until child terminates
                int exitstatus = 0;
                wait(&exitstatus);

                if(WIFEXITED(exitstatus)) {
                    printf("Exitstatus [%s] = %d\n", cmd, WEXITSTATUS(exitstatus));
                }
            }
        }
        isBackgroundProcess = 0;
    }

    return 0;
}

/*
int start_new_process() {
    pid_t pid = fork();

    if(pid == -1) {
        //forking failed

    }else if (pid == 0) {
        //in child proccess
    }else {
        //in parent proccess
    }
    
}
*/
