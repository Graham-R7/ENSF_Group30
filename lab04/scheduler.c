#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define min(a,b) (((a)<(b))?(a):(b))

// total jobs
int numofjobs = 0;
static int current_id = 0;

struct job {
    // job id is ordered by the arrival; jobs arrived first have smaller job id, always increment by 1
    int id;
    int arrival; // arrival time; safely assume the time unit has the minimal increment of 1
    int length;
    int tickets; // number of tickets for lottery scheduling
    // TODO: add any other metadata you need to track here
    int start_time;
    int remaining_time;
    int completion_time;
    int last_executed_time; 
    int run_start_time;

    struct job *next;
};

// the workload list
struct job *head = NULL;


void append_to(struct job **head_pointer, int arrival, int length, int tickets){

    // TODO: create a new job and init it with proper data
    struct job* new_job = (struct job*) malloc (sizeof(struct job));
    if(!new_job){
        printf("Memory allocation failed");
        return;
    }

    new_job->id = current_id++;
    new_job->arrival = arrival;
    new_job->length = length;
    new_job->remaining_time = length;
    new_job->tickets = tickets;

    new_job->start_time = -1;
    new_job->completion_time = 0;
    new_job->last_executed_time = -1;
    new_job->run_start_time = -1;


    new_job->next = NULL;

    // if list is empty, insert new job at the beginning
    if(*head_pointer == NULL){
        *head_pointer = new_job;
    }else{
        // if list is not empty we'll find the end of the list to append the new job
        struct job* temp = *head_pointer;
        while(temp->next != NULL){
            temp = temp->next;
        }

        temp->next = new_job;
    }

    numofjobs++;
    return;
}


void read_job_config(const char* filename)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int tickets  = 0;

    char* delim = ",";
    char *arrival = NULL;
    char *length = NULL;

    // TODO, error checking
    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    // TODO: if the file is empty, we should just exit with error
    while ((read = getline(&line, &len, fp)) != -1)
    {
        if( line[read-1] == '\n' )
            line[read-1] =0;
        arrival = strtok(line, delim);
        length = strtok(NULL, delim);
        tickets += 100;

        append_to(&head, atoi(arrival), atoi(length), tickets);
    }

    fclose(fp);
    if (line) free(line);
}


void policy_SJF()
{
    printf("Execution trace with SJF:\n");

    // TODO: implement SJF policy

    printf("End of execution with SJF.\n");

}


void policy_STCF()
{
    printf("Execution trace with STCF:\n");

    // TODO: implement STCF policy

    printf("End of execution with STCF.\n");
}


void policy_RR(int slice)
{
    printf("Execution trace with RR:\n");

    // TODO: implement RR policy

    printf("End of execution with RR.\n");
}


void policy_LT(int slice)
{
    printf("Execution trace with LT:\n");

    // Leave this here, it will ensure the scheduling behavior remains deterministic
    srand(42);

    // In the following, you'll need to:
    // Figure out which active job to run first
    // Pick the job with the shortest remaining time
    // Considers jobs in order of arrival, so implicitly breaks ties by choosing the job with the lowest ID

    // To achieve consistency with the tests, you are encouraged to choose the winning ticket as follows:
    // int winning_ticket = rand() % total_tickets;
    // And pick the winning job using the linked list approach discussed in class, or equivalent

    printf("End of execution with LT.\n");

}

/*
void policy_LT(int slice) {
    srand(42);
    printf("Execution trace with LT:\n");

    int current_time = 0;
    int jobs_remaining = numofjobs;
    int total_tickets = 0;

    struct job *temp = head;
    while (temp != NULL) {
        total_tickets += (temp->id + 1) * 100;
        temp->start_time = -1;
        temp = temp->next;
    }

    while (jobs_remaining > 0) {
        int available_tickets = 0;
        int earliest_arrival = -1;
        temp = head;

        while (temp != NULL) {
            if (temp->arrival <= current_time && temp->remaining_time > 0) {
                available_tickets += (temp->id + 1) * 100;
            } else if (temp->arrival > current_time && (earliest_arrival == -1 || temp->arrival < earliest_arrival)) {
                earliest_arrival = temp->arrival;
            }
            temp = temp->next;
        }

        if (available_tickets == 0 && earliest_arrival > current_time) {
            current_time = earliest_arrival;
            continue;
        }

        int winning_ticket = rand() % available_tickets;
        int ticket_counter = 0;

        struct job *selected_job = NULL;
        temp = head;

        while (temp != NULL) {
            if (temp->arrival <= current_time && temp->remaining_time > 0) {
                ticket_counter += (temp->id + 1) * 100;
                if (ticket_counter > winning_ticket) {
                    selected_job = temp;
                    break;
                }
            }
            temp = temp->next;
        }

        if (selected_job->start_time == -1) {
            selected_job->start_time = current_time;
        }

        int run_time = min(slice, selected_job->remaining_time);
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", current_time, selected_job->id, selected_job->arrival, run_time);

        current_time += run_time;
        selected_job->remaining_time -= run_time;

        if (selected_job->remaining_time == 0) {
            selected_job->completion_time = current_time;
            jobs_remaining--;
            total_tickets -= (selected_job->id + 1) * 100;
        }
    }

    printf("End of execution with LT.\n");
}
*/

void policy_FIFO(){
    printf("Execution trace with FIFO:\n");

    // TODO: implement FIFO policy

    printf("End of execution with FIFO.\n");
}


int main(int argc, char **argv){

    static char usage[] = "usage: %s analysis policy slice trace\n";

    int analysis;
    char *pname;
    char *tname;
    int slice;


    if (argc < 5)
    {
        fprintf(stderr, "missing variables\n");
        fprintf(stderr, usage, argv[0]);
		exit(1);
    }

    // if 0, we don't analysis the performance
    analysis = atoi(argv[1]);

    // policy name
    pname = argv[2];

    // time slice, only valid for RR
    slice = atoi(argv[3]);

    // workload trace
    tname = argv[4];

    read_job_config(tname);

    if (strcmp(pname, "FIFO") == 0){
        policy_FIFO();
        if (analysis == 1){
            // TODO: perform analysis
        }
    }
    else if (strcmp(pname, "SJF") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "STCF") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "RR") == 0)
    {
        // TODO
    }
    else if (strcmp(pname, "LT") == 0)
    {
        // TODO
    }

	exit(0);
}