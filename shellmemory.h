typedef struct PCB {
    int PID;
    int start;   // spot in memory where instructions begin (format: ##########line###)
    int current;      // current instruction to execute
    int length;
    int score;
    struct PCB *nextProc; // next PCB in the ready queue
    int *pagetable;
} PCB;

void printMem();

void printQueue();

void mem_init();

char *mem_get_value(char *var);
int mem_delete_var(char *var_in);
int mem_set_value(char *var, char *value);

int frame_set(int PID, int page_num, char *line1, char *line2, char *line3);
int frame_delete(int frame_num);
void frame_load(PCB *process, int page_num);

PCB *PCB_init(PCB *process);

void readyQueue_init();

void enqueue(PCB *process, int policy);

int schedule(int policy);

void cleanup(PCB *process);

void clear_variables();