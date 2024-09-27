#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <getopt.h>

// Max arguments set here using define so that the limit can be changed easily.
#define MAX 100

// Print the command if -t option is set
void print_command(char **args) {
    fprintf(stderr, "Executing: ");
    for (int i = 0; args[i] != NULL; i++) {
        fprintf(stderr, "%s ", args[i]);
    }
    fprintf(stderr, "\n");
}

int main(int argc, char *argv[]) {
    // Option variables.
    int opt;
    int n = -1;
    int j = 1;
    int t_select = 0;
    char *command[MAX] = { "/bin/echo", NULL }; // Default command is /bin/echo
    // Variables for reading lines from stdin
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *args[MAX];
    int arg_count = 0;
    int process_count = 0;

    // Setting opterr to 0 to suppress getopt's automatic error messages
    // Since we want to handle the errors ourself.
    opterr = 0; 
    
    // Go through command line options check.
    while ((opt = getopt(argc, argv, ":n:tj:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 't':
                t_select = 1;
                break;
            case 'j':
                j = atoi(optarg);
                break;
            case '?': // Error Case: Unknown option
                fprintf(stderr, "Error Case: Unknown option: -%c\n", optopt);
                exit(EXIT_FAILURE);
            case ':': // Error Case: Missing option argument
                fprintf(stderr, "Error Case: Option -%c requires an argument\n", optopt);
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "Usage: %s [-n num] [-t] [-j jobs] [command]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // If a command is provided, use it instead of the default
    if (optind < argc) {
        for (int i = optind, j = 0; i < argc && j < MAX - 1; i++, j++) {
            command[j] = argv[i];
        }
        command[argc - optind] = NULL;
    }

    // Reading from stdin
    while ((read = getline(&line, &len, stdin)) != -1) {
        line[read - 1] = '\0'; // Remove newline
        args[arg_count++] = strdup(line); // Store line

        if (n != -1 && arg_count >= n) { // Time to execute the command
            args[arg_count] = NULL; // Terminate argument list

            if (t_select) {
                print_command(command);
            }

            if (fork() == 0) { 
                // Child: 
                execvp(command[0], command); // Execute command
                perror("execvp");
                exit(EXIT_FAILURE);
            } else { 
                // Parent:
                process_count++;
                if (process_count >= j) { 
                    // Wait for a process if max jobs has been reached
                    wait(NULL);
                    process_count--;
                }
            }

            // Reset argument count
            arg_count = 0;
        }
    }

    // Handling remaining arguments
    if (arg_count > 0) {
        args[arg_count] = NULL;
        if (t_select) {
            print_command(command);
        }
        if (fork() == 0) {
            execvp(command[0], command);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            process_count++;
            if (process_count >= j) {
                wait(NULL);
                process_count--;
            }
        }
    }

    // Wait for child to "die"
    while (process_count > 0) {
        wait(NULL);  
        // Ignore the exit status
        process_count--;
    }

    return EXIT_SUCCESS;
}