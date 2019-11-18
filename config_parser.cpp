//
// Created by tim on 9/7/19.
//
#include "config_parser.h"
#include <iostream>
#include <stdexcept>
ConfigParser::ConfigParser(){

}

ConfigParser::ConfigParser(string file_name){
    try{
        file.open(file_name);
        if(!file.is_open()){
            throw runtime_error("Could not find file!!");
        }
        if(file_name.find(".conf") == -1){
            throw runtime_error("Wrong Config File Extension!");
        }
    }
    catch(std::runtime_error& e){
        cerr << "An exception occurred. Could not open Config file at " << file_name << "Reason: " << e.what() <<  "!\n";
        exit(EXIT_FAILURE);
    }

}

ConfigParser::~ConfigParser(){
    file.close();
}

void ConfigParser::parse(){
    string current_line = " ";
    string delimiter = ":";

    while(!file.eof()){
        getline(file, current_line);

        if(current_line.find("Start") == -1 && current_line.find("End") == -1){ //Ignore the starting and ending lines
            string rule;
            string value;

            size_t pos = current_line.find(delimiter);

            rule = removeWhiteSpace(current_line.substr(0, pos));
            value = removeWhiteSpace(current_line.substr(pos + 1, current_line.length()));
            output_dictionary.insert(pair<string, string>(rule, value));
        }

    }

}

map<string, string> ConfigParser::retrieveFormattedOutput() const{
    return output_dictionary;
}

    string ConfigParser::removeWhiteSpace(string in){

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