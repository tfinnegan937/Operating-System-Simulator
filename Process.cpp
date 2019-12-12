//
// Created by tfinnegan on 12/9/19.
//

#include "Process.h"
#include <iostream>
using namespace std;
bool Process::empty() const {
    if(instruction_queue.empty()){
        //cout << "EMPTY!\n";
    }
    else{
        //cout << "Instruction type " << get<1>(instruction_queue.front()) << endl;
    }
    return instruction_queue.empty();
}

tuple<char, string, int> Process::getNextInstruction(Config * program_config){
    tuple<char, string, int> front = instruction_queue.front();
    return front;
}

int Process::getCurInsTimeRemaining(){
    return cur_instruction_time_remaining.count();
}

int Process::getTimeRemaining() const{
    return time_remaining + cur_instruction_time_remaining.count();
}

void Process::setCurTimeRemaining(std::chrono::milliseconds cur_time) {
    cur_instruction_time_remaining = cur_time;
}

void Process::popEmptyInstruction(Config * program_config){
    instruction_queue.erase(instruction_queue.begin());
    auto front = instruction_queue.front();
    if(get<1>(front) != "begin" && get<1>(front) != "finish") {
        cur_instruction_time_remaining = std::chrono::milliseconds(
                get<2>(front) * program_config->getCycleTime(get<1>(front)));
        time_remaining -= get<2>(front) * program_config->getCycleTime(get<1>(front));
    }
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

    if(get<1>(final_front) != "begin" && get<1>(final_front) != "finish") {
        time_remaining -= get<2>(final_front) * program_config->getCycleTime(get<1>(final_front));
    }
}

void Process::suspend(){
    //instruction_queue.insert(instruction_queue.begin(), {'A', "Begin", 0});
}

Process::Process(){

}