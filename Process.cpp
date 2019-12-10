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
    if(cur_instruction_time_remaining.count() == 0){
        instruction_queue.erase(instruction_queue.begin());
        cur_instruction_time_remaining =cur_instruction_time_remaining = std::chrono::milliseconds(
                int(get<2>(instruction_queue.front()) * program_config->getCycleTime(get<1>(instruction_queue.front()))));
    }
    tuple<char, string, int> front = instruction_queue.front();
    if(get<1>(front) != "begin" && get<1>(front) != "finish") {
        time_remaining -= get<2>(front) * program_config->getCycleTime(get<1>(front));
    }
    return front;
}

int Process::getTimeRemaining() const{
    return time_remaining;
}

void Process::decCurTimeRemaining(int cur_instruction_time) {
    cur_instruction_time_remaining -= std::chrono::milliseconds(cur_instruction_time);
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

    auto final_front = instruction_queue.front();
    if(get<1>(final_front) != "begin" && get<1>(final_front) != "finish") {
        cur_instruction_time_remaining = std::chrono::milliseconds(
                int(get<2>(final_front) * program_config->getCycleTime(get<1>(final_front))));
    }
    else{
        cur_instruction_time_remaining = std::chrono::milliseconds(0);
    }
}

Process::Process(){

}