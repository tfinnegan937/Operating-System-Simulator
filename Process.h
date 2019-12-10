//
// Created by tfinnegan on 12/9/19.
//

#ifndef OPERATING_SYSTEM_SIMULATOR_PROCESS_H
#define OPERATING_SYSTEM_SIMULATOR_PROCESS_H
#include <chrono>
#include <queue>
#include <tuple>
#include <string>
#include <chrono>
#include "Config.h"
using namespace std;
class Process {
private:
    vector<tuple<char, string, int>> instruction_queue;
    int time_remaining;
    std::chrono::milliseconds cur_instruction_time_remaining;
public:
    Process();
    Process(queue<tuple<char, string, int>> * instructions, Config * program_config);
    int getTimeRemaining() const;
    void decCurTimeRemaining(int cur_instruction_time);
    tuple<char, string, int> getNextInstruction(Config * program_config);
    bool empty() const;

};


#endif //OPERATING_SYSTEM_SIMULATOR_PROCESS_H
