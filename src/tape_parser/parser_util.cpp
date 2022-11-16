#include "parser_util.hpp"


const char* ws = " \t\n\r\f\v";

std::string& rtrim(std::string& s) {
    s.erase(s.find_last_not_of(ws) + 1);
    return s;
}

std::string& ltrim(std::string& s) {
    s.erase(0, s.find_first_not_of(ws));
    return s;
}

std::string& trim(std::string& s) {
    return ltrim(rtrim(s));
}

