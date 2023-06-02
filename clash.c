#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_INPUT_SIZE 100
#define DELIMITER_CHARS "   \n"
const size_t MAX_DIRECTORY_LENGTH_CONST = 100;


int main(int argc, char const *argv[]) {
    
    char args[MAX_INPUT_SIZE][MAX_INPUT_SIZE];      //array to hold arguments as *char (Strings) (there cant be more arguments than input characters so MAX_INPUT_SIZE is the upper bound for number of arguments (and length of each individual argument))
        
    char *line = malloc(MAX_INPUT_SIZE+1 * sizeof(char));   //reserving memory for the contents of stdin that will be written to *line
    if (line == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while(!feof(stdin)) {
        /*printing current working directory:*/
        char *current_directory = malloc(MAX_DIRECTORY_LENGTH_CONST * sizeof(char));
        if((current_directory = getcwd(current_directory, MAX_DIRECTORY_LENGTH_CONST + 1)) == NULL) {
            //error handling
            perror("getcwd");
        }else {
            printf("%s: ", current_directory);
        } 

        /*reading arguments:*/

        //reading MAX_INPUT_SIZE chars from stdin and saving them to *line
        if(fgets(line, MAX_INPUT_SIZE+1, stdin) == NULL){
            /*fgets error handling*/
            if(ferror(stdin)) {     //real error -> terminate with EXIT_FAILURE
                perror("fgets");
                exit(EXIT_FAILURE);
            }else{                  //end of file -> "normal" termination
                break;
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
            continue;
        }
        
        /*parsing input:*/

        char *current_argument;
    
        current_argument = strtok(line, DELIMITER_CHARS);

        if(!current_argument) {
            //line was empty
            fprintf(stderr, "No argument was passed\n");
            continue;
        }
        strcpy(args[0], current_argument);      //copies current argument into args array        
        for (int i = 1; i < sizeof(args); i++) {
            current_argument = strtok(NULL, DELIMITER_CHARS);
            printf("current arg: %s\n", current_argument);
            //if last argument is reached, set NULL character as last object in args array and break from parsing
            if(!current_argument) {
                printf("last arg reached\n");
                char *adress;
                adress = args[i];
                *adress = NULL;
                printf("null char added: %s\n", args[i]);
                break;
            }
            //otherwise copy argument into args array
            strcpy(args[i], current_argument);
        }

        /*print array
        int i = 0;
        printf("[");
        while(args[i] != NULL) {
            printf("%s,", args[i]);
            i++;
        }
        printf("]\n");
        */

        /*starting new process:*/
        pid_t pid = fork();

        if(pid == -1) {
            //forking failed

        }else if (pid == 0) {
            //in child proccess -> executing command
            execvp(args[0], &args);
            //exec only returns on error
            perror("exec");
        }else {
            //in parent proccess
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
