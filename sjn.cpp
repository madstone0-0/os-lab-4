#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

class Process {
public:
    string pid;
    int arrival_time;
    int cpu_cycle;
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;

    Process(string id, int arrival, int cpu)
        : pid(id), arrival_time(arrival), cpu_cycle(cpu),
          start_time(0), completion_time(0),
          waiting_time(0), turnaround_time(0) {}
};

void sjn_scheduling(vector<Process>& processes) {
    // Sort by arrival time
    sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
            return a.arrival_time < b.arrival_time;
        });

    int current_time = 0;
    vector<Process> completed;
    vector<Process> remaining = processes;
    vector<int> waiting_times, turnaround_times;

    while (!remaining.empty()) {
        // Get available processes
        vector<Process*> available;
        for (auto& p : remaining) {
            if (p.arrival_time <= current_time)
                available.push_back(&p);
        }

        if (available.empty()) {
            // Jump to next process if CPU idle
            current_time = remaining.front().arrival_time;
            continue;
        }

        // Select process with shortest CPU cycle
        auto shortest_it = min_element(available.begin(), available.end(),
            [](Process* a, Process* b) {
                return a->cpu_cycle < b->cpu_cycle;
            });

        Process* shortest = *shortest_it;

        // Compute times
        shortest->start_time = current_time;
        shortest->completion_time = current_time + shortest->cpu_cycle;
        shortest->turnaround_time = shortest->completion_time - shortest->arrival_time;
        shortest->waiting_time = shortest->turnaround_time - shortest->cpu_cycle;

        // Update time and record data
        current_time = shortest->completion_time;
        completed.push_back(*shortest);
        waiting_times.push_back(shortest->waiting_time);
        turnaround_times.push_back(shortest->turnaround_time);

        // Remove from remaining list
        remaining.erase(remove_if(remaining.begin(), remaining.end(),
            [&](const Process& p) { return p.pid == shortest->pid; }),
            remaining.end());
    }

    // Display results
    cout << "SJN Scheduling Results:\n";
    cout << left << setw(5) << "Job"
         << setw(8) << "Arrival"
         << setw(6) << "Burst"
         << setw(8) << "Start"
         << setw(11) << "Completion"
         << setw(10) << "Waiting"
         << setw(12) << "Turnaround" << "\n";

    for (auto& p : completed) {
        cout << left << setw(5) << p.pid
             << setw(8) << p.arrival_time
             << setw(6) << p.cpu_cycle
             << setw(8) << p.start_time
             << setw(11) << p.completion_time
             << setw(10) << p.waiting_time
             << setw(12) << p.turnaround_time << "\n";
    }

    // Compute averages
    double avg_wait = 0, avg_turn = 0;
    for (size_t i = 0; i < waiting_times.size(); ++i) {
        avg_wait += waiting_times[i];
        avg_turn += turnaround_times[i];
    }
    avg_wait /= waiting_times.size();
    avg_turn /= turnaround_times.size();

    cout << fixed << setprecision(2);
    cout << "\nAverage Waiting Time: " << avg_wait << " ms\n";
    cout << "Average Turnaround Time: " << avg_turn << " ms\n";
}

int main() {
    vector<Process> processes = {
        {"A", 0, 16},
        {"B", 3, 2},
        {"C", 5, 11},
        {"D", 9, 6},
        {"E", 10, 1},
        {"F", 12, 9},
        {"G", 14, 4},
        {"H", 16, 14},
        {"I", 17, 1},
        {"J", 19, 8}
    };

    sjn_scheduling(processes);
    return 0;
}
