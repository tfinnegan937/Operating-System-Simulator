//
// Created by tim on 9/8/19.
//
#include "input_parser.h"
#include <iostream>

InputParser::InputParser(string file_name){
    try{
        file.open(file_name);
        if(!file.is_open()){
            throw runtime_error("Could not find file!");
        }
        if(file_name.find(".mdf") == -1){
            throw runtime_error("Wrong Meta-Data file extension!");
        }
    }
    catch(std::runtime_error& e){
        cerr << "An exception occurred. Could not open Config file at " << file_name << " as listed in the configuration file! Reason: " << e.what() << "\n";

        exit(EXIT_FAILURE);
    }

}

InputParser::~InputParser(){
    file.close();
}


void InputParser::parse(){
    string delimiter = ";";

    string current_line = " ";
    string current_instruction = " ";

    size_t instruction_pos = 0;
    size_t delimiter_pos = 0;

    while(!file.eof()){
        getline(file, current_line);

        if(current_line.find("Start") == -1 && current_line.find("End") == -1) { //Ignore the starting and ending lines
            while (current_line.find(delimiter) != -1 || current_line.find(".") != -1) {
                delimiter_pos = current_line.find(delimiter);
                //For handling the period at the end
                if(current_line.find(delimiter) == -1){
                    delimiter_pos = current_line.length() - 1;
                }

                current_instruction = current_line.substr(instruction_pos, delimiter_pos); // don't include delimiter itself
                //cout << endl << current_instruction << endl;
                instruction_queue.push(validateInput(tupleFromString(current_instruction)));
                //More period handling
                if(delimiter_pos != current_line.length()) {
                    current_line.erase(instruction_pos, delimiter_pos + 1);
                }
                else
                {
                    current_line.erase(instruction_pos, delimiter_pos);
                }
            }
        }

    }
}

queue<tuple<char, string, int>> InputParser::retrieveFormattedOutput() const{
    return instruction_queue;
}

tuple<char, string, int> InputParser::tupleFromString(string instruction){
    char type;
    string process;
    int cycles;
    //Hacky fix for whitespace being passed to the function.
    //Will double check parse() at a leter date
    for(int i = 0; i < instruction.length(); i++){
        type = instruction.c_str()[i];
        if(int(type) > 64 && int(type) < 91){
            break;
        }
    }
    process = instruction.substr(instruction.find("{")+1, instruction.find("}") - instruction.find("{")-1);
    cycles = stoi(removeWhiteSpace(instruction.substr(instruction.find("}") + 1, instruction.length() - instruction.find("}"))));

    tuple<char, string, int> output_tuple = {type, process, cycles};

    return output_tuple;

}

string InputParser::removeWhiteSpace(string in){

    int start = 0;
    int end = in.length();
    int cursor = 0;
    char ch = in.c_str()[start];

    while(ch == 9 || ch == 32){ //ASCII 9 is tab, ASCII 32 is space
        cursor++;
        ch = in.c_str()[cursor];
        start = cursor;

    }

    ch = ' ';
    cursor = in.length();

    while(ch == 9 || ch == 32){
        end = cursor;
        cursor--;
        ch = in.c_str()[cursor];
    }

    return in.substr(start, end - start);
}

tuple<char, string, int> InputParser::validateInput(tuple<char, string, int> instruction){
    try{
        char type = get<0>(instruction);
        string process = get<1>(instruction);
        int num_cycles = get<2>(instruction);

        if(type == 'S'){
            if(num_cycles != 0){
                throw runtime_error("Operating System Start or End Cycles not Zero!");
            }

            if(process != "begin" && process != "finish"){
                std::cout << process << endl;
                throw runtime_error("Incorrect Operating System Call. Must be begin or finish.");
            }
        }

        if(type == 'A'){
            if(num_cycles !=0){
                throw runtime_error("Application Start or End Cycles not Zero!");
            }

            if(process != "begin" && process != "finish"){
                throw runtime_error("Incorrect Application call. Must be begin or finish.");
            }
        }

        if(type == 'P'){
            if(num_cycles <= 0){
                throw runtime_error("Incorrect Process Cycle Time! Must be > 0");
            }

            if(process != "run"){
                throw runtime_error("Process Descriptor not run!");
            }
        }

        if(type == 'I'){
            if(num_cycles <=0){
                throw runtime_error("Incorrect Input Cycle Time! Must be >0");
            }

            if(process != "hard drive" && process != "keyboard" && process != "mouse"){
                throw runtime_error("Incorrect Input device! Must be hard drive, keyboard, or mouse.");
            }
        }

        if(type == 'O'){
            if(num_cycles <=0){
                throw runtime_error("Incorrect Output Cycle Time! Must be >0");
            }

            if(process != "hard drive" && process != "monitor" && process != "printer"){
                throw runtime_error("Incorrect Output Device! Must be hard drive, monitor, or printer");
            }
        }

        if(type == 'M'){
            if(num_cycles <= 0){
                throw runtime_error("Incorrect Memory Cycle Time! Must be >0");
            }
            if(process != "allocate" && process != "block"){
                throw runtime_error("Incorrect Memory Operation! Must be allocate or block.");
            }
        }
    }
    catch(runtime_error& rte){
        cout << "Encountered error validating metadata: " << rte.what() << endl;
        exit(EXIT_FAILURE);
    }

    return instruction;

}
