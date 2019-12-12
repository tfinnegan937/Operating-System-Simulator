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
pthread_t Simulator::quantum_t;

pthread_mutex_t Simulator::printer;
pthread_mutex_t Simulator::mouse;
pthread_mutex_t Simulator::keyboard;
pthread_mutex_t Simulator::monitor;
pthread_mutex_t Simulator::harddrive;
pthread_mutex_t Simulator::output_queue_m;

Process * Simulator::current_process;
vector<Process> Simulator::active_processes;

Semaphore Simulator::printer_s;
Semaphore Simulator::harddrive_s;
bool Simulator::quantum_interrupt;
bool Simulator::lower_time_interrupt;

queue<tuple<float, string>> Simulator::output_queue;
queue<tuple<char, string, int>> Simulator::print_queue;
queue<tuple<char, string, int>> Simulator::drive_queue;


int Simulator::process_index;
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

    process_index = 0;

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
    start << setprecision(6) << time_stamp << " - Process " << process_index + 1 << ": start processing action\n";
    pushToOutput(time_stamp, start.str());
    current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
    if(current_process->getCurInsTimeRemaining() == 0) {
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << process_index + 1<< ": end processing action\n";
        pushToOutput(time_stamp, end.str());
        current_process->popEmptyInstruction(program_config);
    }
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
    int process_number = process_index + 2;
    prep << fixed;
    start << fixed;
    remove << fixed;
    if(type == "begin"){
        
        process_number = process_index;
        time_stamp = getTimeStamp();
        prep << setprecision(6) << time_stamp << " - OS: preparing process " << process_index + 1<< endl;
        pushToOutput(time_stamp, prep.str());
        time_stamp = getTimeStamp();
        start << setprecision(6) << time_stamp << " - OS: starting process " << process_index + 1 << endl;
        pushToOutput(time_stamp, start.str());
        pcb.setState("START");
    }
    else{
        time_stamp = getTimeStamp();
        remove << setprecision(6) << time_stamp << " - OS: removing process " << process_number << endl;
        pushToOutput(time_stamp, remove.str());
        pcb.setState("EXIT");
    }

    current_process->popEmptyInstruction(program_config);
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
        start << setprecision(6) << time_stamp << " - Process " << process_index + 1<< ": start memory blocking\n";
        pushToOutput(time_stamp, start.str());
        current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
        if(current_process->getCurInsTimeRemaining() == 0) {
            time_stamp = getTimeStamp();
            end << setprecision(6) << time_stamp << " - Process " << process_index + 1<< ": end memory blocking\n";
            pushToOutput(time_stamp, end.str());
            current_process->popEmptyInstruction(program_config);
        }
    }
    else {
        current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
        if(current_process->getCurInsTimeRemaining() == 0) {
            cur_mem = cur_mem + stoi(program_config->getMiscConfigDetail("bsize"));
            time_stamp = getTimeStamp();
            allocate << setprecision(6) << time_stamp << " - Process " << process_index + 1
                     << ": memory allocated at 0x" << std::hex << cur_mem << std::dec << endl;
            pushToOutput(time_stamp, allocate.str());
            current_process->popEmptyInstruction(program_config);
        }
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
    /*while(!queue_copy.empty() /*|| !drive_queue.empty() || !print_queue.empty()){
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

    }*/
    auto schedule_type = program_config->getMiscConfigDetail("scheduler");
    if(schedule_type == "RR") {
        rrExec();
    }
    else{

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


    start << setprecision(6) << time_stamp << " - Process " << process_index + 1<< " start " << type << " input" << endl;
    pushToOutput(time_stamp, start.str());
    current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
    if(current_process->getCurInsTimeRemaining() == 0) {
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << process_index + 1<< " end " << type << " input"
            << endl;
        pushToOutput(time_stamp, end.str());
        current_process->popEmptyInstruction(program_config);
    }
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


    start << setprecision(6) << time_stamp << " - Process " << process_index + 1<< " start " << type << " input" << endl;
    pushToOutput(time_stamp, start.str());
    current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
    if(current_process->getCurInsTimeRemaining()) {
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << process_index + 1 << " end " << type << " input"
            << endl;
        pushToOutput(time_stamp, end.str());
        current_process->popEmptyInstruction(program_config);
    }
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


    start << setprecision(6) << time_stamp << " - Process " << process_index + 1 << " start " << type << " output" << endl;
    pushToOutput(time_stamp, start.str());
    current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
    if(current_process->getCurInsTimeRemaining() == 0) {
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << process_index + 1 << " end " << type << " output"
            << endl;
        pushToOutput(time_stamp, end.str());
        current_process->popEmptyInstruction(program_config);
    }
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
    int process_number = process_index + 1;
    int * data_pointer = (int*) num_cycles;
    int cycles = *data_pointer;

    float time_stamp = getTimeStamp();


    start << setprecision(6) << time_stamp << " - Process " << process_number << " start " << type << " output on PRINT" << printer_num << endl;
    pushToOutput(time_stamp, start.str());
    current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
    if(current_process->getCurInsTimeRemaining() <= 0) {
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << process_number << " end " << type << " output on PRINT"
            << printer_num << endl;
        pushToOutput(time_stamp, end.str());
        current_process->popEmptyInstruction(program_config);
    }
    printer_s.signal();
}

void * Simulator::handleHarddrive(void * hdd_block){
    HDDThreadBlock * hdd_thread_block_p = (HDDThreadBlock* ) hdd_block;
    HDDThreadBlock hdd_thread_block = *hdd_thread_block_p;
    int hdd_num = harddrive_s.getCount();
    int process_number = process_index + 1;
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
    current_process->setCurTimeRemaining(interruptableWait(std::chrono::milliseconds(current_process->getCurInsTimeRemaining()), &quantum_interrupt, &lower_time_interrupt ));
    //cout << current_process->getCurInsTimeRemaining() << endl;
    if(current_process->getCurInsTimeRemaining() == 0) {
        auto ins = current_process->getNextInstruction(program_config);
        //cout << "Hard Drive Complete " << get<0>(ins) << " " << get<1>(ins) << " " << get<2>(ins);
        time_stamp = getTimeStamp();
        end << setprecision(6) << time_stamp << " - Process " << process_number << " end " << type << " " << io_type
            << " on HDD" << hdd_num << endl;
        pushToOutput(time_stamp, end.str());
        current_process->popEmptyInstruction(program_config);
    }

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

    /*while(!active_processes.empty()){
        auto front = active_processes.front();
        while(!front.empty()){
            cout << "Time_remaining: " << front.getTimeRemaining() << endl;
            front.popEmptyInstruction(program_config);
        }
        active_processes.erase(active_processes.begin());
    }*/

}

milliseconds interruptableWait(milliseconds time_wait, bool* qinterrupt, bool * linterrupt){
    milliseconds start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds cur_time = start_time;

    while( (cur_time - start_time < time_wait) && !*qinterrupt && !*linterrupt){
        cur_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    }

    if(cur_time - start_time >= time_wait){
        return std::chrono::milliseconds(0);
    }

    return time_wait - (cur_time - start_time);
}

void Simulator::executeInstruction(){
    auto cur_instruction = current_process->getNextInstruction(program_config);
    //cout << "Instruction: " << get<0>(cur_instruction) << endl;
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

void Simulator::rrExec(){
    process_index = 0;
    current_process = &active_processes[process_index];
    pthread_create(&quantum_t, NULL, handleQuantum, NULL);
    //cout << "DO NOTHING! " << int(allEmpty()) << endl;
    while(!allEmpty()){
        //cout << "Process Index: " << process_index << " Time remaining " << current_process->getTimeRemaining() << endl;
        executeInstruction();
        //cout << "Instruction Executed" << endl;
        if(current_process->getCurInsTimeRemaining() == 0 && !current_process->empty()){
            //current_process->popEmptyInstruction(program_config);
        }
        //cout << getTimeStamp() << " Process " << process_index + 1 << " " << current_process->getTimeRemaining() << endl;
        rrHandleInterrupt();

        if(current_process->empty() && !allEmpty()){
            //cout << "Switch on Complete " << process_index + 1 << endl;

            process_index++;
            if(process_index == active_processes.size()){
                process_index = 0;
            }
            while(active_processes[process_index].empty() && !allEmpty()){
                //cout << "Complete hang" << endl;
                process_index++;
                //cout << "index " << process_index << endl;
                if(process_index == active_processes.size()){
                    process_index = 0;
                }

            }
            //cout << "Switched to " << process_index + 1 << endl;
            current_process = &active_processes[process_index];
        }
    }
}

void Simulator::rrHandleInterrupt(){
    if(quantum_interrupt && !allEmpty()){
        pthread_join(quantum_t, NULL);
        ostringstream output;
        output.precision(6);
        output << fixed;

        ostringstream load;
        load.precision(6);
        load << fixed;

        auto time_stamp = getTimeStamp();
        output << time_stamp << "- Process " << process_index + 1 << " interrupt processing action" << endl;
        pushToOutput(time_stamp, output.str());
        if(!allEmpty()){
            //cout << "Switch on Interrupt" << endl;
            process_index++;
            if(process_index == active_processes.size()){
                process_index = 0;
            }
            while(active_processes[process_index].empty() && !allEmpty()){
                //cout << "Start" << endl;
                process_index++;
                if(process_index == active_processes.size()){
                    process_index = 0;
                }
                //cout << "index " << process_index << endl;
                //cout << "end" << endl;
            }

            current_process = &active_processes[process_index];
            if(get<0>(current_process->getNextInstruction(program_config)) != 'A') {
                load << setprecision(6) << time_stamp << " - OS: resuming process " << process_index + 1 << endl;
                pushToOutput(getTimeStamp(), load.str());
            }
        }

        quantum_interrupt = false;
        pthread_create(&quantum_t, NULL, handleQuantum, NULL);
    }
}

bool Simulator::allEmpty(){
    bool out = false;
    //cout << "allEmpty Called" << endl;
    for(int i = 0; i < active_processes.size(); i++){
        if(active_processes[i].empty()){
            out = true;

        }
            /*cout << i << " Empty? ";
            if(active_processes[i].empty()){
                cout << "True";
            }
            else{
                cout << "False";
            }
            cout << endl;*/

    }
    //cout << "out "  << int(out) << endl;
    return out;
}

void * Simulator::handleQuantum(void *n) {
    milliseconds quantum_length = std::chrono::milliseconds(stoi(program_config->getMiscConfigDetail("quantum")));
    milliseconds start = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds cur = start;

    while(cur-start < quantum_length){
        cur = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    };
    //cout << "QUANTUM\n";
    quantum_interrupt = true;
    pthread_exit(NULL);
}
