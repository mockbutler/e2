cmake_minimum_required(VERSION 3.10)
project(e2)

set(COMPILE_WARNING_AS_ERROR TRUE)
set(EXPORT_COMPILE_COMMANDS TRUE)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_library(edit2 cursor.cc e2.cc eb.cc line.cc mark.cc movemnt.cc pos.cc search.cc)
target_compile_definitions(edit2 PUBLIC ENABLE_DRIBBLE)
set_property(TARGET edit2 PROPERTY CXX_STANDARD 11)

add_executable(e2 e2.cc)
set_property(TARGET e2 PROPERTY CXX_STANDARD 11)
target_link_libraries(e2 edit2 ${CURSES_LIBRARIES})
