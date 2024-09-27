#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

extern char** environ;

// Printing Function: Using Matrix Printing Logic for Environment.
void print_environment(){
    char** temp_environ = environ;
    while (*temp_environ){
        printf("%s", *temp_environ);
        temp_environ++;
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int optionu = 0; // Setting Option u to "Not-Selected"
    int optionv = 0; // Setting Option v to "Not-Selected"
    

    // Checking the selected option using "getopt"
    while ((opt = getopt(argc, argv, "vu:")) != -1) {
        switch (opt) {
        case 'v':
            optionv = 1;
            break;
        case 'u':
            if (optarg[0] == '-') {
                fprintf(stderr, "Invalid argument for -u option: '%s'!\n", optarg);
                return EXIT_FAILURE;
            }
            optionu = 1; 
            // Setting it to 1 here to avoid problems handled by error above.
            if (unsetenv(optarg)) {
                perror("Error Unset Environment var");
                return EXIT_FAILURE;
            }
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [-u name] [name=value]... [command [arg]...]!\n", argv[0]);
            exit(EXIT_FAILURE);
            // Using exit() here since it terminates the programm immediatelly.
            break;
        }
    }

    // If no arguments, print environment.
    if (argc == 1) {
        print_environment();
    }

    // Handling both U and V:
    // Print the removed environment variables to stderr
    if (optionv && optionu) {
        for (int i = 1; i < optind; i++) {
            if (!strcmp(argv[i], "-u")) {
                fprintf(stderr, "Environment Variable Removed: %s!\n", argv[i+1]);
            }
        } 
    }

    // Go through the non-option arguments aka. name=value pairs and commands
    for (int i = optind; i < argc; i++) {
        if (strchr(argv[i], '=')) {
            char *arg_copy = argv[i];
            char *name = strtok(arg_copy, "=");
            char *value = strtok(NULL, "=");

            if ((name == NULL) || (value == NULL)) {
                fprintf(stderr, "Invalid name=value pair!\n");
                return EXIT_FAILURE;
            }

            if (setenv(name, value, 1)) {
                perror("SETENV");
                return EXIT_FAILURE;
            }
            
            if (optionv) {
                fprintf(stderr, "Added %s=%s pair!\n", name, value);
            }
        }
        else {
            // Print the name of the program to be executed to stderr
            if (optionv) {
                fprintf(stderr, "Executing %s!\n", argv[i]);
            }
            execvp(argv[i], &argv[i]);
            perror("execvp");
            return EXIT_FAILURE;
        }
    }

    // If none selected print environment
    if ((optionu == 0) && (optionv == 0)) {
        print_environment();
    }

    return EXIT_SUCCESS;
}