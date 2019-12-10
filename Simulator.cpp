//
// Created by tim on 9/9/19.
//

#include "Simulator.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <bits/stdc++.h> 

Config * Simulator::program_config;
nanoseconds Simulator::start_time;
PCB Simulator::pcb;

pthread_t Simulator::keyboard_t;
pthread_t Simulator::mouse_t;
pthread_t Simulator::monitor_t;
pthread_t * Simulator::harddrive_t;
pthread_t * Simulator::printer_t;

pthread_mutex_t Simulator::printer;
pthread_mutex_t Simulator::mouse;
pthread_mutex_t Simulator::keyboard;
pthread_mutex_t Simulator::monitor;
pthread_mutex_t Simulator::harddrive;
pthread_mutex_t Simulator::output_queue_m;

vector<Process> Simulator::active_processes;

Semaphore Simulator::printer_s;
Semaphore Simulator::harddrive_s;

queue<tuple<float, string>> Simulator::output_queue;
queue<tuple<char, string, int>> Simulator::print_queue;
queue<tuple<char, string, int>> Simulator::drive_queue;


int Simulator::handled_processes;
int Simulator::size;


struct HDDThreadBlock{
    int num_cycles;
    char io_type;
};


Simulator::Simulator(string config_file_path){

    InputParser * metadata_parser;

    program_config = new Config();

    program_config->readFile(config_file_path);
    metadata_parser = new InputParser(program_config->getMiscConfigDetail("meta_file_path"));
    metadata_parser->parse();
    size = stoi(program_config->getMiscConfigDetail("size"));
    instruction_queue = metadata_parser->retrieveFormattedOutput();
    cur_mem = 0;

    handled_processes = 0;

    harddrive_t = new pthread_t[stoi(program_config->getMiscConfigDetail("hcount"))];
    printer_t = new pthread_t[stoi(program_config->getMiscConfigDetail("pcount"))];

    delete metadata_parser;

    metadata_parser = nullptr;
}

void Simulator::outputOperationLog(float time_stamp, string tag, string operation) {
    cout << time_stamp << " - " << tag << ": " << operation << endl;
}

float Simulator::getTimeStamp() {
    nanoseconds time_stamp = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

    nanoseconds cur_time = time_stamp - start_time;

    return float(cur_time.count()/1000000000.0);
}

