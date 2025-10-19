// SRT (Shortest Remaining Time) Job Scheduling Simulation
// build: g++ srt.cpp -o srt -std=c++11
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <climits>
#include "job.h"

static const string WAIT_TIME = "Waiting Time";
static const string TURN_TIME = "Turnaround Time";

struct SRT {
    int tick{};
    vector<Job> readyQueue;
    JobSpawner spawner;
    unordered_map<int, int> remainingTime;
    StatsMap stats;

    SRT() : spawner(makeJobs()) {
        for (const auto& job : spawner.jobHeap) {
            remainingTime[job.id] = job.cycleTime;
            stats[WAIT_TIME][job.id] = 0;
            stats[TURN_TIME][job.id] = 0;
        }
    }

    bool hasRunningJobs() const {
        return any_of(remainingTime.begin(), remainingTime.end(),
                      [](decltype(remainingTime)::value_type kv) { return kv.second > 0; });
    }

    // Find the index of the job with shortest remaining time
    int findShortestRemainingTime() {
        if (readyQueue.empty()) return -1;
        
        int minIdx = 0;
        int minTime = remainingTime[readyQueue[0].id];
        
        for (int i = 1; i < readyQueue.size(); ++i) {
            if (remainingTime[readyQueue[i].id] < minTime) {
                minTime = remainingTime[readyQueue[i].id];
                minIdx = i;
            }
        }
        return minIdx;
    }

    void run() {
        while (spawner.hasJobs() || hasRunningJobs()) {
            // Add arrivals to the ready queue
            auto arrivals = spawner.jobArrival(tick);
            for (const auto& j : arrivals) {
                cout << "t(" << tick << ")\t->\t" << j.id << "\t+\n";
                readyQueue.push_back(j);
            }

            // If no jobs are ready, just tick
            if (readyQueue.empty()) {
                tick++;
                continue;
            }

            // Find job with shortest remaining time
            int shortestIdx = findShortestRemainingTime();
            if (shortestIdx == -1) {
                tick++;
                continue;
            }

            auto currJob = readyQueue[shortestIdx];

            // Update waiting time for all jobs in ready queue except current
            for (int i = 0; i < readyQueue.size(); ++i) {
                if (i != shortestIdx) {
                    stats[WAIT_TIME][readyQueue[i].id]++;
                }
            }

            // Execute one tick for current job
            tick++;
            remainingTime[currJob.id]--;

            // Add arrivals that happen during this tick
            auto newArrivals = spawner.jobArrival(tick);
            for (const auto& j : newArrivals) {
                cout << "t(" << tick << ")\t->\t" << j.id << "\t+\n";
                readyQueue.push_back(j);
            }

            // If current job finished, record turnaround and remove from queue
            if (remainingTime[currJob.id] == 0) {
                stats[TURN_TIME][currJob.id] = tick - currJob.arrivalTime;
                cout << "t(" << tick << ")\t->\t" << currJob.id << "\t-\n";
                readyQueue.erase(readyQueue.begin() + shortestIdx);
            }
        }

        // Print status table
        cout << "\nFinal Job Statistics\n";
        printStatusMapAsTable(stats);
        cout << '\n';

        // Averages and summary
        double n = remainingTime.size();
        double avgWait{};
        double avgTurn{};
        auto waitItr = stats[WAIT_TIME].begin();
        auto turnItr = stats[TURN_TIME].begin();
        for (; waitItr != stats[WAIT_TIME].end() && turnItr != stats[TURN_TIME].end(); 
             ++waitItr, ++turnItr) {
            avgWait += (*waitItr).second;
            avgTurn += (*turnItr).second;
        }
        avgWait /= n;
        avgTurn /= n;

        cout << "\nAverages\n"
             << "Average Waiting Time:\t\t" << avgWait << " ms\n"
             << "Average Turnaround Time:\t" << avgTurn << " ms\n";
    }
};

int main() {
    SRT srt{};
    srt.run();
}
