#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "shellmemory.h"
#include "shell.h"

int counter = 0;

struct memory_struct {
    char *var;
    char *value;
};

int partition = FRAMESIZE;
int history_tail;
int framehistory[FRAMESIZE];
struct memory_struct shellmemory[1000];
struct PCB head; // head of the priority queue

// Helper functions
void printMem() {
    for (int i = 0; i < 1000; i++) {
        printf("[%u]\t%s\t%s\n", i, shellmemory[i].var, shellmemory[i].value);
    }
}

void printQueue() {
    struct PCB *cur = &head;
    printf("\t Ready Queue: \nHead\n");
    while (cur->nextProc != NULL) {
        cur = cur->nextProc;
        printf("[PID = %u]    Current instruction = %u    Length = %u    Score = %i   \n",
               cur->PID, cur->current, cur->length, cur->score);
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
    for (i = 0; i < partition/3; i++) {
        framehistory[i] = -1;
    }
    history_tail = 0;
}

// Set key value pair
int mem_set_value(char *var_in, char *value_in) {
    int i;
    for (i = partition; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].value = strdup(value_in);
            return 1;
        }
    }
    // Value does not exist, need to find a free spot.
    for (i = partition; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            return 1;
        }
    }
    return 0;
}

int mem_delete_var(char *var_in) {
    int i;
    for (i = partition; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            shellmemory[i].var = "none";
            shellmemory[i].value = "none";
            return 1;
        }
    }
    return 0;
}

// get value based on input key
char *mem_get_value(char *var_in) {
    int i;
    for (i = partition; i < 1000; i++) {
        if (strcmp(shellmemory[i].var, var_in) == 0) {
            return strdup(shellmemory[i].value);
        }
    }
    return "Variable does not exist";
}

int frame_set(int PID, int page_num, char *line1, char *line2, char *line3) {
    char buffer[18];
    snprintf(buffer, 18, "%010dpage%03d", PID, page_num);
    int i;
    for (i = 0; i < partition; i = i + 3) {
        if (strcmp(shellmemory[i].var, "none") == 0) {
            shellmemory[i].var = strdup(buffer);
            shellmemory[i + 1].var = strdup(buffer);
            shellmemory[i + 2].var = strdup(buffer);

            shellmemory[i].value = strdup(line1);
            if (strcmp("", line2) == 0)
                shellmemory[i + 1].value = "none";
            else
                shellmemory[i + 1].value = strdup(line2);
            if (strcmp("", line3) == 0)
                shellmemory[i + 2].value = "none";
            else
                shellmemory[i + 2].value = strdup(line3);

            return i;
        }
    }
    return -1;
}

int frame_delete(int frame_num) {
    if (frame_num % 3 != 0)
        return 0;
    int i = frame_num;
    shellmemory[i].var = "none";
    shellmemory[i].value = "none";
    shellmemory[i + 1].var = "none";
    shellmemory[i + 1].value = "none";
    shellmemory[i + 2].var = "none";
    shellmemory[i + 2].value = "none";
    return 1;
}

