// Helper classes and functions for job scheduling
#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
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
