cmake_minimum_required(VERSION 3.10)
project(GalacticDodger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Look for SFML 3 specifically
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(GalacticDodger main.cpp)

# 2. Use the new SFML 3 Target names (Capitalized with namespaces)
target_link_libraries(GalacticDodger SFML::Graphics SFML::Window SFML::System)

# Copy font to build directory
add_custom_command(TARGET GalacticDodger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_SOURCE_DIR}/arial.ttf
    ${CMAKE_BINARY_DIR}/arial.ttf)