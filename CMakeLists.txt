cmake_minimum_required(VERSION 3.10)
project(TDKLambdaG30 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for Linux/GCC
add_compile_options(-Wall -Wextra -Wpedantic)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library source files
set(LIBRARY_SOURCES
    src/tdk_lambda_g30.cpp
)

set(LIBRARY_HEADERS
    include/tdk_lambda_g30.h
)

# Create static library
add_library(tdk_lambda_g30_static STATIC ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
set_target_properties(tdk_lambda_g30_static PROPERTIES OUTPUT_NAME tdk_lambda_g30)

# Create shared library (optional)
add_library(tdk_lambda_g30_shared SHARED ${LIBRARY_SOURCES} ${LIBRARY_HEADERS})
set_target_properties(tdk_lambda_g30_shared PROPERTIES OUTPUT_NAME tdk_lambda_g30)

# Linux pthread linking
target_link_libraries(tdk_lambda_g30_static pthread)
target_link_libraries(tdk_lambda_g30_shared pthread)

# Test program
add_executable(test examples/test.cpp)
target_link_libraries(test tdk_lambda_g30_static)

# Comprehensive test program (all functions)
add_executable(comprehensive_test examples/comprehensive_test.cpp)
target_link_libraries(comprehensive_test tdk_lambda_g30_static)

# Installation rules
install(TARGETS tdk_lambda_g30_static tdk_lambda_g30_shared
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${LIBRARY_HEADERS} DESTINATION include)

# Build configuration info
message(STATUS "")
message(STATUS "=== TDK Lambda G30 Build Configuration ===")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  - tdk_lambda_g30_static (static library)")
message(STATUS "  - tdk_lambda_g30_shared (shared library)")
message(STATUS "  - test (test program)")
message(STATUS "  - comprehensive_test (comprehensive test program)")
message(STATUS "==========================================")
message(STATUS "")
