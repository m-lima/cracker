cmake_minimum_required(VERSION 3.2)
project(Cracker CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Current state
if (WIN32)
  message(STATUS "WIN32")
endif ()
if (MSVC)
  message(STATUS "MSVC")
endif ()
if (APPLE)
  message(STATUS "APPLE")
endif ()

message(STATUS "Using C: " ${CMAKE_C_COMPILER_ID})
message(STATUS "Using CXX: " ${CMAKE_CXX_COMPILER_ID})

# Set high warning levels
if (MSVC)
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif ()
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wno-long-long -pedantic")
endif ()

# Make $HOME compatible with windows
STRING(REGEX REPLACE "\\\\" "/" ENV_HOME_DIR "$ENV{HOME}")

# Add custom modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/findcmake")

# Set base directories
set(CPP_SRC_DIR "src/cpp")
set(OPENCL_SRC_DIR "src/opencl")
#set(OPENCL_OUTPUT_DIR "opencl")
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")

# Add manual libs
list(APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")
list(APPEND INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/${OPENCL_SRC_DIR}")

##------------------------------------------------------------------------------
## Dependencies
##

add_subdirectory("${LIB_DIR}/fmt")
list(APPEND INCLUDE_DIRS "${LIB_DIR}/fmt")
list(APPEND LIBRARIES fmt::fmt-header-only)

find_package(Threads)
list(APPEND LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

find_package(OPENCL REQUIRED)
list(APPEND INCLUDE_DIRS ${OpenCL_INCLUDE_DIR})
list(APPEND LIBRARIES ${OpenCL_LIBRARIES})

add_subdirectory("${LIB_DIR}/cl")
list(APPEND INCLUDE_DIRS ${MFL_CL_INCLUDE})
list(APPEND SOURCES ${MFL_CL_SOURCE})
list(APPEND HEADERS ${MFL_CL_HEADERS})

##------------------------------------------------------------------------------
## Sources
##

list(APPEND SOURCES
    ${CPP_SRC_DIR}/main.cpp
    ${CPP_SRC_DIR}/handlers/target_handler.cpp
    ${CPP_SRC_DIR}/handlers/file_handler.cpp
    ${CPP_SRC_DIR}/handlers/rainbow_handler.cpp
    ${CPP_SRC_DIR}/cracker/cpu_cracker.cpp
    ${CPP_SRC_DIR}/cracker/gpu_cracker.cpp
    ${CPP_SRC_DIR}/md5/md5.cpp
    ${CPP_SRC_DIR}/cl/cracker_program.cpp
    )

list(APPEND HEADERS
    ${CPP_SRC_DIR}/hash.hpp
    ${CPP_SRC_DIR}/handlers/target_handler.hpp
    ${CPP_SRC_DIR}/handlers/file_handler.hpp
    ${CPP_SRC_DIR}/handlers/rainbow_handler.hpp
    ${CPP_SRC_DIR}/cracker/cpu_cracker.hpp
    ${CPP_SRC_DIR}/cracker/gpu_cracker.hpp
    ${CPP_SRC_DIR}/md5/md5.hpp
    ${CPP_SRC_DIR}/cl/cracker_program.hpp
    )

list(APPEND SOURCES_CL
    ${OPENCL_SRC_DIR}/opencl_src/cracker.cl
    )

# For CLion
list(APPEND SOURCES ${HEADERS})

##------------------------------------------------------------------------------
## OpenCL
##

# foreach(SOURCE_CL ${SOURCES_CL})
#   message(STATUS "Copying ${SOURCE_CL} to ${OPENCL_OUTPUT_DIR}")
#   configure_file(${SOURCE_CL} "${OPENCL_OUTPUT_DIR}/." COPYONLY)
# endforeach()

##------------------------------------------------------------------------------
## Targets
##

# Create project
add_executable(cracker ${SOURCES})
#add_executable(cracker ${SOURCES} ${HEADERS} ${SOURCES_CL})
set_target_properties(cracker PROPERTIES CXX_LANGUAGE_STANDARD 14 C_STANDARD 11)

# Organize the source
source_group("Headers" FILES ${HEADERS})
source_group("OpenCL" FILES ${SOURCES_CL})

# Link with FIND_PACKAGE
target_link_libraries(cracker PRIVATE ${LIBRARIES})

# Add manual includes
target_include_directories(cracker PRIVATE ${INCLUDE_DIRS})

