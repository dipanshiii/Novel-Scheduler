#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Job
{
    int job_no;
    int cpu_burst_time;
    int arrival_time;
    int turnaround_time;
    int priority;
    int response_time;
    int waiting_time;
} Job;

// structure to represent a queue node
typedef struct Node
{
    Job job;
    struct Node *next;
} Node;

//  structure to represent a queue
typedef struct Queue
{
    Node *front;
    Node *rear;
} Queue;

void initializeQueue(Queue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}

bool isEmpty(Queue *queue)
{
    return queue->front == NULL;
}

void enqueue(Queue *queue, Job job)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->job = job;
    newNode->next = NULL;

    if (isEmpty(queue))
    {
        queue->front = newNode;
        queue->rear = newNode;
    }
    else
    {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

Job dequeue(Queue *queue)
{
    if (isEmpty(queue))
    {
        Job emptyJob = {0, 0, 0, 0}; 
        // Empty job to signify the queue is empty
        return emptyJob;
    }

    Node *frontNode = queue->front;
    Job job = frontNode->job;

    queue->front = frontNode->next;
    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    free(frontNode);
    return job;
}

// Function to compare priorities for job sorting
int comparePriorities(const void *a, const void *b)
{
    const Job *jobA = (const Job *)a;
    const Job *jobB = (const Job *)b;

    // Lower priority value means higher priority
    return (jobA->priority < jobB->priority) - (jobA->priority > jobB->priority);
}

void readJobsFromFile(const char *filename, Job jobs[], int *num_jobs)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    *num_jobs = 0;
    while (fscanf(file, "%d %d %d", &jobs[*num_jobs].job_no, &jobs[*num_jobs].cpu_burst_time, &jobs[*num_jobs].arrival_time) != EOF)
    {
        jobs[*num_jobs].response_time = -1;
        (*num_jobs)++;
    }
    

    fclose(file);
}

void writeJobMetricsToFile(const char *filename, Job jobs[], int num_jobs)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_jobs; i++)
    {
        fprintf(file, "Job %d - Waiting Time: %d, Response Time: %d, Turnaround Time: %d\n",
                jobs[i].job_no, jobs[i].waiting_time, jobs[i].response_time, jobs[i].turnaround_time);
    }

    fclose(file);
}

int main()
{
    const char *input_filename = "jobs_1.txt"; 
    const char *output_filename = "scheduler_log.txt";
    int num_jobs;
    Job jobs[100]; 

    readJobsFromFile(input_filename, jobs, &num_jobs);


    int current_time = 0;
    int time_slice = 2; 
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalResponseTime = 0;

    Queue ready_queue;
    initializeQueue(&ready_queue);

    
    int waiting_times[num_jobs];
    int response_times[num_jobs];
    int turnaround_times[num_jobs];
    for (int i = 0; i < num_jobs; i++)
    {
        waiting_times[i] = 0;
        response_times[i] = -1;
        turnaround_times[i] = 0;
        if (jobs[i].arrival_time != 0)
        {
            jobs[i].priority = jobs[i].cpu_burst_time / jobs[i].arrival_time;
        }
        else
        {
            
            jobs[i].priority = 0; 
        }
    }

    
    qsort(jobs, num_jobs, sizeof(Job), comparePriorities);

    
    bool all_jobs_completed = false;
    while (!all_jobs_completed)
    {
        all_jobs_completed = true;

        // Dequeue and execute a job based on priority
        for (int i = 0; i < num_jobs; i++)
        {

            if(jobs[i].response_time == -1){
                jobs[i].response_time = current_time - jobs[i].arrival_time;
            }

            if (jobs[i].cpu_burst_time > 0)
            {
                if (jobs[i].cpu_burst_time <= time_slice)
                {
                    current_time += jobs[i].cpu_burst_time;
                    jobs[i].turnaround_time = current_time - jobs[i].arrival_time;
                    jobs[i].waiting_time = jobs[i].turnaround_time-jobs[i].cpu_burst_time;
                    
                    jobs[i].cpu_burst_time = 0;
                }
                else
                {
                    current_time += time_slice;
                    jobs[i].cpu_burst_time -= time_slice;
                    // Update priority for aging
                    jobs[i].priority = (float)jobs[i].cpu_burst_time / jobs[i].arrival_time;
                }
            }

            // Check if any job has remaining burst time
            if (jobs[i].cpu_burst_time > 0)
            {
                all_jobs_completed = false;
            }
        }

        // Sort the jobs based on priority after each iteration
        qsort(jobs, num_jobs, sizeof(Job), comparePriorities);
    }
    for (int i = 0; i < num_jobs; i++)
    {
        totalResponseTime+=jobs[i].response_time;
        totalTurnaroundTime+=jobs[i].turnaround_time;
        totalWaitingTime+=jobs[i].waiting_time;
    }
    

    
    printf("Job Metrics:\n");
    for (int i = 0; i < num_jobs; i++)
    {
        printf("Job %d - Waiting Time: %d, Response Time: %d, Turnaround Time: %d\n",
               jobs[i].job_no, jobs[i].waiting_time, jobs[i].response_time, jobs[i].turnaround_time);
    }

    printf("Total execution time: %d\n", current_time);
    writeJobMetricsToFile(output_filename, jobs, num_jobs);
    FILE *logFile = fopen("scheduler_log.txt", "a");
    if (logFile == NULL) {
        perror("Error opening log file");
        exit(1);
    }
    double avgTurnaroundTime = (double)totalTurnaroundTime / num_jobs;
    double avgResponseTime = (double)totalResponseTime / num_jobs;
    double avgWaitingTime = (double)totalWaitingTime / num_jobs;

    fprintf(logFile, "Average Turnaround Time: %.2lf\n", avgTurnaroundTime);
    fprintf(logFile, "Average Response Time: %.2lf\n", avgResponseTime);
    fprintf(logFile, "Average Waiting Time: %.2lf\n", avgWaitingTime);

    fclose(logFile);

    FILE *datFile = fopen("metricsRR_2.dat", "w");

    fprintf(datFile, "TurnaroundTime: %.2lf\n", avgTurnaroundTime);
    fprintf(datFile, "WaitingTime: %.2lf\n", avgWaitingTime);
    fprintf(datFile, "ResponseTime: %.2lf\n", avgResponseTime);

    //fprintf(datFile, "%.2lf %.2lf %.2lf", avgWaitingTime, avgTurnaroundTime, avgResponseTime);

    fclose(datFile);


    return 0;
}