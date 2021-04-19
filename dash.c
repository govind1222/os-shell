#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define REDIRECTION_ERROR -2

// prints error message to stderr
void errorCall();

// checks to see if the redirection operator is present
// in the buffer
int checkRedirection(char* args[], int* argsSize);

// checks to see if the command is built in and executes
void builtInCommands(char* args[], char* path[], int argsSize);

// returns the number of elements in the args array
int getSize(char* args[]);

//executes the command if not built in
void executeCommand(char *args[], char* path[], int argsSize);

// file pointer to alternative output
// used in cases of redirection
FILE *output;

int main(int argc, char* argv[]) {

    int numParallels = 0;
    char *args[30];
    char *path[30];
    char *commandsMatrix[30][30];
    char *buffer;
    size_t size = 50;
    FILE *fptr = NULL;
    output = NULL;

    // checks to see if CLI is being run in batch more
    // or interactive mode
    if(argc > 1) {
        if(argc > 2) {
            errorCall();
            exit(0);
        }
        fptr = fopen(argv[1], "r");
        if(fptr == NULL) {
            errorCall();
            exit(0);
        }

        // checks to see if batch file is bad
        fseek(fptr, 0, SEEK_END);
        if(ftell(fptr) == 0) {
            errorCall();
            exit(0);
        }
        fseek(fptr, 0, SEEK_SET);
    }

    // initializes buffer
    buffer = (char *)malloc(size * sizeof(char));

    // sets an initial directory
    path[0] = "/bin";
    path[1] = NULL;

    // buffer memory allocation failed
    if(buffer == NULL) {
        errorCall();
        exit(1);
    }

    if(fptr == NULL) printf("dash> ");
    while(1) {
        buffer = (char *)malloc(size * sizeof(char));
        output = NULL;

        // gets input, either from user or file
        if (fptr != NULL && feof(fptr) != EOF) {
            getline(&buffer, &size, fptr);
        } else {
            getline(&buffer, &size, stdin);
        }

        // buffer empty
        if(strlen(buffer) == 0)
            continue;

        char *arg;
        char *command;

        int m = 0, status = 0, commandSize = 0;
        numParallels = 0;

        strtok(buffer, "\n\t\r");
        command = strtok(buffer, "&");
        if(!command) {
            errorCall();
            continue;
        }

        // splits up in input into individual commands
        // e.g. "cd src/test & myls & pwd & uname" becomes
        // ["cd src/test", "myls", "pwd", "uname"]
        while(command) {
            args[numParallels++] = command;
            command = strtok(NULL, "&");
        }

        int forkLocations[numParallels];
        // splits up individual commands into array
        // e.g. ["cd src/test"] becomes ["cd", "src/test"]
        while(m < numParallels){
            arg = strtok(args[m], " ");
            commandSize = 0;
            while(arg){
                commandsMatrix[m][commandSize++] = arg;
                arg = strtok(NULL, " ");
            }
            commandsMatrix[m++][commandSize] = NULL;
        }

        // after parsing input into commands
        // it starts executing
        for(m = 0; m < numParallels; m++) {
            int argsSize = getSize(commandsMatrix[m]);

            if(commandsMatrix[m][0] == NULL) continue;

            // checking to see if command is built in
            if(strcmp(commandsMatrix[m][0], "cd") == 0 ||
               strcmp(commandsMatrix[m][0], "exit") == 0 ||
               strcmp(commandsMatrix[m][0], "path") == 0) {
                builtInCommands(commandsMatrix[m], path, argsSize);
                goto printPrompt;
            }

            // creates a child process for each command
            // runs commands in parallel
            if((forkLocations[m] = fork()) == 0) {
                executeCommand(commandsMatrix[m], path, argsSize);
                goto printPrompt;
            }

            printPrompt:

            if(m == numParallels - 1 && fptr == NULL) {
                printf("dash> ");
            }
        }

        // waits for all child processes to terminate
        pid_t wpid;
        while((wpid = wait(&status)) > 0);
    }
    return 0;
}

//executes the command if not built in
void executeCommand(char *args[], char *path[], int argsSize) {

    // redirection holds the index of the ">" symbol
    // 0 if it doesn't exist
    // -2 if it has incorrect format
    int redirection = checkRedirection(args, &argsSize);

    if(redirection == REDIRECTION_ERROR) {
        errorCall();
        return;
    }

    // redirects output to specified file
    if(redirection > 0) {
        output = freopen(args[redirection + 1], "w+", stdout);
        freopen(args[redirection + 1], "w+", stderr);
        args[redirection] = NULL;
        args[redirection + 1] = NULL;
    }

    // checks to see if executable can be found in
    // specified path directories
    // else prints out error message
    int j = 0, s = 0;
    while(path[j] != NULL) {
        int execPathSize = 2 + strlen(path[j]) + strlen(args[0]);
        char *execPath = (char *)malloc(execPathSize);
        snprintf(execPath, execPathSize, "%s%s%s", path[j++], "/", args[0]);
        strtok(execPath, "\n");
        if(access(execPath, X_OK)  == 0) {
            s++;
            execv(execPath, args);
            break;
        }
    }

    if(!s) {
        errorCall();
    }
    fclose(output);
    return;
}

