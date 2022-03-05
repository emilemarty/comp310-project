#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7;

int help();

int quit();

int badcommand();

int set(char *var, char *value);

int print(char *var);

int run(char *script);

int badcommandTooManyTokens();

int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1) {
        return badcommand();
    }
    if (args_size > MAX_ARGS_SIZE) {
        return badcommandTooManyTokens();
    }

    for (i = 0; i < args_size; i++) { // strip spaces new line etc
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        // help
        if (args_size != 1)
            return badcommand();
        return help();
    } else if (strcmp(command_args[0], "quit") == 0) {
        // quit
        if (args_size != 1)
            return badcommand();
        return quit();
    } else if (strcmp(command_args[0], "set") == 0) {
        // set
        if (args_size > 7)
            return badcommandTooManyTokens();
        if (args_size < 3)
            return badcommand();

        char *space = " ";
        char buffer[500];
        strcpy(buffer, command_args[2]);
        for (i = 3; i < args_size; i++) {
            strcat(buffer, space);
            strcat(buffer, command_args[i]);
        }

        return set(command_args[1], buffer);
    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand();

        char token[100];
        strcpy(token, command_args[1]);

        if (token[0] == '$') {
            char buffer[500];
            strcpy(buffer, mem_get_value(token + 1));

            if (strcmp(buffer, "Variable does not exist") == 0) {
                printf("\n");
            } else {
                printf("%s\n", buffer);
            }
            return 0;
        } else {
            printf("%s\n", token);
            return 0;
        }
    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1)
            return badcommand();
        return system("ls -1b");
    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size != 2)
            return badcommand();
        return run(command_args[1]);
    } else if (strcmp(command_args[0], "exec") == 0) {
        if (args_size > 5)
            return badcommandTooManyTokens();

        switch (args_size) {
            case 3:

                break;

            case 4:

                break;

            case 5:

                break;

            default:
                badcommand();
        }

        return run(command_args[1]);
    } else
        return badcommand();
}

int help() {

    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
echo STRING		Displays the STRING passed to echo\n \
print VAR		Displays the STRING assigned to VAR\n \
my_ls			Lists all the files present in the current directory\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n \
exec prog1 prog2	Executes up to 3 concurrent programs, according to a \n \
     prog3 POLICY	given scheduling policy: FCFS, SJF, RR, or AGING";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("%s\n", "Bye!");
    exit(0);
}

int badcommand() {
    printf("%s\n", "Unknown Command");
    return 1;
}

int badcommandTooManyTokens() {
    printf("%s\n", "Bad command: Too many tokens");
    return 2;
}

// For run command only
int badcommandFileDoesNotExist() {
    printf("%s\n", "Bad command: File not found");
    return 3;
}

int set(char *var, char *value) {

    char *link = "=";
    char buffer[1000];
    strcpy(buffer, var);
    strcat(buffer, link);
    strcat(buffer, value);

    mem_set_value(var, value);

    return 0;
}

int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int policyCheck(char *policy) {
    if (strcmp(policy, "FCFS") == 0)
        return 1;
    if (strcmp(policy, "SJF") == 0)
        return 2;
    if (strcmp(policy, "RR") == 0)
        return 3;
    if (strcmp(policy, "AGING") == 0)
        return 4;
    return -1;
}

int run(char *script) {
    struct PCB newProcess;
    PCB_init(&newProcess);

    int line_number = 0;
    char line[1000];
    char buffer[18];
    FILE *p = fopen(script, "rt"); // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, 999, p);
    while (1) {
        snprintf(buffer, 18, "%010dline%03d", newProcess.PID, line_number);
        mem_set_value(buffer, line);
        if (line_number == 0) {
            newProcess.current = line_number;
            strcpy(newProcess.start, buffer);
        }
        line_number++;

        // errCode = parseInput(line); // which calls interpreter()

        memset(line, 0, sizeof(line));
        memset(buffer, 0, sizeof(buffer));

        if (feof(p)) {
            break;
        }
        fgets(line, 999, p);
    }

    fclose(p);
    newProcess.length = line_number;

    enqueue(&newProcess, 1);
    int errCode = schedule(1);
    cleanup(&newProcess);
    return errCode;
}
