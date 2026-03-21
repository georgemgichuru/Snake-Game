cmake_minimum_required(VERSION 3.10)
project(SnakeGame VERSION 1.0.0)

# ============================================================================
# Project Configuration
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Dependencies Paths
# ============================================================================

# Third-party directory
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# GLAD
set(GLAD_INCLUDE_DIR ${THIRD_PARTY_DIR}/glad/include)
set(GLAD_SOURCE ${THIRD_PARTY_DIR}/glad/src/glad.c)

# GLM (header-only)
set(GLM_INCLUDE_DIR ${THIRD_PARTY_DIR}/glm)

# GLFW - Navigate into the versioned folder
# Since you have GLFW/glfw-3.4.bin.WIN64/
set(GLFW_BASE_DIR ${THIRD_PARTY_DIR}/GLFW)
set(GLFW_VERSION_DIR ${GLFW_BASE_DIR}/glfw-3.4.bin.WIN64)

# Check if the versioned directory exists
if(EXISTS ${GLFW_VERSION_DIR})
    message(STATUS "Found GLFW at: ${GLFW_VERSION_DIR}")
    
    # Set include directory
    set(GLFW_INCLUDE_DIR ${GLFW_VERSION_DIR}/include)
    
    # For Visual Studio 2022, use lib-vc2022
    set(GLFW_LIBRARY_DIR ${GLFW_VERSION_DIR}/lib-vc2022)
    
    # If lib-vc2022 doesn't exist, try other options
    if(NOT EXISTS ${GLFW_LIBRARY_DIR})
        if(EXISTS ${GLFW_VERSION_DIR}/lib-vc2019)
            set(GLFW_LIBRARY_DIR ${GLFW_VERSION_DIR}/lib-vc2019)
            message(STATUS "Using lib-vc2019 instead")
        elseif(EXISTS ${GLFW_VERSION_DIR}/lib-vc2017)
            set(GLFW_LIBRARY_DIR ${GLFW_VERSION_DIR}/lib-vc2017)
            message(STATUS "Using lib-vc2017 instead")
        elseif(EXISTS ${GLFW_VERSION_DIR}/lib-static-ucrt)
            set(GLFW_LIBRARY_DIR ${GLFW_VERSION_DIR}/lib-static-ucrt)
            message(STATUS "Using lib-static-ucrt instead")
        else()
            message(FATAL_ERROR "No GLFW library directory found. Checked: lib-vc2022, lib-vc2019, lib-vc2017, lib-static-ucrt")
        endif()
    endif()
    
    message(STATUS "GLFW Include: ${GLFW_INCLUDE_DIR}")
    message(STATUS "GLFW Library: ${GLFW_LIBRARY_DIR}")
    
else()
    message(FATAL_ERROR "Cannot find GLFW at: ${GLFW_VERSION_DIR}")
endif()

# ============================================================================
# Platform-Specific GLFW Library
# ============================================================================

if(WIN32)
    set(PLATFORM_LIBRARIES opengl32)
    
    # Check for GLFW library file
    if(EXISTS ${GLFW_LIBRARY_DIR}/glfw3.lib)
        set(GLFW_LIBRARY ${GLFW_LIBRARY_DIR}/glfw3.lib)
        message(STATUS "Using static GLFW library")
    elseif(EXISTS ${GLFW_LIBRARY_DIR}/glfw3dll.lib)
        set(GLFW_LIBRARY ${GLFW_LIBRARY_DIR}/glfw3dll.lib)
        set(GLFW_DLL ${GLFW_LIBRARY_DIR}/glfw3.dll)
        message(STATUS "Using dynamic GLFW library")
    else()
        message(FATAL_ERROR "Cannot find GLFW library file in ${GLFW_LIBRARY_DIR}")
    endif()
    
elseif(APPLE)
    set(PLATFORM_LIBRARIES "-framework OpenGL")
    set(GLFW_LIBRARY ${GLFW_LIBRARY_DIR}/libglfw.3.dylib)
    
else() # Linux/Unix
    set(PLATFORM_LIBRARIES GL dl pthread)
    set(GLFW_LIBRARY ${GLFW_LIBRARY_DIR}/libglfw.so.3)
endif()

# ============================================================================
# Include Directories
# ============================================================================

# stb_image (header-only — just needs the include directory)
set(STB_IMAGE_INCLUDE_DIR ${THIRD_PARTY_DIR}/stb_image)
message(STATUS "stb_image Include: ${STB_IMAGE_INCLUDE_DIR}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
    ${THIRD_PARTY_DIR}/soloud
    ${STB_IMAGE_INCLUDE_DIR}   # <-- add this line
)

# ============================================================================
# Source Files
# ============================================================================

file(GLOB_RECURSE SOLOUD_SOURCES
    ${THIRD_PARTY_DIR}/soloud/src/core/*.cpp
    ${THIRD_PARTY_DIR}/soloud/src/audiosource/wav/*.cpp
    ${THIRD_PARTY_DIR}/soloud/src/audiosource/wav/*.c
    ${THIRD_PARTY_DIR}/soloud/src/backend/miniaudio/*.cpp
)

add_library(soloud STATIC ${SOLOUD_SOURCES})

target_include_directories(soloud PUBLIC
    ${THIRD_PARTY_DIR}/soloud
)

target_compile_definitions(soloud PUBLIC WITH_MINIAUDIO)

set(SOURCES
    src/main.cpp
    src/Core/InputManager.cpp
    src/Core/InputManager.h
    src/Core/Renderer.cpp
    src/Core/Renderer.h
    src/Core/ResourceManager.cpp
    src/Core/ResourceManager.h
    src/Core/Time.cpp
    src/Core/Time.h
    src/Core/ParticleSystem.cpp
    src/Core/ParticleSystem.h
    src/GameObjects/Snake.cpp
    src/GameObjects/Snake.h
    src/GameStates/GameState.cpp
    src/GameStates/GameState.h
    ${GLAD_SOURCE}
)

# ============================================================================
# Create Executable
# ============================================================================

add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================================================
# Link Libraries
# ============================================================================

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW_LIBRARY}
    ${PLATFORM_LIBRARIES}
    soloud
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${THIRD_PARTY_DIR}/soloud/include
)

# ============================================================================
# Compiler Options
# ============================================================================

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
    # Disable some warnings
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4251 /wd4275 /wd4996)
    # Set Windows subsystem to console for debugging
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Copy Resources
# ============================================================================

# Copy shaders to output directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
        COMMENT "Copying shaders to output directory"
    )
endif()

# Copy GLFW DLL on Windows (if using dynamic library)
if(WIN32 AND GLFW_DLL)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GLFW_DLL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying GLFW DLL to output directory"
    )
endif()

# ============================================================================
# Custom Targets
# ============================================================================

# Add a custom target to run the game
add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Running ${PROJECT_NAME}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Add a custom target for development
add_custom_target(dev
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Building and running ${PROJECT_NAME}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# ============================================================================
# Build Configuration Summary
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Snake Game Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "GLFW Include: ${GLFW_INCLUDE_DIR}")
message(STATUS "GLFW Library: ${GLFW_LIBRARY}")
message(STATUS "GLM Include: ${GLM_INCLUDE_DIR}")
message(STATUS "GLAD Include: ${GLAD_INCLUDE_DIR}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")