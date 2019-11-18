//
// Created by tim on 9/7/19.
//

#ifndef ASSIGNMENT_1_INPUT_PARSER_H
#define ASSIGNMENT_1_INPUT_PARSER_H
#include "parser.h"
#include <fstream>
#include <tuple>
#include <queue>
#include <string>
using namespace std;

class InputParser : public Parser<queue<tuple<char, string, int>>>{
private:
    ifstream file;
    string removeWhiteSpace(string input);
    queue<tuple<char, string, int>> instruction_queue;

    tuple<char, string, int> tupleFromString(string instruction);
    tuple<char, string, int> validateInput(tuple<char, string, int> instruction);

public:
    InputParser(string file_name);
    ~InputParser();
    void parse() override;
    queue<tuple<char, string, int>> retrieveFormattedOutput() const override;
};

#endif //ASSIGNMENT_1_INPUT_PARSER_H
