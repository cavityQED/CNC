set(LIBRARY_SOURCE_FILES	controlPanel.cpp
							knob.h)

set(LINKED_LIBRARIES		Qt5::Core
							Qt5::Widgets)

include_directories(${CMAKE_SOURCE_DIR})

add_library				(controlLib	${LIBRARY_SOURCE_FILES}	)
target_link_libraries	(controlLib	${LINKED_LIBRARIES}		)
target_compile_options	(controlLib	PRIVATE	"-Wno-psabi"	)