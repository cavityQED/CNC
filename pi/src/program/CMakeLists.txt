set(LIBRARY_SOURCE_FILES	program.cpp
							codeBlock.cpp)

set(LINKED_LIBRARIES		Qt5::Core
							Qt5::Widgets)

include_directories(${CMAKE_SOURCE_DIR})												

add_library				(programLib		${LIBRARY_SOURCE_FILES}	)
target_link_libraries	(programLib		${LINKED_LIBRARIES}		)
target_compile_options	(programLib		PRIVATE	"-Wno-psabi"	)