void frame_load(PCB *process, int page_num) {
    srand((unsigned int) time(NULL));
    FILE *fp = fopen("backing_store", "rt");
    char line1[1000];
    char line2[1000];
    char line3[1000];
    int start = process->start + (page_num * 3);
    int length = process->length;
    int loadCount = 0;
    if (page_num + 1 == (length + 2) / 3)   // if we are loading the final page
        loadCount = ((length - 1) % 3) + 1;
    else
        loadCount = 3;

    for (int j = 0; j < start; j++) {
        fgets(line1, 999, fp);
    }
    memset(line1, 0, sizeof(line1));
    fgets(line1, 999, fp);
    if (loadCount >= 2)
        fgets(line2, 999, fp);
    if (loadCount >= 3)
        fgets(line3, 999, fp);
    int loc = frame_set(process->PID, page_num, line1, line2, line3);
    if (loc == -1) {    // Need to evict
        printf("%s\n", "Page fault! Victim page contents:");
        int r = history_LRU();
        printf("%s", shellmemory[r * 3].value);
        printf("%s", shellmemory[r * 3 + 1].value);
        printf("%s", shellmemory[r * 3 + 2].value);
        char *token = strtok(shellmemory[r * 3].var, "page");
        int PID = atoi(token);
        token = strtok(NULL, "page");
        int index = atoi(token);
        PCB *cur = &head;
        while (cur->nextProc != NULL) {
            cur = cur->nextProc;
            if (cur->PID == PID) {
                cur->pagetable[index] = -1;
                break;
            }
        }
        frame_delete(r);
        loc = frame_set(process->PID, page_num, line1, line2, line3);
        printf("%s\n", "End of victim page contents.");
    }
    process->pagetable[page_num] = loc;
    fclose(fp);
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
            if (process->length < cur->length) {
                prev->nextProc = process;
                process->nextProc = cur;
                inserted = 1;
                break;
            }
        }
        if (inserted == 0)
            cur->nextProc = process;
    } else if (policy == 4) {  // AGING: order the ready list by (increasing) process score
        int inserted = 0;
        while (cur->nextProc != NULL) {
            PCB *prev = cur;
            cur = cur->nextProc;
            if (process->score < cur->score) {
                prev->nextProc = process;
                process->nextProc = cur;
                inserted = 1;
                break;
            }
        }
        if (inserted == 0)
            cur->nextProc = process;
    } else {  // append to end of queue
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
    PCB *cur = &head;
    if (cur->nextProc == NULL) {    // base case
        return 0;
    }
    cur = cur->nextProc;
    int length = cur->length;

    int page_fault = 0;

    if (policy == 3) {      // Round-robin
        int startAt = cur->current;
        int i;
        for (i = startAt; i < MIN(startAt + 2, length); i++) {
            int frame_num = cur->pagetable[i / 3];
            if (frame_num == -1) {
                page_fault = 1;
                cur->current = i;
                dequeue();
                enqueue(cur, policy);
                frame_load(cur, i / 3);
                break;
            } else
                parseInput(shellmemory[frame_num + (i % 3)].value);
            history_write(frame_num);
        }
        if (page_fault == 0) {
            if (startAt + 2 >= length) {   // program complete
                dequeue();
            } else {
                cur->current = startAt + 2;
                dequeue();
                enqueue(cur, policy);
            }
        }
        page_fault = 0;
    } else if (policy == 4) {      // Aging
        int startAt = cur->current;
        int frame_num = cur->pagetable[startAt / 3];
        if (frame_num == -1) {
            page_fault = 1;
            cur->current = startAt;
            dequeue();
            enqueue(cur, policy);
            frame_load(cur, startAt / 3);
        } else
            parseInput(shellmemory[frame_num + (startAt % 3)].value);
        history_write(frame_num);
        if (page_fault == 0) {
            if (startAt + 1 >= length) {   // program complete
                dequeue();
            } else {
                cur->current = startAt + 1;
                PCB *temp = cur;
                int promote = 0;
                int curAge = temp->score;

                while (cur->nextProc != NULL) {
                    cur = cur->nextProc;
                    int age = cur->score;
                    if (age > 0)
                        cur->score = age - 1;
                }
                cur = temp;
                while (cur->nextProc != NULL) {
                    cur = cur->nextProc;
                    if (cur->score < curAge) {
                        promote = 1;
                        break;
                    }
                }
                if (promote == 1) {
                    dequeue();
                    enqueue(temp, policy);
                }
            }
        }
        page_fault = 0;
    } else {
        for (int i = cur->current; i < length; i++) {
            int frame_num = cur->pagetable[i / 3];
            if (frame_num == -1) {
                page_fault = 1;
                cur->current = i;
                frame_load(cur, i / 3);
                dequeue();
                enqueue(cur, policy);
                break;
            } else
                parseInput(shellmemory[frame_num + (i % 3)].value);
            history_write(frame_num);
        }
        if (page_fault == 0)
            dequeue();
        page_fault = 0;
    }
    return schedule(policy);
}

void history_write(int frame_number) {
    for (int i = 0; i < history_tail; i++) {
        if (framehistory[i] == frame_number)
            framehistory[i] = -1;
    }
    framehistory[history_tail] = frame_number;
    history_tail++;
}

int history_LRU() {
    int i;
    for (i = 0; i < history_tail; i++) {
        if (framehistory[i] != -1)
            break;
    }
    int LRU = framehistory[i];
    for (i = 0; i < history_tail; i++) {
        if (framehistory[i] == LRU)
            framehistory[i] = -1;
    }
    return LRU;
}

void cleanup(PCB *process) {
    char buffer[18];
    int length = process->length;
    for (int i = 0; i < length; i++) {  // Clean-up: remove script source code from memory
        snprintf(buffer, 18, "%010dline%03d", process->PID, i);
        mem_delete_var(buffer);
    }
}

void clear_variables() {
    for (int i = partition; i < 1000; i++) {
        shellmemory[i].var = "none";
        shellmemory[i].value = "none";
    }
}