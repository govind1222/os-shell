#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>

void errorCall();

//bool checkExecutable(char * );

int main() {

    char *args[30];
    char *buffer;
    size_t size = 50;
    char *token;
    char *path[30];

    buffer = (char *) malloc(size * sizeof(char));
    path[0] = "/usr/bin";
    path[1] = "/bin";
    path[2] = NULL;

    if (buffer == NULL) {
        errorCall();
        exit(1);
    }


    printf("dash> ");
    while (1) {

        getline(&buffer, &size, stdin);
        int argsSize = 0;
        token = strtok(buffer, " ");
        while (argsSize < 30 && token != NULL) {
            args[argsSize++] = token;
            token = strtok(NULL, " ");
        }

        if (argsSize >= 30) args[29] = NULL;
        else args[argsSize] = NULL;


        // checking to see if command is built in

        if (strcmp(args[0], "exit") == 0) {

            // error to pass in arguments
            if (args[1] != NULL) {
                errorCall();
            } else exit(0);

        } else if (strcmp(args[0], "cd") == 0) {

            if (argsSize == 1 || argsSize > 2) {
                errorCall();
            } else {
                if (chdir(args[1]) != 0) {
                    errorCall();
                }
            }

        } else if (strcmp(args[0], "path") == 0) {
            // TODO - finish later
        }

        //check if executable can be found

        int j = 0;
        int s = 0;
        while (path[j] != NULL) {
            /* char execPath[100];
             strcat(execPath, path[j++]);
             strcat(execPath, "/");
             strcat(execPath, args[0]);
             strcat(execPath,"\0");
             printf("%s\n", execPath);*/
            if (access("/bin/ls", X_OK) == 0) {
                s++;
                strcpy(args[0], "/bin/ls");
                execv(args[0], args);
                //TODO replace with close after fork() implementation
                break;

            }
            break;
        }

        printf("%d worked\n", s);

        /*
        int rc = fork();
        if(rc == 0) {

         close(1);
         execv(args[0], args);

        } else {
         int wc = wait(NULL);
         printf("dash> ");
        }*/

        exit(1);

    }

    return 0;

}

// error message
void errorCall() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}