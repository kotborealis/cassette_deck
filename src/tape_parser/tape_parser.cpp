#include <sstream>

#include "tape_parser.hpp"
#include "parser_util.hpp"

TapeParser::TapeParser(const std::string& tape) {
    using Kind = TapeCommand::Kind;

    std::istringstream ss(tape);
    int lineno = 0;

    std::string line;
    while(std::getline(ss, line)) {
        lineno++;
        line = trim(line);
        if(line.empty()) continue;

        const std::string err_suffix = " (line " + std::to_string(lineno) + ")";

        std::string cmd;
        auto cmd_end = line.find(' ');
        if(cmd_end == std::string::npos) {
            cmd = line;
            if(cmd == "ENTER")
                commands.push(new TapeCommandKey('\n'));
            else if(cmd == "BACKSPACE")
                commands.push(new TapeCommandKey('\b'));
            else if(cmd == "SPACE")
                commands.push(new TapeCommandKey(' '));
            else if(cmd == "TAB")
                commands.push(new TapeCommandKey('\t'));
            else if(cmd == "AWAIT")
                commands.push(new TapeCommandAwait(60 * 1000));
            // TODO: arrows
            else
                throw TapeException("(" + line + ") Unknown command: " + cmd + err_suffix);
        } else {
            cmd = line.substr(0, cmd_end);
            std::string arg = line.substr(cmd_end + 1);

            if(cmd == "TYPE")
                for(char c : arg) {
                    commands.push(new TapeCommandKey(c));
                    commands.push(new TapeCommandSleep(50));
                }
            else if(cmd == "REQUIRE") {
                if(system((std::string("which ") + arg + " > /dev/null 2>&1").c_str()))
                    throw TapeException("Required program " + arg + " not found" + err_suffix);
            }
            else if(cmd == "OUTPUT")
                _output = arg;
            else if(cmd == "SLEEP")
                commands.push(new TapeCommandSleep(std::stoi(arg)));
            else if(cmd == "WIDTH")
                _width = std::stoi(arg);
            else if(cmd == "HEIGHT")
                _height = std::stoi(arg);
            else if(cmd == "AWAIT")
                commands.push(new TapeCommandAwait(std::stoi(arg)));
            else
                throw TapeException("Unknown command: " + cmd + err_suffix);
        }
    }

    commands.push(new TapeCommand{Kind::end});
}

TapeParser::~TapeParser() {
    while(!commands.empty()) {
        delete commands.front();
        commands.pop();
    }
}

TapeCommand* TapeParser::front() const {
    return commands.front();
}

void TapeParser::pop() {
    delete commands.front();
    commands.pop();
}

bool TapeParser::empty() const {
    return commands.empty();
}

uint16_t TapeParser::width() const {
    return _width;
}
uint16_t TapeParser::height() const {
    return _height;
}
const char* TapeParser::output() const {
    return _output.c_str();
}