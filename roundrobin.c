#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_JOBS 100

typedef struct {
    int jobID;
    int cpuBurst;
    int remainingCpuBurst;
    int ioRequestTime;
    int ioCompletionTime;
    int waitingTime;
    int responseTime;
    int turnaroundTime;
    // int sleepMode;
    int ioTime;
    int io;
    int arrivalTime;
} Job;

void parseJobData(char *line, Job *job) {
    char *token = strtok(line, ",");
    job->jobID = atoi(token);

    token = strtok(NULL, ",");
    job->cpuBurst = atoi(token);
    job->remainingCpuBurst = job->cpuBurst;

    token = strtok(NULL, ",");
    job->ioCompletionTime = atoi(token);

    token = strtok(NULL, ",");
    job->arrivalTime = atoi(token);
}

int main() {
    FILE *file = fopen("/Users/harshsingh/Documents/College/SEM_3/OS/Assignments/Scheduler/job_data.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    Job jobs[MAX_JOBS];
    int numJobs = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        parseJobData(line, &jobs[numJobs]);
        numJobs++;
    }

    fclose(file);

  
    int timeQuantum = 9; 
    int currentTime = 0;
FILE *logFile = fopen("scheduler_log.txt", "w");
        if (logFile == NULL) {
            perror("Error opening log file");
            return 1;
        }
        for (int i = 0; i < numJobs; i++) {
            Job *job = &jobs[i];
            job->io=0;
            job->ioTime=99999;
            job->responseTime=0;
        }

        int c =0;

    while (numJobs > 0) {
        for (int i = 0; i < numJobs; i++) {
            Job *job = &jobs[i];
            if(job->arrivalTime>currentTime)
            continue;
            if (job->remainingCpuBurst > 0) {

                int executionTime = (job->remainingCpuBurst < timeQuantum) ? job->remainingCpuBurst : timeQuantum;
                job->remainingCpuBurst -= executionTime;
                currentTime += executionTime;

                if(job->responseTime==0)
                    {job->responseTime=currentTime-job->arrivalTime;printf("The recorded response time is %d\n", job->responseTime);}
                
                printf("Job %d is executing for %d seconds at time %d\n", job->jobID, executionTime, currentTime);
                
                if(job->remainingCpuBurst==0 && job->io==0)
                   {
                    job->io=1;
                    job->ioTime = currentTime;
                    printf("Job %d is going into I/O for %d seconds at time %d\n", job->jobID, job->ioCompletionTime, currentTime);

                    c++;
                   } 

              }
            }
            if(c==numJobs)
            break;
        if(currentTime==0)
            currentTime+=timeQuantum;

        }

    while(c!=0)
    {   
        currentTime+=5;
        for (int i = 0; i < numJobs; i++) {
            Job *job = &jobs[i];
            if(currentTime-job->ioTime>=job->ioCompletionTime)
            {
                printf("Job %d is out of I/O at time %d\n", job->jobID, currentTime);

                        job->io=2;
            }
            if (job->remainingCpuBurst == 0 && job->io==2) {
                        c--;
                       int completionTime = currentTime;
                        job->turnaroundTime = completionTime-job->arrivalTime;
                        job->waitingTime = job->turnaroundTime-job->cpuBurst;

                    job->ioCompletionTime += currentTime;
                    fprintf(logFile, "%d,%d,%d,%d,%d,%d\n", job->jobID, job->cpuBurst,job->arrivalTime, job->responseTime,  job->waitingTime, job->turnaroundTime);

                    for (int j = i; j < numJobs - 1; j++) {
                        jobs[j] = jobs[j + 1];
                    }
                    for (int i = 0; i < numJobs; i++) {
                        Job *job = &jobs[i];
                    }
                    numJobs--;
                    i--; 
                }
        }
    }
        fclose(logFile);

        printf("All jobs have completed.\n");

    return 0;
}
