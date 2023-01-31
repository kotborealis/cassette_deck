cmake_minimum_required(VERSION 3.0)

project(cassette_deck)

set(CMAKE_CXX_STANDARD 17)

include(3rd_party/gifenc.cmake)

add_library(tape_parser
    src/tape_parser/tape_parser.cpp
    src/tape_parser/parser_util.cpp
)
target_include_directories(tape_parser PUBLIC ./src/tape_parser)

add_executable(tape_parser_test
    src/tape_parser/tape_parser.test.cpp
)
target_link_libraries(tape_parser_test tape_parser)

add_library(tape_player
    src/tape_player/tape_player.cpp
)
target_link_libraries(tape_player PUBLIC
    pthread
    tape_parser
)

add_executable(mkfont_bdf
    src/tools/mkfont_bdf.c
    src/tools/mkfont_bdf.h
    src/tools/bdf.h
    src/tools/util.h
)

# custom target to build glyph.h
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/glyph.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND $<TARGET_FILE:mkfont_bdf>
            ./fonts/milkjf/milkjf_k16.bdf
            ./fonts/milkjf/milkjf_8x16r.bdf
            ./fonts/milkjf/milkjf_8x16.bdf
            ./fonts/terminus/ter-u16n.bdf
            > ${CMAKE_BINARY_DIR}/glyph.h
    DEPENDS mkfont_bdf
)

add_executable(theme_generator ./tool/theme_generator)

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/themes.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND $<TARGET_FILE:theme_generator>
            ./themes/
            ${CMAKE_BINARY_DIR}/themes.h
    DEPENDS theme_generator
)

set(CASSETE_DECK_SOURCES
    src/x/yaftx.c
    src/x/x.h
    ${CMAKE_BINARY_DIR}/glyph.h
    ${CMAKE_BINARY_DIR}/themes.h
)

add_executable(cassette_deck_preview ${CASSETE_DECK_SOURCES})
find_package(X11 COMPONENTS X11)
target_link_libraries(cassette_deck_preview PRIVATE X11 gifenc tape_player)
target_include_directories(cassette_deck_preview PRIVATE ${CMAKE_BINARY_DIR})

add_executable(cassette_deck_headless ${CASSETE_DECK_SOURCES})
target_compile_definitions(cassette_deck_headless PRIVATE HEADLESS)
target_link_libraries(cassette_deck_headless PRIVATE gifenc tape_player)
target_include_directories(cassette_deck_headless PRIVATE ${CMAKE_BINARY_DIR})

file(GENERATE
    OUTPUT ${CMAKE_BINARY_DIR}/cassette_deck
    INPUT ${CMAKE_SOURCE_DIR}/bin/cassette_deck.in
)