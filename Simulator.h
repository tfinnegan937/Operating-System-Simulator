/*
File Purpose: To define the Simulator class that serves as the body of the program.

 Attribute Definitions:

    program_config: This variable initializes the config data from the config file and then provides member functions
    that allow for the program to access configuration details. It is of the Config class, which is defined in Config.h

    instruction_queue: This variable is a queue of tuples. It contains the instructions in the form (type, instruction, #cycles)
    in the order that they were listed in the meta data file

 Method Definitions:

    Constructors:
        Simulator(string config_file_path): This constructor takes the path to the config as a parameter. It then initializes
        program_config with this config_file_path, which then reads and parses the data. It then reads the metadata file
        path from program_config, and then parses that data. The resulting data is then placed into instruction_queue.

    Helper Functions:
        void logToBoth(queue<tuple<char, string, int>>): This function takes a copy of the instruction queue and then passes
        it to logToMonitor() and logToFile()

        void logToFile(queue<tuple<char, string, int>>): This function reads the log file from program_config and then opens it.
        It then multiplies the queue input (a copy of instruction_queue) by the appropriate cycle time listed in program_config
        for each instruction and then outputs it to a file.

        void logToMonitor(queue<tuple<char, string, int>>)

        tuple<char, string, int> validateInput(tuple<char, string, int> instruction): Takes in an instruction tuple as input.
        It then validates the tuple. If the tuple is validated, it returns the original, unaltered parameter. If it is not,
        it returns an error and ends the program.

    Public Functions:
        void run(): As the constructor for Simulator initializes the appropriate configuration files, all that the run
        function does is run the appropriate logging function based upon the configuration rules.
*/
#ifndef ASSIGNMENT_1_SIMULATOR_H
#define ASSIGNMENT_1_SIMULATOR_H
#include "Config.h"
#include "input_parser.h"
#include "PCB.h"
#include "Semaphore.h"
#include <fstream>
#include <queue>
#include <tuple>
#include <pthread.h>
#include <chrono>
#include <string>
using namespace std::chrono;

class Simulator {
private:
    static Config * program_config;
    queue<tuple<char, string, int>> instruction_queue;
    static queue<tuple<char, string, int>> drive_queue;
    static queue<tuple<char, string, int>> print_queue;
    static queue<string> output_queue;
    static nanoseconds start_time;
    static int size;
    static PCB pcb;
    static pthread_t tinput;
    static pthread_t toutput;
    static pthread_mutex_t keyboard;
    static pthread_mutex_t mouse;
    static pthread_mutex_t monitor;
    static pthread_mutex_t harddrive;
    static pthread_mutex_t printer;
    static Semaphore harddrive_s;
    static Semaphore printer_s;
    static int handled_processes;
    static string file_output;
    static float getTimeStamp();
    static void *ProcessInput(void *inputtype);
    static void *ProcessOutput(void *outputtype);
    long int cur_mem;
    void processMemory(tuple<char, string, int> instruction);
    void processProcessRun(tuple<char, string, int> instruction);
    void processProcessOperation(tuple<char, string, int> instruction);
    void logToBoth(queue<tuple<char, string, int>> queue_copy);
    void logToMonitor(queue<tuple<char, string, int>> queue_copy);
    void logToFile(queue<tuple<char, string, int>> queue_copy);

    static void outputOperationLog(float time_stamp, string tag, string operation);

    void cpuLoop();

public:
    Simulator(string config_file_path);
    void run();
};


#endif //ASSIGNMENT_1_SIMULATOR_H
