include_guard()

add_library(gifenc ./3rd_party/gifenc/gifenc.c)
target_include_directories(gifenc PUBLIC ./3rd_party/gifenc)