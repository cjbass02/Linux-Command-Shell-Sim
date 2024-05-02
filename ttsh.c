/**
 * @file main.c
 * @brief Program entry point.  Runs the teeny tiny shell
 *
 * Course: CSC3210
 * Section: 002
 * Assignment: Teeny Tiny Shell
 * Name: Christian Basso
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_MAX 256
#define CMD_MAX 5

/**
 * @brief Reads a string of user input
 * @param dest - buffer for user input string
 * @return 0 on success
 * @return -1 on error
 */
int read_cmd_string(char dest[INPUT_MAX]) {

    // Read user input
    if(fgets(dest, INPUT_MAX, stdin) == NULL) {
        fprintf(stderr, "Unable to read user input\n");
        return -1;
    }

    // Remove trailing return character
    int len = strlen(dest);
    if(dest[len-1] == '\n') {
        dest[len - 1] = '\0';
    }

    return 0;
}

/**
 * @brief Parses a string and divides it into individual commands
 * @param input - string containing user input
 * @param cmd_strs - the target array for command strings
 * @return the number of commands found in the input
 * @return -1 on error
 */
int parse_commands(char input[INPUT_MAX], char cmd_strs[CMD_MAX][INPUT_MAX]) {

    // Chop the input into command strings
    int cmd_count = 0;
    char* cmd_ptr = strtok(input, ";");
    while(cmd_ptr) {
        if(cmd_count >= CMD_MAX) {
            fprintf(stderr, "Too many commands\n");
            return -1;
        }
        strncpy(cmd_strs[cmd_count], cmd_ptr, INPUT_MAX);
        cmd_count++;
        cmd_ptr = strtok(NULL, ";");
    }

    return cmd_count;
}

int parse_pipe(char command[INPUT_MAX], char pipe_strs[5][INPUT_MAX]) {
    // Chop the input into pipe command strings
    int pipe_count = 0;
    char* pipe_ptr = strtok(command, "|");
    while(pipe_ptr) {
        if(pipe_count >= 5) {
            fprintf(stderr, "Too many pipes\n");
            return -1;
        }
        strncpy(pipe_strs[pipe_count], pipe_ptr, INPUT_MAX);
        pipe_count++;
        pipe_ptr = strtok(NULL, "|");
    }

    return pipe_count;
}



int handle_pipe(char command[INPUT_MAX]) {
    //setup the array for the pipe commands
    char pipe_strs[CMD_MAX][INPUT_MAX];
    // Split up the commands by the | character
    int pipe_count = parse_pipe(command, pipe_strs);
    if (pipe_count == -1) {
        return -1;
    }

    // Create file descriptors for the pipe and the current input file descriptor (comes in handy
    // with deciding which end of the pipe to use for input and output)
    int i, fd[2], in_fd = 0;

    // For each pipe command
    for (i = 0; i < pipe_count; i++) {
        // Parse the arguments
        char* args[11];
        int arg_count = 0;
        char* arg_ptr = strtok(pipe_strs[i], " ");
        while (arg_ptr) {
            if (arg_count >= 10) {
                printf("Too many arguments\n");
                return -1;
            }
            args[arg_count++] = arg_ptr;
            arg_ptr = strtok(NULL, " ");
        }
        args[arg_count] = NULL;


        // Create a pipe
        pipe(fd);

        // Fork a process
        int pid = fork();
        if (pid == 0) {
            // Child
            // If not the first command, set the input file descriptor to the read end of the pipe
            if (in_fd != 0) {
                // Copy the current input method to the read end of the pipe
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }
            // If not the last command, set the output file descriptor to the write end of the pipe
            if (i < pipe_count - 1) {
                dup2(fd[1], STDOUT_FILENO);
            }
            // Close ends of the pipe
            close(fd[0]);
            close(fd[1]);

            // Execute the command
            if (execvp(args[0], args) == -1) {
                printf("Error: could not execute command\n");
                exit(EXIT_FAILURE);
            }
        } else {
            //Parent
            // wait for child to finish
            wait(NULL);

            // If not the first command, close the input file descriptor
            if (in_fd != 0) {
                close(in_fd);
            }

            // Set the input file descriptor to the read end of the pipe
            in_fd = fd[0];
            close(fd[1]);
        }
    }

    return 0;
}




/**
 * @brief Handles running one command from user input
 */

int run_command() {

    char user_input[INPUT_MAX];
    char cmd_strs[CMD_MAX][INPUT_MAX];

    // TODO need to be able to get input from
    //    the user in a loop
    

    // Print the input prompt
    printf("$> ");
    scanf("$> %s", user_input);
    

    
    
    
    // Read user input
    if(read_cmd_string(user_input) == -1) {
        return 1;
    }


    // TODO: Figure out how to handle the 'quit' command
    if(strcmp(user_input, "quit") == 0 || strcmp(user_input, "Quit") == 0){
        return -1;
    }


    // Chop the input into command strings
    int cmd_count = parse_commands(user_input, cmd_strs);
    if(cmd_count == -1) {
        return 1;
    }


    // At this point, cmd_strs holds an array of all command strings with their arguments

    // Chop the commands into arguments and execute one at a time
    for(int i = 0; i < cmd_count; i++) {
        // Example of a command at this point, "ls | grep ap | grep a"
        // After tokenized, "[ls], [grep ap], [grep a]"

        // If pipe (AKA if cmd_strs[i] contains "|")
        if(strstr(cmd_strs[i], "|") != NULL) {
            if(-1== handle_pipe(cmd_strs[i])) {
                return -1;
            }
        //if not pipe
        } else {
            //    1) Chop the command into command line arguments
            char* args[11];
            int arg_count = 0;
            char* arg_ptr = strtok(cmd_strs[i], " ");
            while(arg_ptr) {
                if(arg_count >= 10) {
                    printf("Too many arguments\n");
                    return 1;
                }
                args[arg_count] = arg_ptr;
                arg_count++;
                arg_ptr = strtok(NULL, " ");
            }
            args[arg_count] = NULL;

            

            
            //    2) fork a process
            int pid = fork();
            //    3) execute the command with execvp
            if(pid == -1) {
                printf("Error: could not fork\n");
                return 1;
            } else if (pid == 0) {
                
                if( -1 == execvp(args[0], args)) {
                    printf("Unknown Command \n");
                    return 1;
                }
            } else {
                //    4) wait for the child process to complete
                wait(NULL);
            }
        }

    }
    return 0;

}


/**
 * @brief Program entry procedure for the shell, loops throguh running commands until the user exits
 */
int main() {
    int quit_flag = 0;

    while(1) {
        quit_flag = run_command();
        if(quit_flag == -1){
            return 0;
        }
    }
    

    return -1;
}

/**
 * Improvements for future: 
 * This was a good lab. The only real trouble I had was understanding that I could just make a new pipe for every | I see in a command.
 * That is, I can just make the new pipe in the loop where I look at each piped command, they dont need to be unique.
 * I though I would have to make all my pipes before a loop. 
 * This lab took just the right amount of brain power to make work, so I give it a 10/10 on difficulty and fun.
*/