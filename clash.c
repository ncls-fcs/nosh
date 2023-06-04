#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "plist.c"

#define MAX_INPUT_SIZE 100
#define DELIMITER_CHARS "   \n"
const size_t MAX_DIRECTORY_LENGTH_CONST = 100;

int checkBackgroundProcess(pid_t pid, const char *cmd) {

    int exitstatus = 0;
    if(waitpid(pid, &exitstatus, WNOHANG) == -1) {  
        perror("waitpid");
    }
    if(WIFEXITED(exitstatus)) {                                 //if process specified by pid stopped, report exitstatus and remove process from list of background tasks
        printf("Exitstatus [%s] = %d\n", cmd, exitstatus);
        char *buf = "";     //buffer that needs to be given to removeElement
        removeElement(pid, buf, 0);
    }
    return 0;   //traverse through whole list (return value of 0 indicates going to next element in list)
}

void printcwd(){
    char *current_directory = malloc(MAX_DIRECTORY_LENGTH_CONST * sizeof(char));
    if((current_directory = getcwd(current_directory, MAX_DIRECTORY_LENGTH_CONST + 1)) == NULL) {
        //error handling
        perror("getcwd");
    }else {
        printf("%s: ", current_directory);
    }
}

int readstdin(char *line){
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
        
    char *line = malloc(MAX_INPUT_SIZE+1 * sizeof(char));   //reserving memory for the contents of stdin that will be written to *line
    if (line == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    int (*checkBackgroundProcessPtr) (pid_t, const char *) = &checkBackgroundProcess;

    while(!feof(stdin)) {
        /*collecting zombies of background tasks:*/
        walkList(checkBackgroundProcessPtr);

        /*printing current working directory:*/
        printcwd();

        /*reading arguments:*/
        if(readstdin(line) == -1){   //reading MAX_INPUT_SIZE chars from stdin and saving them to *line
            continue;   //if input was overlength, skip to new command line
        }

        /*backup of current line*/
        char *cmd = malloc(sizeof(char) * strlen(line));
        if(cmd == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(cmd, line);          //copy contents (without the newline character) of previously read stdin to "cmd" to be used later (line is later modified by strtok)
        cmd[strlen(line)-1] = '\0';

        char *last_argument;    //pointer to hold the last argument in line (to check wether it´s a "&")

        /*parsing input:*/
        char *current_argument = malloc(sizeof(char) * MAX_INPUT_SIZE);     //allocates memory for buffer in which each parsed argument will reside until it´s added to args array (each argument cant be bigger than the maximum amount of input characters so that´s the upper limit)
        if(current_argument == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    
        current_argument = strtok(line, DELIMITER_CHARS);    
        if(!current_argument) {
            //line was empty
            fprintf(stderr, "No argument was passed\n");
            continue;
        }

        args[0] = malloc(sizeof(char) * strlen(current_argument));  //reserves memory in argument array to hold first argument
        if(args[0] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        args[0] = current_argument;     //adding command name to first position in arguments array

        for (int i = 1; i < sizeof(args); i++) {
            current_argument = strtok(NULL, DELIMITER_CHARS);

            //if last argument is reached, set NULL character as last object in args array and break from parsing
            if(!current_argument) {
                last_argument = malloc(sizeof(char) * strlen(args[i-1]));
                if(last_argument == NULL) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                last_argument = args[i-1];   //assigning last real argument to the variable last_argument
                
                if(strcmp(last_argument, "&") == 0) {
                    //if the last argument is a token indicating a background task, remove it from the array of arguments
                    args[i-1] = NULL;
                }else{
                    args[i] = NULL;
                }
                break;
            }
            //otherwise copy argument into args array
            args[i] = malloc(sizeof(char) * strlen(current_argument));  //allocates memory in args array for the argument string
            if(args[i] == NULL) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            args[i] = current_argument;
        }

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
            if(strcmp(last_argument, "&") == 0) {
                //background task
                insertElement(pid, cmd);       //add background task to list of background tasks
                continue;
            }else{
                //foreground task -> suspend thread until child terminates
                int exitstatus = 0;
                wait(&exitstatus);

                if(WIFEXITED(exitstatus)) {
                    printf("Exitstatus [%s] = %d\n", cmd, exitstatus);
                }
            }
        }
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
