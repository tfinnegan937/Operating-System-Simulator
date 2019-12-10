//
// Created by tfinnegan on 12/9/19.
//

#ifndef OPERATING_SYSTEM_SIMULATOR_PROCESS_H
#define OPERATING_SYSTEM_SIMULATOR_PROCESS_H
#include <chrono>
#include <queue>
#include <tuple>
#include <string>
#include "Config.h"
class Process {
private:
    vector<tuple<char, string, int>> instruction_queue;
    int time_remaining;
public:
    Process();
    Process(queue<tuple<char, string, int>> instructions, Config * program_config);
    int getTimeRemaining() const;
    tuple<char, string, int> getNextInstruction(Config * program_config);
    bool empty();

};


#endif //OPERATING_SYSTEM_SIMULATOR_PROCESS_H
