cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_STANDARD 20)
project(cnc)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
if(Qt5Core_FOUND)
	message("FOUND Qt5")
	message(STATUS "${Qt5Widgets_INCLUDE_DIRS}")
endif()

add_subdirectory(utilities)
add_subdirectory(action)
add_subdirectory(control)
add_subdirectory(device)

include_directories(${CMAKE_SOURCE_DIR})

set(libdocs	MainWindow.h
			JogController.cpp
			PositionReadout.cpp
			Laser.cpp
			Program.cpp)

set(LINKED_LIBRARIES	Qt5::Core
						Qt5::Widgets
						actionLib
						controlLib
						deviceLib)

add_library(cncLib ${libdocs})
target_link_libraries(cncLib Qt5::Core Qt5::Widgets wiringPi pthread rt utilities pigpio ${LINKED_LIBRARIES})
target_compile_options(cncLib PRIVATE "-Wno-psabi")
 
add_executable(cnc testing_main.cpp)
target_link_libraries(cnc cncLib)
target_compile_options(cnc PRIVATE "-Wno-psabi")
