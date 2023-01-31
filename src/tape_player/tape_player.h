#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void* tape_t;

struct tape_player_t {
    tape_t tape;
    long long last_frame_time;
    int awaiting;
};

typedef struct tape_player_t tape_player_t;

tape_player_t* tape_player_new(char* src);

void tape_player_free(tape_player_t* player);

uint16_t tape_player_width(tape_player_t* player);

uint16_t tape_player_height(tape_player_t* player);

const char* tape_player_output(tape_player_t* player);

const char* tape_player_theme(tape_player_t* player);

int tape_player_frame(tape_player_t* tape, ssize_t (*write)(int, const void*, size_t), int fd);

void tape_player_awaited(tape_player_t* tape);

#ifdef __cplusplus
}
#endif
