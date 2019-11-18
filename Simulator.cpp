//
// Created by tim on 9/9/19.
//

#include "Simulator.h"
#include <iostream>
#include <iomanip>
#include <unistd.h>

Config * Simulator::program_config;
nanoseconds Simulator::start_time;
PCB Simulator::pcb;

pthread_t Simulator::tinput;
pthread_t Simulator::toutput;

pthread_mutex_t Simulator::printer;
pthread_mutex_t Simulator::mouse;
pthread_mutex_t Simulator::keyboard;
pthread_mutex_t Simulator::monitor;
pthread_mutex_t Simulator::harddrive;

Semaphore Simulator::printer_s;
Semaphore Simulator::harddrive_s;

queue<string> Simulator::output_queue;
queue<tuple<char, string, int>> Simulator::print_queue;
queue<tuple<char, string, int>> Simulator::drive_queue;


int Simulator::handled_processes;
int Simulator::size;



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

    delete metadata_parser;

    metadata_parser = nullptr;
}

void Simulator::outputOperationLog(float time_stamp, string tag, string operation) {
    cout << time_stamp << " - " << tag << ": " << operation << endl;
}

float Simulator::getTimeStamp() {
    nanoseconds time_stamp = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

    nanoseconds cur_time = time_stamp - start_time;

    return float(cur_time.count())/1000000;
}

void * Simulator::ProcessOutput(void *outputtype) {

    tuple<char, string, int> * instructionp = (tuple<char, string, int> *) outputtype;
    tuple<char, string, int> instruction = *instructionp;

    string type = get<1>(instruction);

    ostringstream start;
    ostringstream end;
    int count;
    if(type == "keyboard"){
        pthread_mutex_lock(&keyboard);
    }
    if(type == "mouse"){
        pthread_mutex_lock(&mouse);
    }

    if(type == "hard drive"){
        harddrive_s.wait();
        count = harddrive_s.getCount();
        if(count == stoi(program_config->getMiscConfigDetail("hcount"))){
            drive_queue.push(instruction);
            return (void*)0;
        }
    }

    if(type == "printer"){
        printer_s.wait();
        count = printer_s.getCount();
        if(count == stoi(program_config->getMiscConfigDetail("pcount"))){
            drive_queue.push(instruction);
            return (void*)0;
        }
    }

    if(type == "monitor"){
        pthread_mutex_lock(&monitor);
    }
    int cycles = get<2>(instruction);

    float time_stamp = getTimeStamp();

    start << setprecision(6) << time_stamp << " - Process " << handled_processes << " start " << type << " output";
    if(type == "printer"){
        start << " on PRINT" << count-1;
    }
    if(type == "hard drive"){
        start << " on HDD" << count-1;
    }
    start << endl;
    output_queue.push(start.str());
    sleep((program_config->getCycleTime(type) * cycles) / 1000.0);
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << handled_processes << " end " << type << " output";

    if(type == "printer"){
        end << " on PRINT" << count -1;
    }
    if(type == "hard drive"){
        end << " on HDD" << count -1;
    }

    end << endl;
    output_queue.push(end.str());

    if(type == "keyboard"){
        pthread_mutex_unlock(&keyboard);
    }
    if(type == "mouse"){
        pthread_mutex_unlock(&mouse);
    }

    if(type == "hard drive"){
        harddrive_s.signal();
    }

    if(type == "printer"){
        printer_s.signal();
    }

    if(type == "monitor"){
        pthread_mutex_unlock(&monitor);
    }
}

void * Simulator::ProcessInput(void *inputtype) {
    cout << setprecision(6);
    tuple<char, string, int> * instructionp = (tuple<char, string, int> *) inputtype;
    tuple<char, string, int> instruction = *instructionp;

    string type = get<1>(instruction);
    ostringstream start;
    ostringstream end;
    int count;

    if(type == "keyboard"){
        pthread_mutex_lock(&keyboard);
    }
    if(type == "mouse"){
        pthread_mutex_lock(&mouse);
    }

    if(type == "hard drive"){
        harddrive_s.wait();
        count = harddrive_s.getCount();
    }

    if(type == "printer"){
        printer_s.wait();
        count = printer_s.getCount();
    }

    if(type == "monitor"){
        pthread_mutex_lock(&monitor);
    }
    int cycles = get<2>(instruction);

    float time_stamp = getTimeStamp();

    start << setprecision(6) << time_stamp << " - Process " << handled_processes << " start " << type << " input";
    if(type == "printer"){
        start << " on PRINT" << count - 1;
    }
    if(type == "hard drive"){
        start << " on HDD" << count - 1;
    }
    start << endl;
    output_queue.push(start.str());
    sleep((program_config->getCycleTime(type) * cycles) / 1000.0);
    time_stamp = getTimeStamp();
    end << setprecision(6) << time_stamp << " - Process " << handled_processes << " end " << type << " input";

    if(type == "printer"){
        end << " on PRINT" << count - 1;
    }
    if(type == "hard drive"){
        end << " on HDD" << count - 1;
    }
    end << endl;
    output_queue.push(end.str());
    if(type == "keyboard"){
        pthread_mutex_unlock(&keyboard);
    }
    if(type == "mouse"){
        pthread_mutex_unlock(&mouse);
    }

    if(type == "hard drive"){
        harddrive_s.signal();
    }

    if(type == "printer"){
        printer_s.signal();
    }

    if(type == "monitor"){
        pthread_mutex_unlock(&monitor);
    }
}

