#include <string.h>
#include <stdio.h>
#include "shellmemory.h"
#include "shell.h"

int counter = 0;

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[1000];
struct PCB head; // head of the priority queue

// Helper functions
void printMem() {
    for (int i = 0; i < 1000; i++) {
        printf("[%u]\t%s\t%s\n", i, shellmemory[i].var, shellmemory[i].value);
    }
}

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

int MIN(int a, int b) {
    if (b < a)
        return b;
    else
        return a;
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
int mem_set_value(char *var_in, char *value_in) {

    int i;

    for (i = 0; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return 1;
        }
    }

    // Value does not exist, need to find a free spot.
    for (i = 0; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return 1;
        }
    }

    return 0;
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
    process->PID = counter;
    counter++;
    process->nextProc = NULL;

    return process;
}

void readyQueue_init() {
    head.nextProc = NULL;
}

void enqueue(PCB *process, int policy) {
    PCB *cur = &head;
    if (policy == 2) {  // SJF: order the ready list by (increasing) process length
        int inserted = 0;
        while (cur->nextProc != NULL) {
            PCB *prev = cur;
            cur = cur->nextProc;
            if(process->length < cur->length) {
                prev->nextProc = process;
                process->nextProc = cur;
                inserted = 1; break;
            }
        }
        if (inserted == 0)
            cur->nextProc = process;
    }
    else {  // append to end of queue
        while (cur->nextProc != NULL) {
            cur = cur->nextProc;
        }
        cur->nextProc = process;
    }
}

void dequeue() {
    if (head.nextProc == NULL)
        return;
    else {
        PCB *cur = head.nextProc;
        head.nextProc = cur->nextProc;
        cur->nextProc = NULL;
    }
}

int schedule(int policy) {
    char buffer[18];
    PCB *cur = &head;


    if (cur->nextProc == NULL) {    // base case
        return 0;
    }
    cur = cur->nextProc;
    int length = cur->length;

    if (policy == 3) {      // Round-robin
        int startAt = cur->current;
        for(int i = startAt; i < MIN(startAt+2, length); i++) {
            snprintf(buffer, 18, "%010dline%03d", cur->PID, i);
            parseInput(mem_get_value(buffer));
        }
        if(startAt+2 >= length) {   // program complete
            dequeue();
        }
        else {
            cur->current = startAt+2;
            dequeue();
            enqueue(cur, policy);
        }
    }
    else {
        for (int i = 0; i < length; i++) {
            snprintf(buffer, 18, "%010dline%03d", cur->PID, i);
            parseInput(mem_get_value(buffer));
        }
        dequeue();
    }
    return schedule(policy);
}

void cleanup(PCB *process) {
    char buffer[18];
    int length = process->length;
    for (int i = 0; i < length; i++) {  // Clean-up: remove script source code from memory
        snprintf(buffer, 18, "%010dline%03d", process->PID, i);
        mem_delete_var(buffer);
    }
}