void Simulator::ProcessOutput(tuple<char, string, int> instruction) {
    HDDThreadBlock * hddblock = (struct HDDThreadBlock*)malloc(sizeof(struct HDDThreadBlock));
    int * cycles = (int*)malloc(sizeof(*cycles));

    string type = get<1>(instruction);
    *cycles = get<2>(instruction);
    try {
        if (type == "hard drive") {
            hddblock->io_type = 'o';
            hddblock->num_cycles = *cycles;
            int hdd_count = harddrive_s.getCount();

            pthread_create(&harddrive_t[hdd_count], NULL, handleHarddrive, (void*) hddblock);
            pthread_join(harddrive_t[hdd_count], NULL);

        }

        else if (type == "monitor") {
            pthread_create(&monitor_t, NULL, handleMonitor, (void*)cycles);
            pthread_join(monitor_t, NULL);
        }

        else if (type == "printer") {
            int print_count = printer_s.getCount();
            pthread_create(&printer_t[print_count], NULL, handlePrinter, (void*)cycles);
            pthread_join(printer_t[print_count], NULL);
        }

        else{
            throw runtime_error("Invalid Output Device " + type);
        }
    }
    catch(const runtime_error& rtr){
        cerr << "\nError! " << rtr.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void Simulator::ProcessInput(tuple<char, string, int> instruction) {
    HDDThreadBlock * hddblock = (struct HDDThreadBlock*)malloc(sizeof(struct HDDThreadBlock));
    int * cycles = (int*)malloc(sizeof(*cycles));
    string type = get<1>(instruction);
    *cycles = get<2>(instruction);

    try{
        if (type == "hard drive") {
            
            hddblock->io_type = 'i';
            hddblock->num_cycles = *cycles;
            int hdd_count = harddrive_s.getCount();
            pthread_create(&harddrive_t[hdd_count], NULL, handleHarddrive, (void*) hddblock);
            pthread_join(harddrive_t[hdd_count], NULL);
        }
        else if(type == "keyboard"){
            pthread_create(&keyboard_t, NULL, handleKeyboard, (void*) cycles);
            pthread_join(keyboard_t, NULL);
        }
        else if(type == "Mouse"){
            pthread_create(&mouse_t, NULL, handleMouse, (void*) cycles);
            pthread_join(mouse_t, NULL);
        }
        else{
            throw runtime_error("Invalid Input Device " + type);
        }
    }
    catch(const runtime_error& rtr){
        cerr << "\nError! " << rtr.what() << endl;
        exit(EXIT_FAILURE);
    }
}

void Simulator::processProcessRun(tuple<char, string,  int> instruction) {
    cout << setprecision(6);

    string type = get<1>(instruction);
    float cycles = get<2>(instruction);
    ostringstream start;
    ostringstream end;
    start.precision(6);
    end.precision(6);
    start << fixed;
    end << fixed;
    float time_stamp = getTimeStamp();
    start << setprecision(6) << time_stamp << " - Process " << handled_processes << ": start processing action\n";
    pushToOutput(time_stamp, start.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << handled_processes << ": end processing action\n";
    pushToOutput(time_stamp, end.str());
}

void Simulator::processProcessOperation(tuple<char, string, int> instruction) {
    cout << setprecision(6);

    string type = get<1>(instruction);
    int cycles = get<2>(instruction);
    float time_stamp;
    ostringstream prep;
    prep.precision(6);
    ostringstream start;
    start.precision(6);
    ostringstream remove;
    remove.precision(6);
    int process_number = handled_processes;
    prep << fixed;
    start << fixed;
    remove << fixed;
    if(type == "begin"){
        handled_processes++;
        process_number = handled_processes;
        time_stamp = getTimeStamp();
        prep << setprecision(6) << time_stamp << " - OS: preparing process " << process_number << endl;
        pushToOutput(time_stamp, prep.str());
        time_stamp = getTimeStamp();
        start << setprecision(6) << time_stamp << " - OS: starting process " << process_number << endl;
        pushToOutput(time_stamp, start.str());
        pcb.setState("START");
    }
    else{
        time_stamp = getTimeStamp();
        remove << setprecision(6) << time_stamp << " - OS: removing process " << process_number << endl;
        pushToOutput(time_stamp, remove.str());
        pcb.setState("EXIT");
    }
}

void Simulator::processMemory(tuple<char, string, int> instruction) {
    cout << setprecision(6);

    string type = get<1>(instruction);
    int cycles = get<2>(instruction);

    string mem_space = "0xFF8C"; // Temporary placeholder memory
    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    ostringstream allocate;
    allocate.precision(6);
    start << fixed;
    end << fixed;
    allocate << fixed;
    float time_stamp;

    if(type == "block"){
        time_stamp = getTimeStamp();
        start << setprecision(6) << time_stamp << " - Process " << handled_processes << ": start memory blocking\n";
        pushToOutput(time_stamp, start.str());
        std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << handled_processes << ": end memory blocking\n";
        pushToOutput(time_stamp, end.str());
    }
    else {
        std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );

        cur_mem = cur_mem + stoi(program_config -> getMiscConfigDetail("bsize"));
        time_stamp = getTimeStamp();
        allocate << setprecision(6) << time_stamp << " - Process " << handled_processes << ": memory allocated at 0x" << std::hex << cur_mem << std::dec << endl;
        pushToOutput(time_stamp, allocate.str());

    }
}

void Simulator::cpuLoop(){
    cout << setprecision(6);

    pthread_mutex_init(&mouse, NULL);
    pthread_mutex_init(&keyboard, NULL);
    pthread_mutex_init(&monitor, NULL);
    pthread_mutex_init(&output_queue_m, NULL);

    harddrive_s.init(&harddrive, stoi(program_config->getMiscConfigDetail("hcount")));
    printer_s.init(&printer, stoi(program_config->getMiscConfigDetail("pcount")));

    start_time = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

    auto queue_copy = instruction_queue;

    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    start << fixed;
    end << fixed;

    float time_stamp = getTimeStamp();
    start << setprecision(6) << 0.0 << " - Simulator program starting\n";
    pushToOutput(time_stamp, start.str());

    tuple<char, string, int> cur_instruction;
    populateProcessVector();
    while(!queue_copy.empty() /*|| !drive_queue.empty() || !print_queue.empty()*/){
        pcb.setState("RUNNING");
        cur_instruction = queue_copy.front();
        queue_copy.pop();
        string type = get<1>(cur_instruction);
        switch(get<0>(cur_instruction)){
            case 'A':
                this->processProcessOperation(cur_instruction);
                break;
            case 'P':
                this->processProcessRun(cur_instruction);
                break;
            case 'I':
                pcb.setState("WAITING");
                ProcessInput(cur_instruction);
                pcb.setState("READY");
                break;
            case 'O':
                pcb.setState("WAITING");
                ProcessOutput(cur_instruction);
                pcb.setState("READY");
                break;
            case 'M':
                this->processMemory(cur_instruction);
                break;

        }

    }
    /*pthread_join(tinput, NULL);
    pthread_join(toutput, NULL);*/
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Simulator program ending\n";
    pushToOutput(time_stamp, end.str());
}

void Simulator::run(){
    this->cpuLoop();
    if(program_config->getMiscConfigDetail("log_type") == "Log to Both"){
        logToBoth(instruction_queue);
    }
    if(program_config->getMiscConfigDetail("log_type") == "Log to File"){
        logToFile(instruction_queue);
    }
    if(program_config->getMiscConfigDetail("log_type") == "Log to Monitor"){
        logToMonitor(instruction_queue);
    }
    setprecision(6);
}



void Simulator::logToBoth(queue<tuple<char, string, int>> queue_copy){
    logToMonitor(queue_copy);
    logToFile(queue_copy);
}

void Simulator::logToMonitor(queue<tuple<char, string, int>> queue_copy){
    cout << endl;

    cout << "Configuration File Data" << endl;
    cout << "Monitor = " << program_config->getCycleTime("monitor") << " ms/cycle" << endl;
    cout << "Processor = " << program_config->getCycleTime("run") << " ms/cycle" << endl;
    cout << "Mouse = " << program_config->getCycleTime("mouse") << " ms/cycle" << endl;
    cout << "Hard Drive = " << program_config->getCycleTime("hard drive") << " ms/cycle" << endl;
    cout << "Keyboard = " << program_config->getCycleTime("keyboard") << " ms/cycle" << endl;
    cout << "Memory = " << program_config->getCycleTime("allocate") << " ms/cycle" << endl;
    cout << "Printer = " << program_config->getCycleTime("printer") << " ms/cycle" << endl;
    cout << "Logged to: ";
    if(program_config->getMiscConfigDetail("log_type") == "Log to File"){
        cout << program_config->getMiscConfigDetail("log_file_path");
    }
    else if(program_config->getMiscConfigDetail("log_type") == "Log to Monitor"){
        cout << "monitor";
    }
    else{
        cout << "monitor and " << program_config->getMiscConfigDetail("log_file_path");
    }
    cout << endl << endl;

    cout << "Meta Data Metrics" << endl;

    while(!instruction_queue.empty()){
        tuple<char, string, int> cur_instruction = queue_copy.front();
        char cur_type = get<0>(cur_instruction);
        string cur_process = get<1>(cur_instruction);
        int cur_cycle = get<2>(cur_instruction);
        int cur_cycle_time;

        queue_copy.pop();

        if(cur_type == 'S' && cur_process == "finish"){
            break;
        }

        if(cur_type == 'S' || cur_type == 'A'){
            continue;
        }




        cur_cycle_time = program_config->getCycleTime(cur_process);

        cout << cur_type << "{" << cur_process << "}" << cur_cycle << " - " << cur_cycle_time * cur_cycle << " ms" << endl;


    }
    queue<tuple<float, string>> output = output_queue;
    vector<tuple<float, string>> output_v;
    while(!output.empty()){
        output_v.push_back(output.front());
        output.pop();
    }


    sort(output_v.begin(), output_v.end());
    for(int i = 0; i < output_v.size(); i++){
        cout << get<1>(output_v[i]);
    }



}

void Simulator::logToFile(queue<tuple<char, string, int>> queue_copy){
    ofstream fout;
    try {
        fout.open(program_config->getMiscConfigDetail("log_file_path"));

        if(program_config->getMiscConfigDetail("log_file_path").find("lgf") == -1){
            throw runtime_error("Wrong Log File Extension!");
        }
    }
    catch(std::runtime_error& e){
        cerr << "An exception occurred: " << e.what() << endl;
        exit(EXIT_FAILURE);
    }

    fout << endl;
    fout << "Configuration File Data" << endl;
    fout << "Monitor = " << program_config->getCycleTime("monitor") << " ms/cycle" << endl;
    fout << "Processor = " << program_config->getCycleTime("run") << " ms/cycle" << endl;
    fout << "Mouse = " << program_config->getCycleTime("mouse") << " ms/cycle" << endl;
    fout << "Hard Drive = " << program_config->getCycleTime("hard drive") << " ms/cycle" << endl;
    fout << "Keyboard = " << program_config->getCycleTime("keyboard") << " ms/cycle" << endl;
    fout << "Memory = " << program_config->getCycleTime("allocate") << " ms/cycle" << endl;
    fout << "Printer = " << program_config->getCycleTime("printer") << " ms/cycle" << endl;
    fout << "Logged to: ";
    if(program_config->getMiscConfigDetail("log_type") == "Log to File"){
        fout << program_config->getMiscConfigDetail("log_file_path");
    }
    else if(program_config->getMiscConfigDetail("log_type") == "Log to Monitor"){
        fout << "monitor";
    }
    else{
        fout << "monitor and " << program_config->getMiscConfigDetail("log_file_path");
    }
    fout << endl << endl;

    fout << "Meta Data Metrics" << endl;

    while(!instruction_queue.empty()){
        tuple<char, string, int> cur_instruction = queue_copy.front();
        char cur_type = get<0>(cur_instruction);
        string cur_process = get<1>(cur_instruction);
        int cur_cycle = get<2>(cur_instruction);
        int cur_cycle_time;

        queue_copy.pop();

        if(cur_type == 'S' && cur_process == "finish"){
            break;
        }

        if(cur_type == 'S' || cur_type == 'A'){
            continue;
        }




        cur_cycle_time = program_config->getCycleTime(cur_process);

        fout << cur_type << "{" << cur_process << "}" << cur_cycle << " - " << cur_cycle_time * cur_cycle << " ms" << endl;


    }
    queue<tuple<float, string>> output = output_queue;
    vector<tuple<float, string>> output_v;
    while(!output.empty()){
        output_v.push_back(output.front());
        output.pop();
       }

    sort(output_v.begin(), output_v.end());
    for(int i = 0; i < output_v.size(); i++){
        fout << get<1>(output_v[i]);
    }


}

void * Simulator::handleKeyboard(void * num_cycles){
    pthread_mutex_lock(&keyboard);
    string type = "keyboard";
    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    start << fixed;
    end << fixed;
    int * data_pointer = (int*) num_cycles;
    int cycles = *data_pointer;
    float time_stamp = getTimeStamp();


    start << setprecision(6) << time_stamp << " - Process " << handled_processes << " start " << type << " input" << endl;
    pushToOutput(time_stamp, start.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << handled_processes << " end " << type << " input" << endl;
    pushToOutput(time_stamp, end.str());
    pthread_mutex_unlock(&keyboard);

}

void * Simulator::handleMouse(void * num_cycles){
    pthread_mutex_lock(&mouse);
    string type = "mouse";
    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    start << fixed;
    end << fixed;
    int * data_pointer = (int*) num_cycles;
    int cycles = *data_pointer;

    float time_stamp = getTimeStamp();


    start << setprecision(6) << time_stamp << " - Process " << handled_processes << " start " << type << " input" << endl;
    pushToOutput(time_stamp, start.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << handled_processes << " end " << type << " input" << endl;
    pushToOutput(time_stamp, end.str());
    pthread_mutex_unlock(&mouse);
}

void * Simulator::handleMonitor(void * num_cycles){
    pthread_mutex_lock(&monitor);
    string type = "monitor";
    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    start << fixed;
    end << fixed;
    int * data_pointer = (int*) num_cycles;
    int cycles = *data_pointer;

    float time_stamp = getTimeStamp();


    start << setprecision(6) << time_stamp << " - Process " << handled_processes << " start " << type << " output" << endl;
    pushToOutput(time_stamp, start.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << handled_processes << " end " << type << " output" << endl;
    pushToOutput(time_stamp, end.str());
    pthread_mutex_unlock(&monitor);
}

void * Simulator::handlePrinter(void * num_cycles){
    int printer_num = printer_s.getCount();
    printer_s.wait();
    string type = "printer";
    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    start << fixed;
    end << fixed;
    int process_number = handled_processes;
    int * data_pointer = (int*) num_cycles;
    int cycles = *data_pointer;

    float time_stamp = getTimeStamp();


    start << setprecision(6) << time_stamp << " - Process " << process_number << " start " << type << " output on PRINT" << printer_num << endl;
    pushToOutput(time_stamp, start.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << process_number << " end " << type << " output on PRINT" << printer_num << endl;
    pushToOutput(time_stamp, end.str());

    printer_s.signal();
}

void * Simulator::handleHarddrive(void * hdd_block){
    HDDThreadBlock * hdd_thread_block_p = (HDDThreadBlock* ) hdd_block;
    HDDThreadBlock hdd_thread_block = *hdd_thread_block_p;
    int hdd_num = harddrive_s.getCount();
    int process_number = handled_processes;
    harddrive_s.wait();
    string type = "hard drive";
    ostringstream start;
    start.precision(6);
    ostringstream end;
    end.precision(6);
    start << fixed;
    end << fixed;


    int cycles = hdd_thread_block.num_cycles;
    //cout << cycles << endl;
    string io_type;

    if(hdd_thread_block.io_type == 'i'){
        io_type = "input";
    }
    else{
        io_type = "output";
    }
    //cout << io_type << endl;
    //cout << hdd_thread_block.io_type << endl;
    float time_stamp = getTimeStamp();


    start << setprecision(6) << time_stamp << " - Process " << process_number << " start " << type << " " << io_type<< " on HDD" << hdd_num << endl;
    pushToOutput(time_stamp, start.str());
    std::this_thread::sleep_for(std::chrono::milliseconds(int(program_config->getCycleTime(type) * cycles)) );
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << process_number << " end " << type << " " << io_type << " on HDD" << hdd_num << endl;
    pushToOutput(time_stamp, end.str());

    harddrive_s.signal();
}

void Simulator::pushToOutput(float time_stamp, string s){
    pthread_mutex_lock(&output_queue_m);
    tuple<float, string> output = {time_stamp, s};
    output_queue.push(output);
    pthread_mutex_unlock(&output_queue_m);
}

void Simulator::populateProcessVector(){
    auto queue_copy = instruction_queue;

    //Remove any instructions that are not part of a process
    while(get<0>(queue_copy.front()) != 'A'){
        queue_copy.pop();
    }

    while(!queue_copy.empty()){
        if(get<0>(queue_copy.front()) == 'S'){
            queue_copy.pop();
        }

        queue<tuple<char, string, int>> cur_process;
        while(get<1>(queue_copy.front()) != "finish"){
            cur_process.push(queue_copy.front());
            queue_copy.pop();
        }

        if(get<1>(queue_copy.front()) == "finish"){
            cur_process.push(queue_copy.front());
            queue_copy.pop();
        }

        Process  out_proc(&cur_process, program_config);
        if(get<0>(queue_copy.front()) == 'S'){
            queue_copy.pop();
        }


        active_processes.push_back(out_proc);
    }


}

void Simulator::executeInstruction(Process * cur_proc){

}
