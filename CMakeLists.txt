project(KigsTests)

IF(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/projects)
	message(STATUS "CMake enter test projects")
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/projects projects)
endif()
