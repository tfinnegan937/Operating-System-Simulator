//
// Created by tfinnegan on 12/9/19.
//

#include "Process.h"
#include <iostream>
using namespace std;
bool Process::empty() const {
    return instruction_queue.empty();
}

tuple<char, string, int> Process::getNextInstruction(Config * program_config){
    tuple<char, string, int> front = instruction_queue.front();
    if(get<1>(front) != "begin" && get<1>(front) != "finish") {
        time_remaining -= get<2>(front) * program_config->getCycleTime(get<1>(front));
    }
    instruction_queue.erase(instruction_queue.begin());
    return front;
}

int Process::getTimeRemaining() const{
    return time_remaining;
}

Process::Process(queue<tuple<char, string, int>> * instructions, Config * program_config){
    auto instructions_copy = *instructions;
    time_remaining = 0;
    while(!instructions_copy.empty()){
        auto front = instructions_copy.front();
        if(get<1>(front) != "begin" && get<1>(front) != "finish") {
            time_remaining += get<2>(front) * program_config->getCycleTime(get<1>(front));

        }
        instruction_queue.push_back(front);
        instructions_copy.pop();
    }
}

Process::Process(){

}