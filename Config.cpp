//
// Created by tim on 9/8/19.
//

#include "Config.h"
#include <iostream>
#include <stdexcept>
//Private Methods

void Config::initializeConfigData(map<string, string> parser_output) {
    try {
        //Initialize Processor Cycle Time in msec
        if(stoi(parser_output.at("Processor cycle time {msec}")) <= 0){
            throw runtime_error("Processor cycle time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("run"), stoi(parser_output.at("Processor cycle time {msec}"))));
        }
        //Initialize Monitor Display Time
        if(stoi(parser_output.at("Monitor display time {msec}")) <= 0){
            throw runtime_error("Monitor display time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("monitor"), stoi(parser_output.at("Monitor display time {msec}"))));
        }
        //Initialize Mouse Cycle Time
        if(stoi(parser_output.at("Mouse cycle time {msec}")) <= 0){
            throw runtime_error("Mouse cycle time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("mouse"), stoi(parser_output.at("Mouse cycle time {msec}"))));
        }

        //Initialize Hard Drive Cycle Time
        if(stoi(parser_output.at("Hard drive cycle time {msec}")) <= 0){
            throw runtime_error("Hard drive cycle time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("hard drive"), stoi(parser_output.at("Hard drive cycle time {msec}"))));
        }

        //Initialize Keyboard Cycle Time
        if(stoi(parser_output.at("Keyboard cycle time {msec}")) <= 0){
            throw runtime_error("Keyboard cycle time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("keyboard"), stoi(parser_output.at("Keyboard cycle time {msec}"))));
        }

        //Initialize Memory Cycle Time
        if(stoi(parser_output.at("Memory cycle time {msec}")) <= 0){
            throw runtime_error("Memory cycle time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("allocate"), stoi(parser_output.at("Memory cycle time {msec}"))));
            cycle_times.insert(pair<string, int>(string("block"), stoi(parser_output.at("Memory cycle time {msec}"))));
        }

        //Initialize Printer Cycle Time
        if(stoi(parser_output.at("Printer cycle time {msec}")) <= 0){
            throw runtime_error("Printer cycle time <= 0");
        }
        else {
            cycle_times.insert(pair<string, int>(string("printer"), stoi(parser_output.at("Printer cycle time {msec}"))));
        }

        if(stoi(parser_output.at("System memory {kbytes}")) <= 0){
            throw runtime_error("Memory Allocation <=0 KB");
        }
        else{
            misc_configuration_details.insert(pair<string, string>(string("size"), parser_output.at("System memory {kbytes}")));
        }

        if(stoi(parser_output.at("Memory block size {kbytes}")) <= 0){
            throw runtime_error("Memory block size <=0 KB");
        }
        else{
            misc_configuration_details.insert(pair<string, string>(string("bsize"), parser_output.at("Memory block size {kbytes}")));
        }

        if(stoi(parser_output.at("Printer quantity")) <= 0){
            throw runtime_error("Printer quantity less than zero");
        }
        else{
            misc_configuration_details.insert(pair<string, string>(string("pcount"), parser_output.at("Printer quantity")));
        }

        if(stoi(parser_output.at("Hard drive quantity")) <= 0){
            throw runtime_error("Hard drive quantity less than zero");
        }
        else{
            misc_configuration_details.insert(pair<string, string>(string("hcount"), parser_output.at("Hard drive quantity")));
        }

        if(stoi(parser_output.at("Processor Quantum Number {msec}")) <= 0){
            throw runtime_error("Quantum number less than zero");
        }
        else{
            misc_configuration_details.insert(pair<string,string>(string("quantum"), parser_output.at("Processor Quantum Number {msec}")));
        }

        if(parser_output.at("CPU Scheduling Code") != "RR" && parser_output.at("CPU Scheduling Code") != "STR"){
            throw runtime_error("Invalid CPU Scheduler : " + parser_output.at("CPU Scheduling Code"));
        }
        else{
            misc_configuration_details.insert(pair<string, string>(string("scheduler"), parser_output.at("CPU Scheduling Code")));
        }

        misc_configuration_details.insert(pair<string, string>(string("meta_file_path"), parser_output.at("File Path")));
        misc_configuration_details.insert(pair<string, string>(string("log_file_path"), parser_output.at("Log File Path")));
        misc_configuration_details.insert(pair<string, string>(string("log_type"), parser_output.at("Log")));

    }
    catch(const out_of_range& oor) { //Thrown by [map].at(key) when the given input is not inside of the map
                                     //Catches misspellings and incorrect config options
        cerr << "\nError! Config file element misspelled or invalid. Error: " << oor.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch(const runtime_error& rtr){
        cerr << "\nError! " << rtr.what() << "\n";
        exit(EXIT_FAILURE);
    }

}
//Public Methods

Config::Config(){
    pthread_mutex_init(&config_access, NULL);
}

void Config::readFile(string file_name){
    pthread_mutex_lock(&config_access);

    ConfigParser * config_generator;

    map<string, string> parser_output;

    config_generator = new ConfigParser(file_name);
    config_generator->parse();

    parser_output = config_generator->retrieveFormattedOutput();
    this->initializeConfigData(parser_output);
    delete config_generator;
    config_generator = nullptr;
    pthread_mutex_unlock(&config_access);

}

int Config::getCycleTime(string type) const{
    pthread_mutex_lock(&config_access);
    int cycle_time;
    try{
        cycle_time = cycle_times.at(type);
    }
    catch(const out_of_range& oor){
        cerr << "\nIncorrect Cycle Time Descriptor " << type << endl;
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&config_access);

return cycle_time;
}

string Config::getMiscConfigDetail(string type) const {
    pthread_mutex_lock(&config_access);
    string config_detail;
    try{
        config_detail = misc_configuration_details.at(type);
    }
    catch(const out_of_range& oor){
        cerr << "\nIncorrect Configuration Rule " << type << endl;
        exit(EXIT_FAILURE);

}
    pthread_mutex_unlock(&config_access);

    return config_detail;
}
