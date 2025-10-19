// First Come First Served Job Scheduling Simulation
// build: g++ fcfs.cpp -o fcfs -std=c++11
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "job.h"

static const string WAIT_TIME = "Waiting Time";
static const string TURN_TIME = "Turnaround Time";

void simulateFCFS(Jobs& jobs) {
    int currentTime = 0;
    StatsMap stats;

    
    sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
        return a.arrivalTime < b.arrivalTime;
    });

   
    for (auto& job : jobs) {
        
        currentTime = max(currentTime, job.arrivalTime);
        
        
        job.startTime = currentTime;
        
        
        job.completionTime = job.startTime + job.cycleTime;
        
        
        int waitingTime = job.startTime - job.arrivalTime;
        
        
        int turnaroundTime = job.completionTime - job.arrivalTime;
        
        
        stats[WAIT_TIME][job.id] = waitingTime;
        stats[TURN_TIME][job.id] = turnaroundTime;
        
       
        currentTime = job.completionTime;
    }

    
    cout << "FCFS Scheduling Results:\n";
    printStatusMapAsTable(stats);

   
    int totalWaiting = 0, totalTurnaround = 0;
    for (const auto& job : jobs) {
        totalWaiting += stats[WAIT_TIME][job.id];
        totalTurnaround += stats[TURN_TIME][job.id];
    }
    cout << "\nAverage Waiting Time: " << (double)totalWaiting / jobs.size() << " ms\n";
    cout << "Average Turnaround Time: " << (double)totalTurnaround / jobs.size() << " ms\n";
}

int main() {
    Jobs jobs = makeJobs();
    simulateFCFS(jobs);
    return 0;
}