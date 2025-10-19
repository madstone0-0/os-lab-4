// Round Robin Job Scheduling Simulation
// build: g++ round.cpp -o round -std=c++11
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "job.h"

static const string WAIT_TIME = "Waiting Time";
static const string TURN_TIME = "Turnaround Time";

struct RoundRobin {
    int tick{};
    queue<Job> readyQueue;
    unordered_set<int> readySet;
    JobSpawner spawner;
    int timeQuantum{4};
    unordered_map<int, int> remainingTime;
    StatsMap stats;

    RoundRobin() : spawner(makeJobs()) {
        for (const auto& job : spawner.jobHeap) {
            remainingTime[job.id] = job.cycleTime;
            // stats: metric -> job.id -> value
            stats[WAIT_TIME][job.id] = 0;
            stats[TURN_TIME][job.id] = 0;
        }
    }

    bool hasRunningJobs() const {
        return any_of(remainingTime.begin(), remainingTime.end(),
                      [](decltype(remainingTime)::value_type kv) { return kv.second > 0; });
    }

    void run() {
        while (spawner.hasJobs() || hasRunningJobs()) {
            // Add arrivals to the arrivals queue
            auto arrivals = spawner.jobArrival(tick);
            for (const auto& j : arrivals) {
                cout << "t(" << tick << ")\t->\t" << j.id << "\t+\n";
                readyQueue.push(j);
                readySet.insert(j.id);
            }

            // If no jobs are ready, just tick
            if (readyQueue.empty()) {
                tick++;
                continue;
            }

            auto currJob = readyQueue.front();
            readyQueue.pop();
            readySet.erase(currJob.id);
            auto timeSlice = min(timeQuantum, remainingTime[currJob.id]);

            // Execute one tick at a time to handle arrivals and waiting updates
            for (int i{}; i < timeSlice; ++i) {
                // Update waiting time for all ready and waiting jobs
                for (const auto& kv : stats[WAIT_TIME]) {
                    if (readySet.find(kv.first) != readySet.end()) stats[WAIT_TIME][kv.first]++;
                }

                // Execute one tick for current job
                tick++;
                remainingTime[currJob.id]--;

                // Add arrivals that happen during this tick
                auto newArrivals = spawner.jobArrival(tick);
                for (const auto& j : newArrivals) {
                    cout << "t(" << tick << ")\t->\t" << j.id << "\t+\n";
                    readyQueue.push(j);
                    readySet.insert(j.id);
                }

                // If current finished record turnaround
                if (remainingTime[currJob.id] == 0) {
                    stats[TURN_TIME][currJob.id] = tick - currJob.arrivalTime;
                    break;
                }
            }

            // If not finished, requeue
            if (remainingTime[currJob.id] > 0) {
                readyQueue.push(currJob);
                readySet.insert(currJob.id);
            } else {
                cout << "t(" << tick << ")\t->\t" << currJob.id << "\t-\n";
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
        for (; waitItr != stats[WAIT_TIME].end() && turnItr != stats[TURN_TIME].end(); ++waitItr, ++turnItr) {
            avgWait += (*waitItr).second;
            avgTurn += (*turnItr).second;
        }
        avgWait /= n;
        avgTurn /= n;
        cout << "\nAverages\n"
             << "Average Waiting Time:\t\t" << avgWait << " ms\nAverage Turnaround Time:\t" << avgTurn << " ms\n";
    }
};

int main() {
    RoundRobin rr{};
    rr.run();
}
