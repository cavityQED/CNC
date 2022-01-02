set(LIBRARY_SOURCE_FILES	device.cpp
							spiDevice.cpp
							laser.cpp
							stepperMotor.cpp
							stepperGroup.cpp)

set(LINKED_LIBRARIES		Qt5::Core
							Qt5::Widgets)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR})							

add_library				(deviceLib	${LIBRARY_SOURCE_FILES}	)
target_link_libraries	(deviceLib	${LINKED_LIBRARIES}		)
target_compile_options	(deviceLib	PRIVATE	"-Wno-psabi"	)