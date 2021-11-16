cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 14)
project(cnc)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

add_subdirectory(utilities)

set(libdocs	MainWindow.h
			JogController.cpp
			PositionReadout.cpp
			MotorController.cpp
			ConfigureAxes.cpp
			Curve.cpp
			Line.cpp
			GCode.cpp
			SPI.cpp
			Laser.cpp
			Program.cpp
			Action.cpp)

add_library(cncLib ${libdocs})
target_link_libraries(cncLib Qt5::Core Qt5::Widgets wiringPi pthread rt utilities pigpio)
target_compile_options(cncLib PRIVATE "-Wno-psabi")
 
add_executable(cnc testing_main.cpp)
target_link_libraries(cnc cncLib)
target_compile_options(cnc PRIVATE "-Wno-psabi")
