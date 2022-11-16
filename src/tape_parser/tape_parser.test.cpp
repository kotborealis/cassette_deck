#include <string>
#include <cassert>

#include "tape_parser.hpp"

int main() {
    // invalid command
    {
        std::string data = "INVALID_COMMAND";

        try {
            TapeParser parser(data);
            assert(false);
        } catch (TapeException& e) {
            assert(true);
        }
    }

    // missing arg
    {
        std::string data = "OUTPUT";

        try {
            TapeParser parser(data);
            assert(false);
        } catch (TapeException& e) {
            assert(true);
        }
    }

    // extra arg
    {
        std::string data = "LEFT 1";

        try {
            TapeParser parser(data);
            assert(false);
        } catch (TapeException& e) {
            assert(true);
        }
    }

    // Happy path
    {
        std::string data = R"(
            REQUIRE echo
            OUTPUT test.gif
            SLEEP 1000
            TYPE echo "Hello World"
            ENTER
            TYPE 123
            )";

        TapeParser p(data);
        assert(true);
    }
}