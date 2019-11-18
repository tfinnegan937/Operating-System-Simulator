/*
File Purpose: To define the Simulator class that serves as the body of the program.

 Attribute Definitions:
    cycle_times: cycle_times maps the configuration attributes listed in the appropriate configuration rule to the cycle
    times assigned to them. This variable is accessed by getCycleTime(string);

    misc_configuration_details: maps the configuration attributes listed in the appropriate configuration rule to the
    string that defines its functionality. This includes the meta data file name, the log file name, and the logging type.


 Method Definitions:

    Constructors:
        Config(): Empty Default Constructor

    Helper Functions:
        void initializeConfigData(map<string, string> parser_output): This function is called inside of readFile(). After
        the file has been read, this function pushes the appropriate data into the appropriate maps for later use.



    Public Functions:

        void readFile(string file_name): This function reads the file listed in the parameter file_name and then sends it
        to a config_parser object. That parser returns a map of strings, which is then pushed to the cycle_times and
        misc_configuration_details queues

        int getCycleTime(string type): Returns the cycle duration of the type of operation described by string type.

        string getMiscConfigDetail(string type) const: Returns the logging rule or file path described by string type.

*/

#ifndef ASSIGNMENT_1_CONFIG_H
#define ASSIGNMENT_1_CONFIG_H
#include "config_parser.h"
#include <string>
#include <map>
#include <utility>

using namespace std;

class Config {
private:
    map <string, int> cycle_times;
    map <string, string> misc_configuration_details;


    void initializeConfigData(map<string, string> parser_output); //Called inside of "readFile(string file_name)." Copies data from the parser into the proper data structures;

public:
    Config();

    void readFile(string file_name);
    int getCycleTime(string type) const;
    string getMiscConfigDetail(string type) const;

};


#endif //ASSIGNMENT_1_CONFIG_H
