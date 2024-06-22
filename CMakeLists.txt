cmake_minimum_required(VERSION 3.12)

project(gbmu VERSION 1.0 LANGUAGES CXX)

set(SOURCES
	src/main.cpp
	src/GBMUMenu/GBMUMenu.cpp
	src/GameBoy/GameBoy.cpp
	src/GameBoy/MMU/MMU.cpp
	src/GameBoy/CPU/CPU.cpp
	src/GameBoy/PPU/PPU.cpp
	src/GameBoy/APU/APU.cpp
)

find_package(SDL2 REQUIRED)

include_directories(gbmu ${SDL2_INCLUDE_DIRS})
include_directories(gbmu PRIVATE src/)

add_executable(gbmu ${SOURCES})
target_link_libraries(gbmu ${SDL2_LIBRARIES})
