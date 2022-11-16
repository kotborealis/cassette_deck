#pragma once

#include <string>
#include <queue>

class TapeException : public std::exception {
public:
    TapeException(const std::string& msg) : msg(msg) {}
    const char* what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};

struct TapeCommand {
    enum class Kind {
        none,
        end,
        output,
        require,
        key,
        sleep,
        await
    };

    const Kind kind;
};

struct TapeCommandKey : public TapeCommand {
    TapeCommandKey(const char key) : TapeCommand{Kind::key}, key(key) {}
    const char key;
};

struct TapeCommandSleep : public TapeCommand {
    TapeCommandSleep(const int ms) : TapeCommand{Kind::sleep}, ms(ms) {}
    const int ms;
};

struct TapeCommandAwait : public TapeCommand {
    TapeCommandAwait(const int ms) : TapeCommand{Kind::await}, ms(ms) {}

    const int ms;
};


class TapeParser {
public:
    TapeParser(const std::string& tape);
    ~TapeParser();

    TapeCommand* front() const;
    void pop();
    bool empty() const;

    uint16_t width() const;
    uint16_t height() const;
    const char* output() const;

private:
    uint16_t _width = 640, _height = 384;
    std::string _output = "output.gif";

    std::queue<TapeCommand*> commands;
};