void Simulator::processProcessRun(tuple<char, string,  int> instruction) {
    cout << setprecision(6);

    string type = get<1>(instruction);
    float cycles = get<2>(instruction);
    ostringstream start;
    ostringstream end;
    start << setprecision(6) << getTimeStamp() << " - Process " << handled_processes << ": start processing action\n";
    output_queue.push(start.str());
    sleep((program_config->getCycleTime(type) * cycles) / 1000.0);
    end << setprecision(6) << getTimeStamp() << " - Process " << handled_processes << ": end processing action\n";
    output_queue.push(end.str());
}

void Simulator::processProcessOperation(tuple<char, string, int> instruction) {
    cout << setprecision(6);

    string type = get<1>(instruction);
    int cycles = get<2>(instruction);
    ostringstream prep;
    ostringstream start;
    ostringstream remove;
    if(type == "begin"){
        handled_processes++;
        prep << setprecision(6) << getTimeStamp() << " - OS: preparing process " << handled_processes << endl;
        output_queue.push(prep.str());
        start << setprecision(6) << getTimeStamp() << " - OS: starting process " << handled_processes << endl;
        output_queue.push(start.str());
        pcb.setState("START");
    }
    else{
        remove << setprecision(6) << getTimeStamp() << " - OS: removing process " << handled_processes << endl;
        output_queue.push(remove.str());
        pcb.setState("EXIT");
    }
}

void Simulator::processMemory(tuple<char, string, int> instruction) {
    cout << setprecision(6);

    string type = get<1>(instruction);
    int cycles = get<2>(instruction);

    string mem_space = "0xFF8C"; // Temporary placeholder memory
    ostringstream start;
    ostringstream end;
    ostringstream allocate;

    if(type == "block"){
        start << setprecision(6) << getTimeStamp() << " - Process " << handled_processes << ": start memory blocking\n";
        output_queue.push(start.str());
        sleep((program_config->getCycleTime(type) * cycles) / 1000.0);
        end << setprecision(6) << getTimeStamp() << " - Process " << handled_processes << ": end memory blocking\n";
        output_queue.push(end.str());
    }
    else {
        sleep((program_config->getCycleTime(type) * cycles) / 1000.0);

        cur_mem = cur_mem + stoi(program_config -> getMiscConfigDetail("bsize"));
        allocate << setprecision(6) << getTimeStamp() << " - Process " << handled_processes << ": memory allocated at 0x" << std::hex << cur_mem << std::dec << endl;
        output_queue.push(allocate.str());

    }
}

void Simulator::cpuLoop(){
    cout << setprecision(6);
    pthread_mutex_init(&mouse, NULL);
    pthread_mutex_init(&keyboard, NULL);
    pthread_mutex_init(&monitor, NULL);
    harddrive_s.init(&harddrive, stoi(program_config->getMiscConfigDetail("hcount")));
    printer_s.init(&printer, stoi(program_config->getMiscConfigDetail("pcount")));

    start_time = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
    auto queue_copy = instruction_queue;
    ostringstream start;
    ostringstream end;
    start << setprecision(6) << getTimeStamp() << " - Simulator program starting\n";
    output_queue.push(start.str());
    tuple<char, string, int> cur_instruction;
    while(!queue_copy.empty() || !drive_queue.empty() || !print_queue.empty()){
        pcb.setState("RUNNING");
        if(!drive_queue.empty()){
            if (harddrive_s.getCount() < stoi(program_config->getMiscConfigDetail("hcount"))){
                cur_instruction = drive_queue.front();
                drive_queue.pop();
            }
            else{
                if(queue_copy.empty()){
                    break;
                }
                cur_instruction = queue_copy.front();
                queue_copy.pop();
            }
        }
        else if(!print_queue.empty()){
            if (printer_s.getCount() < stoi(program_config->getMiscConfigDetail("pcount"))){
                cur_instruction = print_queue.front();
                print_queue.pop();
            }
            else{
                if(queue_copy.empty()){
                    break;
                }
                cur_instruction = queue_copy.front();
                queue_copy.pop();
            }
        }
        else{
            if(queue_copy.empty()){
                break;
            }
            cur_instruction = queue_copy.front();
            queue_copy.pop();
        }
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
                pthread_create(&tinput, NULL, ProcessInput, &cur_instruction);
                pthread_join(tinput, NULL);
                pcb.setState("READY");
                break;
            case 'O':
                pcb.setState("WAITING");
                pthread_create(&toutput, NULL, ProcessOutput, &cur_instruction);
                pthread_join(tinput, NULL);
                pcb.setState("READY");
                break;
            case 'M':
                this->processMemory(cur_instruction);
                break;

        }

    }
    pthread_join(tinput, NULL);
    pthread_join(toutput, NULL);
    end << setprecision(6) << getTimeStamp() << " - Simulator program ending\n";
    output_queue.push(end.str());
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
    queue<string> output = output_queue;
    while(!output.empty()){
        cout << output.front();
        output.pop();
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
    queue<string> output = output_queue;
    while(!output.empty()){
        fout << output.front();
        output.pop();
    }
    fout.close();

}