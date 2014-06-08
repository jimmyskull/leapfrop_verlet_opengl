# Copyright © 2013 Paulo Roberto Urio <paulourio@gmail.com> 
cmake_minimum_required(VERSION 2.6)
project(TRABALHO_FINAL)
enable_language(C)

# Set a default build type if and only if user did not define one as command
# line options and he did not give custom CFLAGS or CXXFLAGS. Otherwise, flags
# from default build type would overwrite user-defined ones.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_C_FLAGS AND NOT CMAKE_CXX_FLAGS)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_COMPILER g++)
    add_definitions(-DDEBUG_VERBOSITY=WARN)
endif(CMAKE_BUILD_TYPE MATCHES Release)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_COMPILER g++)
    add_definitions(-DDEBUG_VERBOSITY=TESTING)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
Find_Package(SDL REQUIRED)
Find_Package(SDL_image REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} 
					${SDLIMAGE_INCLUDE_DIR})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# set(IGNORED_WARNINGS "-Wno-c++98-compat-pedantic -Wno-gnu -Wno-padded")
set(CMAKE_CXX_FLAGS_DEBUG 
    "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra ${IGNORED_WARNINGS}")
set(CMAKE_CXX_FLAGS_RELEASE 
    "${CMAKE_CXX_FLAGS_RELEASE} -DDEBUG_VERBOSITY=WARN -Wall")

add_executable(simulate main.cc)

set_target_properties(simulate PROPERTIES COMPILE_FLAGS "-std=c++11")

target_link_libraries(simulate m ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} 
								${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
