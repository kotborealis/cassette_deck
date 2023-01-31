#include <thread>
#include <iostream>
#include <chrono>
#include <queue>

#include "tape_parser.hpp"
#include "tape_player.h"

#include<sys/time.h>

long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}


#ifdef __cplusplus
extern "C" {
#endif

tape_player_t* tape_player_new(char* src) {
    TapeParser* tape = nullptr;

    try {
        tape = new TapeParser(src);
    }
    catch(TapeException ex) {
        printf("Error while parsing tape: %s\n", ex.what());
        return nullptr;
    }

    tape_player_t* player = (tape_player_t*)malloc(sizeof(tape_player_t));
    player->last_frame_time = 0;
    player->tape = tape;
    player->awaiting = 0;

    return player;
}

void tape_player_free(tape_player_t* player) {
    delete (TapeParser*)player->tape;
    free(player);
}

int tape_player_frame(tape_player_t* player, ssize_t (*write)(int, const void*, size_t), int fd) {
    auto tape = (TapeParser*)player->tape;

    if(tape->empty()) return 1;

    const auto cmd = tape->front();

    auto now = timeInMilliseconds();
    if(player->last_frame_time == 0) {
        player->last_frame_time = now;
    }

    switch(cmd->kind) {
        case TapeCommand::Kind::key:
            write(fd, &((TapeCommandKey*)cmd)->key, 1);
            tape->pop();
            break;
        case TapeCommand::Kind::sleep: {
            auto i = ((TapeCommandSleep*)cmd)->ms;

            if(now - player->last_frame_time <= i)
                return 0;

            tape->pop();
            break;
        }
        case TapeCommand::Kind::await: {
            if(player->awaiting == 0) {
                player->awaiting = 1;
            }
            return 0;
        }
        case TapeCommand::Kind::end:
            tape->pop();
            return 1;
            break;
    }

    player->last_frame_time = now;

    if(!tape->empty() && tape->front()->kind == TapeCommand::Kind::await) {
        return tape_player_frame(player, write, fd);
    }

    return 0;
}

uint16_t tape_player_width(tape_player_t* player) {
    return ((TapeParser*)player->tape)->width();
}

uint16_t tape_player_height(tape_player_t* player) {
    return ((TapeParser*)player->tape)->height();
}

const char* tape_player_output(tape_player_t* player) {
    return ((TapeParser*)player->tape)->output();
}

const char* tape_player_theme(tape_player_t* player) {
    return ((TapeParser*)player->tape)->theme();
}

void tape_player_awaited(tape_player_t* player) {
    if(player->awaiting == 1) {
        player->awaiting = 0;
        if(((TapeParser*)player->tape)->front()->kind == TapeCommand::Kind::await) {
            ((TapeParser*)player->tape)->pop();
        }
    }
}

#ifdef __cplusplus
}
#endif

