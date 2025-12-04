cmake_minimum_required(VERSION 3.18.1)
project("polivsystem")

add_library(polivsystem SHARED
        native-lib.cpp
)

find_library(log-lib log)

target_link_libraries(polivsystem
        ${log-lib}
)