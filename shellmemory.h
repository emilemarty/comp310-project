typedef struct PCB {
    int PID;
    char start[18];   // spot in memory where instructions begin (format: ##########line###)
    int current;      // current instruction to execute
    int length;
    struct PCB *nextProc; // next PCB in the ready queue
} PCB;

void mem_init();

char *mem_get_value(char *var);

void mem_set_value(char *var, char *value);

PCB *PCB_init(PCB *process);

void readyQueue_init();

void enqueue(PCB *process, int policy);

int schedule(int policy);

void cleanup(PCB *process);