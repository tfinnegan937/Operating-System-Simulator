////////////////////////////////////////////////////////////////////////////////////

CS 446 - Principles of Operating Systems: Assignment 3

Author: Timothy Finnegan
Submission Date: 10/4/2019
Completion Date: 10/3/2019

/////////////////////////////////////////////////////////////////////////////////////

Description: This program, run through ./assignment3 [config file path].conf, reads in a config file and some rules regarding
the time that it takes a set of processes to complete a cycle and what to do with that information, and then a meta data
file including a set of instructions and the number of cycles, and then logs the contents of the configuration file,
and the list of instructions and the amount of time that they took to complete. 

The program then attempts to simulate the execution of these instruction in the time that it would usually take to execute.

The threads are created at lines 153 and 159 of Simulator.cpp

Mutexes are defined in Simulator.h, and used in lines 68-151 of Simulator.cpp, and 153-226 of Simulator.cpp

Sephamores are defined in Sephamore.h and Sephamore.cpp, and used in the same lines as the mutexes


Compilation:

To Compile: make
To Clean: make clean

The program is designed to be compiled in an Ubuntu enviroment, but will likely compile for Windows as well.

Running:

Execute the following command

[Path To Executable]/assignment3 [path to config file]/[config file name].conf
