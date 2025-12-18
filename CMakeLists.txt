cmake_minimum_required(VERSION 3.10)

project(Kermit VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

# Project description
set(PROJECT_DESCRIPTION "Kermit - A hidden service router")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(kermit ${SOURCES})

# Link libraries (we'll add more as needed)
find_package(OpenSSL REQUIRED)
target_link_libraries(kermit PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Install targets
install(TARGETS kermit DESTINATION bin)

# Enable testing
enable_testing()

# Add compile options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(kermit PRIVATE -Wall -Wextra -Wpedantic -O2)
endif()