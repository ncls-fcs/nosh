#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define MAX_DIRECTORY_LENGTH 100
const size_t MAX_DIRECTORY_LENGTH_CONST = 100;

int main(int argc, char const *argv[]) {
    char *current_directory = malloc(MAX_DIRECTORY_LENGTH_CONST * sizeof(char));
    if((current_directory = getcwd(current_directory, MAX_DIRECTORY_LENGTH_CONST + 1)) == NULL) {
        //error handling
        perror("getcwd");
    }else {
        printf("%s:", current_directory);
    } 


    return 0;
}

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
