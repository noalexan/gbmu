cmake_minimum_required(VERSION 3.10)

project(gbmu VERSION 1.0 LANGUAGES CXX)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(emulator 
	src/main.cpp 
	src/GameBoy/GameBoy.cpp 
	src/GameBoy/Cartridge/Cartridge.cpp
	src/GameBoy/CPU/CPU.cpp
	src/GameBoy/MMU/MMU.cpp
	src/GameBoy/PPU/PPU.cpp
	src/GameBoy/APU/APU.cpp
	src/GameBoy/Serial/Serial.cpp
	src/GameBoy/Timer/Timer.cpp
)

target_link_libraries(emulator ${SDL2_LIBRARIES})