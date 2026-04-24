cmake_minimum_required(VERSION 3.14)
project(TicTacToeSoloAIPlugin LANGUAGES CXX)

if(DEFINED ENV{LOGOS_MODULE_BUILDER_ROOT})
    include($ENV{LOGOS_MODULE_BUILDER_ROOT}/cmake/LogosModule.cmake)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/LogosModule.cmake")
    include(cmake/LogosModule.cmake)
else()
    message(FATAL_ERROR "LogosModule.cmake not found")
endif()

logos_module(
    NAME tictactoe_solo_ai
    SOURCES
        src/tictactoe_interface.h
        src/tictactoe_plugin.h
        src/tictactoe_plugin.cpp
    EXTERNAL_LIBS
        tictactoe
)
