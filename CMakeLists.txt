cmake_minimum_required(VERSION 3.28)
project(starter)

set(CMAKE_CXX_STANDARD 23)
add_subdirectory(src)

include(FetchContent)

# GLM - 1.0.1
set(CURRENT_PACKAGE "GLM")
    message(STATUS "${CURRENT_PACKAGE}")

    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.1
    )
    set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
    message(STATUS "${CURRENT_PACKAGE}: Disabled test flags Populated")

    FetchContent_MakeAvailable(glm)
    message(STATUS "${CURRENT_PACKAGE}: Made Available")

    if(NOT glm_POPULATED)
        message(FATAL_ERROR "${CURRENT_PACKAGE}: Failed to add")
    endif ()
unset(CURRENT_PACKAGE)

# GLFW - 3.4
set(CURRENT_PACKAGE "GLFW")
    message(STATUS "${CURRENT_PACKAGE}")

    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    message(STATUS "${CURRENT_PACKAGE}: Disabled test flags Populated")

    FetchContent_MakeAvailable(glfw)
    message(STATUS "${CURRENT_PACKAGE}: Made Available")

    if(NOT glfw_POPULATED)
        message(FATAL_ERROR "${CURRENT_PACKAGE}: Failed to add")
    endif ()
unset(CURRENT_PACKAGE)

# Vulkan
find_package(Vulkan REQUIRED)

add_executable(starter src/headers/
        src/triangle.cpp
        src/headers/triangle.h)
target_sources(starter PRIVATE src/main.cpp)

target_link_libraries(
        starter
        PRIVATE Vulkan::Vulkan
        PRIVATE glm::glm
        PRIVATE glfw
)
