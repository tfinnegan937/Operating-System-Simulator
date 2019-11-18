//
// Created by tim on 9/7/19.
//

#ifndef ASSIGNMENT_1_CONFIG_PARSER_H
#define ASSIGNMENT_1_CONFIG_PARSER_H
#include "parser.h"
#include <fstream>
#include <string>
#include <utility>
#include <type_traits>
#include <map>

using namespace std;

class ConfigParser : public Parser<map<string, string>> {
private:
    ifstream file;
    string removeWhiteSpace(string input);

    map<string, string> output_dictionary;

public:
    ConfigParser();
    ConfigParser(string file_name);
    ~ConfigParser();
    void parse() override;
    map<string, string> retrieveFormattedOutput() const override;

};

#endif //ASSIGNMENT_1_CONFIG_PARSER_H
