//
// Generic Interface for text parsers of various types
//

#ifndef ASSIGNMENT_1_PARSER_H
#define ASSIGNMENT_1_PARSER_H
#include <string>
using namespace std;
template <class OutputType>
class Parser{
public:

    virtual void parse() = 0;
    virtual OutputType retrieveFormattedOutput() const = 0;
};
#endif //ASSIGNMENT_1_PARSER_H
