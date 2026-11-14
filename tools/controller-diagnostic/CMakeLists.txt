cmake_minimum_required(VERSION 3.25)
project(horipad_diag C)

add_executable(horipad_diag main.c)
set_target_properties(horipad_diag PROPERTIES C_STANDARD 17 C_EXTENSIONS ON)
target_compile_options(horipad_diag PRIVATE -Wall -Wextra -Werror)
xsdk_add_xex(horipad_diag)
