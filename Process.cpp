//
// Created by tfinnegan on 12/9/19.
//

#include "Process.h"

bool Process::empty() const {
    return instruction_queue.empty();
}

tuple<char, string, int> Process::getNextInstruction(Config * program_config){
    tuple<char, string, int> front = instruction_queue.back();
    time_remaining -= get<2>(front) * program_config -> getCycleTime(get<1>(front));
    instruction_queue.pop_back();
    return front;
}

int Process::getTimeRemaining() const{
    return time_remaining;
}

Process::Process(queue<tuple<char, string, int>> instructions, Config * program_config){
    while(!instructions.empty()){
        front = instructions.front();
        time_remaining += get<2>(front) * program_config->getCycleTime(get<1>(front));
        instruction_queue.push_back(front);
        instructions.pop();
    }
}

Process::Process(){

}