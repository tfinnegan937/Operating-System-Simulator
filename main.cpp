/*
 * File Purpose: To provide an entry point to the program
 *
 * Execution: The entry point function takes in the program input, and then attempts to validate whether any configuration
 * file path has been input. If a configuration file path has not been inserted as a parameter, the program ends with a runtime error.
 *
 * If a parameter has been input, the program initializes the Simulator with the configuration file path, and then runs the simulator.
 */

#include <iostream>
#include "config_parser.h"
#include "input_parser.h"
#include "Config.h"
#include "Simulator.h"
#include <iomanip>
using namespace std;

int main(int argc, char * argv[]) {
    cout << setprecision(6);
    try{
        if(argc <= 1){
            throw runtime_error("No arguments!");
        }
    }
    catch(const runtime_error & rte){
        cerr << "No Arguments have been provided! Please provide a path to the configuration file.\n";
        exit(EXIT_FAILURE);
    }
    Simulator simulator(argv[1]);

    simulator.run();
    return 0;
}