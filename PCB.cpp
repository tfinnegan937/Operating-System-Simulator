//
// Created by plays on 9/30/2019.
//

#include "PCB.h"

string PCB::validateState(string state){
    try {
        if (!(state == "START" || state == "READY" || state == "RUNNING" || state == "WAITING" || state == "EXIT")) {
            throw logic_error(state + " is not a valid state.\n");
        }
    }
    catch(exception &e){
        cout << e.what();
        exit(EXIT_FAILURE);
    }

    return state;
}

void PCB::setState(string state) {
    state = validateState(state);
}

string PCB::getState() const{
    return state;
}
