// Shortest Job Next Scheduling Simulation
// build: g++ sjn.cpp -o sjn -std=c++11
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include "job.h"
using namespace std;

static const string WAIT_TIME = "Waiting Time";
static const string TURN_TIME = "Turnaround Time";

void sjn_scheduling(const Jobs& processes) {
    // Job spawner
    auto spawner = JobSpawner(processes);
    StatsMap stats;
    for (const auto& job : spawner.jobHeap) {
        // stats: metric -> job.id -> value
        stats[WAIT_TIME][job.id] = 0;
        stats[TURN_TIME][job.id] = 0;
    }

    int current_time = 0;
    Jobs remaining;
    Jobs completed;

    while (spawner.hasJobs() || completed.size() < processes.size()) {
        // Get available processes
        auto available = spawner.jobArrival(current_time);
        for (const auto& j : available) remaining.push_back(j);

        if (remaining.empty()) {
            // Jump to next process if CPU idle
            current_time = spawner.jobHeap.front().arrivalTime;
            continue;
        }

        // Select process with shortest CPU cycle
        auto shortest_it = min_element(remaining.begin(), remaining.end(),
                                       [](const Job& a, const Job& b) { return a.cycleTime < b.cycleTime; });

        auto& shortest = *shortest_it;

        // Compute times
        shortest.startTime = current_time;
        shortest.completionTime = current_time + shortest.cycleTime;
        stats[WAIT_TIME][shortest.id] = shortest.startTime - shortest.arrivalTime;
        stats[TURN_TIME][shortest.id] = shortest.completionTime - shortest.arrivalTime;

        // Update time and record data
        current_time = shortest.completionTime;
        completed.push_back(shortest);
        remaining.erase(shortest_it);
    }

    // Display results
    cout << "SJN Scheduling Results:\n";
    printStatusMapAsTable(stats);

    // Compute averages
    double n = processes.size();
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

    cout << fixed << setprecision(2);
    cout << "\nAverage Waiting Time: " << avgWait << " ms\n";
    cout << "Average Turnaround Time: " << avgTurn << " ms\n";
}

int main() {
    Jobs processes = makeJobs();

    sjn_scheduling(processes);
    return 0;
}