// prints error message to stderr
void errorCall() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// checks to see if the redirection operator is present
// in the buffer
int checkRedirection(char * args[], int *argsSize) {

    // counts the number of times ">" occurs
    int count = 0;
    int m;
    for(m = 0; args[m]; m++) {
        int len;
        for(len = 0; len < strlen(args[m]); len++) {
            if(args[m][len] == '>') {
                count++;
            }
        }
    }

    // more than 1 redirection symbol is an error
    if(count >= 2) {
        return REDIRECTION_ERROR;
    }

    // no redirection symbol
    // return 0 to indicate output goes to stdout
    if(count == 0) {
        return 0;
    }

    // at this point in the method, we know there has
    // to be at least one redirection symbol
    if(*argsSize == 1) {
        char *token = strtok(args[0], ">");
        char *nextToken = strtok(NULL, ">");
        // if either token is null, then > symbol
        // was not located in the middle - error
        if(token == NULL || nextToken == NULL ) {
            return REDIRECTION_ERROR;
        }

        // restructue the argument array
        // e.g. ["ls>output"] => ["ls", ">", "output"]
        args[0] = token;
        args[1] = ">";
        args[2] = (char *)malloc(strlen(nextToken)*sizeof(char));
        args[2] = nextToken;
        args[3] = NULL;
        *argsSize = 3;
        return 1;
    }

    // if the ">" redirection symbol is already split up
    if(strlen(args[*argsSize-2]) == 1 && args[*argsSize-2][0] == '>') {
        return *argsSize - 2;
    }

    // if the ">" redirection symbol is the first character or the last
    // it is an error - return -2
    int length = strlen(args[*argsSize-1]);
    if(args[*argsSize-2][0] == '>' || args[*argsSize-1][length-1] == '>') {
        return -2;
    }

    // this is the case where the ">" is arranged like this:
    // ls> output ==> ls > output
    length = strlen(args[*argsSize-2]);
    if(args[*argsSize-2][length-1] == '>') {
        int index = *argsSize;
        args[*argsSize-2][length-1] = '\0';
        args[*argsSize] = (char *)malloc(strlen(args[*argsSize-1]) * sizeof(char));
        args[*argsSize] = args[*argsSize-1];
        args[*argsSize-1] = ">";
        args[*argsSize+1]=NULL;
        *argsSize = *argsSize + 1;
        return index - 1;
    }

    // this is the case where the ">" is arranged like this:
    // ls >output
    length = strlen(args[*argsSize-1]);
    if(args[*argsSize-1][0] == '>') {
        int index = *argsSize;
        args[*argsSize] = args[*argsSize-1];
        args[*argsSize]= strtok(args[*argsSize-1], ">");
        args[*argsSize-1] = ">";
        args[*argsSize+1] = NULL;
        *argsSize = index + 1;
        return index-1;
    }

    // this is the case where the ">" is arranged like this:
    // ls src/test>output ==> ls src/test > output
    char *token = strtok(args[*argsSize-1], ">");
    char *nextToken = strtok(NULL, ">");

    if (token != NULL && nextToken != NULL) {
        int index = *argsSize;
        args[*argsSize - 1] = token;
        args[*argsSize] = ">";
        args[*argsSize + 1] = (char *)malloc(strlen(nextToken) * sizeof(char));
        args[*argsSize + 1] = nextToken;
        args[*argsSize + 2] = NULL;
        *argsSize = *argsSize + 2;
        return index;
    }

    // at this point, some format error with ">" symbol
    // return error code
    return REDIRECTION_ERROR;
}

// checks to see if the command is built in and executes
void builtInCommands(char *args[], char *path[], int argsSize) {
    // built in commands
    if(strcmp(args[0], "exit") == 0) {
        if(args[1] != NULL ) {
            errorCall();
        } else {
            exit(0);
        }
    } else if (strcmp(args[0], "cd") == 0) {
        if(argsSize == 1 || argsSize > 2) {
            errorCall();
        } else {
            if(chdir(args[1]) != 0) {
                errorCall();
            }
        }
    } else if(strcmp(args[0], "path") == 0) {
        int pathVar = 0;
        while(pathVar < argsSize - 1 &&
              pathVar < 30 && args[pathVar] != NULL) {
            int length = strlen(args[1 + pathVar]);
            // makes sure there isn't any redundant "/" signs
            if (args[1 + pathVar][length-1] == '/') {
                args[1 + pathVar][length - 1] = '\0';
            }
            path[pathVar] = args[1 + pathVar];
            pathVar++;
        }
        path[pathVar] = NULL;
    }
}

// returns the number of elements in the args array
int getSize(char * args[]) {
    int m;
    for(m = 0; args[m]; m++);
    return m;
}
