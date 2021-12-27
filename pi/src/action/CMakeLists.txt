set(LIBRARY_SOURCE_FILES	action.cpp
							stepperAction.cpp
							laserAction.cpp)

set(LINKED_LIBRARIES		Qt5::Core
							Qt5::Widgets)

include_directories(${CMAKE_SOURCE_DIR})												

add_library				(actionLib	${LIBRARY_SOURCE_FILES}	)
target_link_libraries	(actionLib	${LINKED_LIBRARIES}		)
target_compile_options	(actionLib	PRIVATE	"-Wno-psabi"	)