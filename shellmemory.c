#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "shellmemory.h"
#include "shell.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[1000];
struct PCB head; // head of the priority queue

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++)
        if (*(model + i) == *(var + i))
            matchCount++;
    if (matchCount == len)
        return 1;
    else
        return 0;
}

char *extract(char *model) {
    char token = '='; // look for this to find value
    char value[1000]; // stores the extract value
    int i, j, len = strlen(model);
    for (i = 0; i < len && *(model + i) != token; i++); // loop till we get there
    // extract the value
    for (i = i + 1, j = 0; i < len; i++, j++)
        value[j] = *(model + i);
    value[j] = '\0';
    return strdup(value);
}

// Shell memory functions

void mem_init() {

    int i;
    for (i = 0; i < 1000; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {

    int i;

    for (i = 0; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    // Value does not exist, need to find a free spot.
    for (i = 0; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return;
        }
    }

    return;
}

void mem_delete_var(char *var_in) {

    int i;

    for (i = 0; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].var = "none";
            shellmemory[i].value = "none";
            return;
        }
    }
}

// get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {

            return strdup(shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}

// Scheduler functions

PCB *PCB_init(PCB *process) {
    srand(time(NULL));
    int pass = 0;
    int random;

    while (pass == 0) {
        random = rand();
        PCB *cur = head.nextProc;
        if (cur == NULL)
            break;
        while (cur != NULL) {
            if (cur->PID != random) {
                pass = 1;
            } else {
                pass = 0;
                break;
            }
            cur = cur->nextProc;
        }
    }

    process->PID = random;
    process->nextProc = NULL;

    return process;
}

void readyQueue_init() {
    head.nextProc = NULL;
}

void enqueue(PCB *process) {
    PCB *cur = &head;
    while (cur->nextProc != NULL) {
        cur = cur->nextProc;
    }
    cur->nextProc = process;
}

int schedule() {
    int errCode = 0;
    char buffer[18];
    PCB *cur = &head;
    while (cur->nextProc != NULL) {
        cur = cur->nextProc;
        int length = cur->length;
        for (int i = cur->current; i < length; i++) {
            snprintf(buffer, 18, "%010dline%03d", cur->PID, i);
            errCode = parseInput(mem_get_value(buffer));
        }
        for (int i = 0; i < length; i++) {
            snprintf(buffer, 18, "%010dline%03d", cur->PID, i);
            mem_delete_var(buffer);
        }
    }
    head.nextProc = NULL;
    return errCode;
}