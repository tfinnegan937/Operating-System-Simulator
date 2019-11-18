//
// Created by plays on 9/30/2019.
//

#ifndef ASSIGNMENT_1_PCB_H
#define ASSIGNMENT_1_PCB_H
#include <string>
#include <stdexcept>
#include <iostream>
using namespace std;
class PCB {
private:
    string state;

    string validateState(string state);
public:
    void setState(string state);
    string getState() const;
};


#endif //ASSIGNMENT_1_PCB_H
