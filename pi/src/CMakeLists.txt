cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 14)
project(cnc)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(libdocs	JogController.cpp
			PositionReadout.cpp
			MotorController.cpp
			MainWindow.h)

add_library(cncLib ${libdocs})
target_link_libraries(cncLib Qt5::Core Qt5::Widgets wiringPi pthread rt)
 
add_executable(cnc testing_main.cpp)
target_link_libraries(cnc cncLib)
