// Helper classes and functions for job scheduling
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

struct Job {
    char id{};
    int arrivalTime{};
    int cycleTime{};
    int startTime{-1};
    int completionTime{};

    Job(char id, int arr, int cyc) : id{id}, arrivalTime{arr}, cycleTime{cyc} {}
    Job(char id, int arr, int cyc, int st, int ct)
        : id{id}, arrivalTime{arr}, cycleTime{cyc}, startTime{st}, completionTime{ct} {}

    bool operator<(const Job& other) const noexcept { return arrivalTime < other.arrivalTime; }
    bool operator>(const Job& other) const noexcept { return arrivalTime > other.arrivalTime; }
};

using Jobs = vector<Job>;

inline Jobs makeJobs() {
    return {
        {'A', 0, 16},   //
        {'B', 3, 2},    //
        {'C', 5, 11},   //
        {'D', 9, 6},    //
        {'E', 10, 1},   //
        {'F', 12, 9},   //
        {'G', 14, 4},   //
        {'H', 16, 14},  //
        {'I', 17, 1},   //
        {'J', 19, 8},   //
    };
}

struct JobSpawner {
    explicit JobSpawner(const Jobs& jobs) : jobHeap{jobs} { make_heap(jobHeap.begin(), jobHeap.end(), greater<Job>()); }

    bool hasJobs() const { return !jobHeap.empty(); }

    Jobs jobArrival(int tick) {
        Jobs arrived;
        while (hasJobs() && jobHeap.front().arrivalTime <= tick) {
            pop_heap(jobHeap.begin(), jobHeap.end(), greater<Job>());
            auto job = jobHeap.back();
            jobHeap.pop_back();
            arrived.emplace_back(job);
        }
        return arrived;
    }

    Jobs jobHeap;
};

using StatsMap = unordered_map<string, unordered_map<char, int>>;

inline void printStatusMapAsTable(StatsMap& m) {
    vector<string> metrics;
    metrics.reserve(m.size());
    for (const auto& kv : m) metrics.emplace_back(kv.first);

    // collect sorted job ids
    set<char> jobIds;
    for (auto& kv1 : m) {
        for (auto& kv2 : kv1.second) jobIds.insert(kv2.first);
    }

    const string keyName = "Job ID";

    // compute column widths
    unordered_map<string, int> colWidths;
    int maxJobIdLen = 0;
    for (char jid : jobIds) maxJobIdLen = max(maxJobIdLen, (int)string(1, jid).size());
    colWidths[keyName] = max((int)keyName.size(), maxJobIdLen);

    for (const auto& metric : metrics) {
        int max_val_len = (int)metric.size();
        auto itMetric = m.find(metric);
        if (itMetric != m.end()) {
            for (char jid : jobIds) {
                auto itVal = itMetric->second.find(jid);
                if (itVal != itMetric->second.end()) {
                    max_val_len = max(max_val_len, (int)to_string(itVal->second).size());
                }
            }
        }
        colWidths[metric] = max_val_len;
    }

    // header
    cout << left << setw(colWidths[keyName]) << keyName;
    for (const auto& metric : metrics) {
        cout << " | " << left << setw(colWidths[metric]) << metric;
    }
    cout << '\n';

    // separator length: sum widths + 3 chars per metric for " | "
    int sepLen = accumulate(colWidths.begin(), colWidths.end(), 0,
                            [](int acc, const pair<const string, int>& p) { return acc + p.second; }) +
                 3 * (int)metrics.size();
    cout << string(sepLen, '-') << '\n';

    // rows
    for (char jid : jobIds) {
        string jidStr(1, jid);
        cout << left << setw(colWidths[keyName]) << jidStr;
        for (const auto& metric : metrics) {
            string val;
            auto itMetric = m.find(metric);
            if (itMetric != m.end()) {
                auto itVal = itMetric->second.find(jid);
                if (itVal != itMetric->second.end()) val = to_string(itVal->second) + " ms";
            }
            cout << " | " << left << setw(colWidths[metric]) << val;
        }
        cout << '\n';
    }
}

static const string WAIT_TIME = "Waiting Time";
static const string TURN_TIME = "Turnaround Time";

using Avgs = pair<double, double>;

Avgs simulateFCFS(Jobs jobs) {
    int currentTime = 0;
    StatsMap stats;

    sort(jobs.begin(), jobs.end());

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

    int totalWaiting = 0;
    int totalTurnaround = 0;
    for (const auto& job : jobs) {
        totalWaiting += stats[WAIT_TIME][job.id];
        totalTurnaround += stats[TURN_TIME][job.id];
    }
    double avgWaiting = (double)totalWaiting / jobs.size();
    double avgTurnaround = (double)totalTurnaround / jobs.size();
    cout << "\nAverage Waiting Time: " << avgWaiting << " ms\n";
    cout << "Average Turnaround Time: " << avgTurnaround << " ms\n";
    return {avgWaiting, avgTurnaround};
}

Avgs sjn_scheduling(const Jobs& processes) {
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
    return {avgWait, avgTurn};
}

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

    Avgs run() {
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
        for (; waitItr != stats[WAIT_TIME].end() && turnItr != stats[TURN_TIME].end(); ++waitItr, ++turnItr) {
            avgWait += (*waitItr).second;
            avgTurn += (*turnItr).second;
        }
        avgWait /= n;
        avgTurn /= n;

        cout << "\nAverages\n"
             << "Average Waiting Time:\t\t" << avgWait << " ms\n"
             << "Average Turnaround Time:\t" << avgTurn << " ms\n";
        return {avgWait, avgTurn};
    }
};

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

    Avgs run() {
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
        return {avgWait, avgTurn};
    }
};

int main() {
    // FCFS
    cout << "First-Come, First-Served (FCFS) Scheduling\n";
    auto fcfsAvgs = simulateFCFS(makeJobs());
    cout << '\n';

    // SJN
    cout << "Shortest Job Next (SJN) Scheduling\n";
    auto sjnAvgs = sjn_scheduling(makeJobs());
    cout << '\n';

    // SRT
    cout << "Shortest Remaining Time (SRT) Scheduling\n";
    SRT srt{};
    auto srtAvgs = srt.run();
    cout << '\n';

    // Round Robin
    cout << "Round Robin Scheduling\n";
    RoundRobin rr{};
    auto rrAvgs = rr.run();
    cout << '\n';

    // Averages table
    cout << "Summary of Averages\n";
    auto fmt = [](double v) {
        stringstream ss;
        ss << fixed << setprecision(2) << v << " ms";
        return ss.str();
    };

    cout << left << setw(25) << "Scheduling Algorithm" << " | " << left << setw(20) << "Average Waiting Time"
         << " | " << setw(22) << "Average Turnaround Time" << '\n'
         << string(25 + 3 + 20 + 3 + 22, '-') << '\n'
         << left << setw(25) << "FCFS" << " | " << left << setw(20) << fmt(fcfsAvgs.first) << " | " << left << setw(22)
         << fmt(fcfsAvgs.second) << '\n'
         << left << setw(25) << "SJN" << " | " << left << setw(20) << fmt(sjnAvgs.first) << " | " << left << setw(22)
         << fmt(sjnAvgs.second) << '\n'
         << left << setw(25) << "SRT" << " | " << left << setw(20) << fmt(srtAvgs.first) << " | " << left << setw(22)
         << fmt(srtAvgs.second) << '\n'
         << left << setw(25) << "Round Robin" << " | " << left << setw(20) << fmt(rrAvgs.first) << " | " << left
         << setw(22) << fmt(rrAvgs.second) << '\n';

    return 0;
}
