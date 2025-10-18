// Shortest Remaining Time Job Scheduling Simulation
// build: g++ srt.cpp -o srt -std=c++11
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "job.h"

static const string WAIT_TIME = "Waiting Time";
static const string TURN_TIME = "Turnaround Time";
