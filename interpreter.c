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

int policyCheck(char *policy);

int run(char *script);

int badcommandTooManyTokens();

int badcommandFileDoesNotExist();

int exec(char *scripts[3], int length, int policy);

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
        remove("backing_store");
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
    } else if (strcmp(command_args[0], "resetmem") == 0) {
        if (args_size != 1)
            return badcommand();
        clear_variables();
        return 0;
    } else if (strcmp(command_args[0], "run") == 0) {
        if (args_size != 2)
            return badcommand();
        return run(command_args[1]);
    } else if (strcmp(command_args[0], "exec") == 0) {
        if (args_size > 5)
            return badcommandTooManyTokens();
        if (args_size < 3)
            return badcommand();

        int policy = policyCheck(command_args[args_size - 1]);
        if (policy < 0) {
            printf("%s\n", "Bad command: Incorrect policy name");
            return 1;
        }
        char *scripts[3] = {"none", "none", "none"};
        for (int k = 0; k < args_size - 2; k++) {
            scripts[k] = command_args[k + 1];
        }
        return exec(scripts, args_size - 2, policy);
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
resetmem		    Deletes the content of the variable store.\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n \
exec prog1 prog2	Executes up to 3 concurrent programs, according to a \n \
     prog3 POLICY	given scheduling policy: FCFS, SJF, RR, or AGING \n";
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
    FILE *p = fopen(script, "rt"); // the program is in a file
    FILE *b = fopen("backing_store", "w");

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    fgets(line, 999, p);
    newProcess.start = 0;
    newProcess.current = 0;
    while (1) {
        fputs(line, b);
        line_number++;
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
        fgets(line, 999, p);
    }

    fclose(p);
    fclose(b);
    int procLength = line_number;
    newProcess.length = procLength;
    newProcess.score = procLength;
    newProcess.pagetable = malloc(((procLength + 2) / 3) * sizeof(int));   // There are ceiling(length/3) pages
    for (int j = 0; j < (procLength + 2) / 3; j++) {
        newProcess.pagetable[j] = -1;
    }

    FILE *fp = fopen("backing_store", "rt");
    char line1[1000];
    char line2[1000];
    char line3[1000];
    fgets(line1, 999, fp);
    if (procLength >= 2)
        fgets(line2, 999, fp);
    if (procLength >= 3)
        fgets(line3, 999, fp);
    int loc = frame_set(newProcess.PID, 0, line1, line2, line3);
    newProcess.pagetable[0] = loc;
    if (procLength >= 4) {
        memset(line1, 0, sizeof(line1));
        memset(line2, 0, sizeof(line1));
        memset(line3, 0, sizeof(line1));
        fgets(line1, 999, fp);
        if (procLength >= 5)
            fgets(line2, 999, fp);
        if (procLength >= 6)
            fgets(line3, 999, fp);
        int loc = frame_set(newProcess.PID, 1, line1, line2, line3);
        newProcess.pagetable[1] = loc;
    }
    fclose(fp);
    enqueue(&newProcess, 1);
    int errCode = schedule(1);
    cleanup(&newProcess);
    return errCode;
}

int exec(char *scripts[3], int length, int policy) {
    struct PCB proc1;
    struct PCB proc2;
    struct PCB proc3;
    struct PCB *newProcess[3] = {&proc1, &proc2, &proc3};

    for (int i = 0; i < length; i++) {
        PCB_init(newProcess[i]);
    }

    FILE *b = fopen("backing_store", "w");
    int line_number = 0;
    for (int i = 0; i < length; i++) {
        char line[1000];
        FILE *p = fopen(scripts[i], "rt"); // the program is in a file

        if (p == NULL) {
            return badcommandFileDoesNotExist();
        }

        int start = line_number;
        newProcess[i]->start = start;
        newProcess[i]->current = 0;
        fgets(line, 999, p);
        while (1) {
            fputs(line, b);
            line_number++;
            memset(line, 0, sizeof(line));

            if (feof(p)) {
                break;
            }
            fgets(line, 999, p);
        }
        fputs("\n", b);
        fclose(p);
        int procLength = line_number - start;
        newProcess[i]->length = procLength;
        newProcess[i]->score = procLength;
        newProcess[i]->pagetable = malloc(((procLength + 2) / 3) * sizeof(int));   // There are ceiling(length/3) pages
        for (int j = 0; j < (procLength + 2) / 3; j++) {
            newProcess[i]->pagetable[j] = -1;
        }
    }
    fclose(b);
    FILE *fp = fopen("backing_store", "rt");
    char line1[1000];
    char line2[1000];
    char line3[1000];
    for (int i = 0; i < length; i++) {
        int start = newProcess[i]->start;
        int procLength = newProcess[i]->length;
        fseek(fp, 0, SEEK_SET);
        for (int j = 0; j < start; j++) {
            fgets(line1, 999, fp);
        }
        memset(line1, 0, sizeof(line1));
        memset(line2, 0, sizeof(line1));
        memset(line3, 0, sizeof(line1));
        fgets(line1, 999, fp);
        if (procLength >= 2)
            fgets(line2, 999, fp);
        if (procLength >= 3)
            fgets(line3, 999, fp);
        int loc = frame_set(newProcess[i]->PID, 0, line1, line2, line3);
        newProcess[i]->pagetable[0] = loc;
        if (procLength >= 4) {
            memset(line1, 0, sizeof(line1));
            memset(line2, 0, sizeof(line1));
            memset(line3, 0, sizeof(line1));
            fgets(line1, 999, fp);
            if (procLength >= 5)
                fgets(line2, 999, fp);
            if (procLength >= 6)
                fgets(line3, 999, fp);
            int loc = frame_set(newProcess[i]->PID, 1, line1, line2, line3);
            newProcess[i]->pagetable[1] = loc;
        }
        enqueue(newProcess[i], policy);
    }
    fclose(fp);
    int errCode = schedule(policy);
    for (int i = 0; i < length; i++) {
        free(newProcess[i]->pagetable);
    }
    return errCode;
}