cmake_minimum_required(VERSION 3.10)
project(SnakeGame VERSION 1.0.0)

# ============================================================================
# Project Configuration
# ============================================================================

# Set C++ standard
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

# GLAD (OpenGL loader)
set(GLAD_INCLUDE_DIR ${THIRD_PARTY_DIR}/glad/include)
set(GLAD_SOURCE ${THIRD_PARTY_DIR}/glad/src/glad.c)

# GLM (header-only math library)
set(GLM_INCLUDE_DIR ${THIRD_PARTY_DIR}/glm)

# GLFW (window and input library)
set(GLFW_INCLUDE_DIR ${THIRD_PARTY_DIR}/GLFW/include)
set(GLFW_LIBRARY_DIR ${THIRD_PARTY_DIR}/GLFW/lib)

# ============================================================================
# Platform Detection
# ============================================================================

if(WIN32)
    set(PLATFORM_LIBRARIES opengl32)
    
    # Detect architecture
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "x64")
        set(GLFW_LIB_PATH ${GLFW_LIBRARY_DIR}/glfw3.lib)
    else()
        set(ARCH "x86")
        set(GLFW_LIB_PATH ${GLFW_LIBRARY_DIR}/glfw3.lib)
    endif()
    
    # Check for GLFW DLL
    if(EXISTS ${GLFW_LIBRARY_DIR}/glfw3.dll)
        set(GLFW_DLL ${GLFW_LIBRARY_DIR}/glfw3.dll)
    endif()
    
elseif(APPLE)
    set(PLATFORM_LIBRARIES "-framework OpenGL")
    set(GLFW_LIB_PATH ${GLFW_LIBRARY_DIR}/libglfw.3.dylib)
    
else() # Linux/Unix
    set(PLATFORM_LIBRARIES GL dl pthread)
    set(GLFW_LIB_PATH ${GLFW_LIBRARY_DIR}/libglfw.so.3)
endif()

# ============================================================================
# Include Directories
# ============================================================================

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${GLFW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR}
)

# ============================================================================
# Source Files
# ============================================================================

# Core system files
set(CORE_SOURCES
    src/Core/InputManager.cpp
    src/Core/Renderer.cpp
    src/Core/ResourceManager.cpp
    src/Core/Time.cpp
)

# Game objects
set(GAMEOBJECT_SOURCES
    src/GameObjects/Snake.cpp
)

# Game states
set(GAMESTATE_SOURCES
    src/GameStates/GameState.cpp
)

# Main entry point
set(MAIN_SOURCES
    src/main.cpp
)

# Combine all sources
set(SOURCES
    ${CORE_SOURCES}
    ${GAMEOBJECT_SOURCES}
    ${GAMESTATE_SOURCES}
    ${MAIN_SOURCES}
    ${GLAD_SOURCE}
)

# ============================================================================
# Create Executable
# ============================================================================

add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================================================
# Link Libraries
# ============================================================================

target_link_libraries(${PROJECT_NAME}
    ${GLFW_LIB_PATH}
    ${PLATFORM_LIBRARIES}
)

# ============================================================================
# Compiler Options
# ============================================================================

# Enable warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX-)
    # Disable specific warnings
    target_compile_options(${PROJECT_NAME} PRIVATE /wd4251 /wd4275 /wd4996)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ============================================================================
# Resource Files
# ============================================================================

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Copy GLFW DLL on Windows
if(WIN32 AND GLFW_DLL)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GLFW_DLL}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying GLFW DLL to output directory"
    )
endif()

# ============================================================================
# Platform-Specific Settings
# ============================================================================

if(APPLE)
    # macOS specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE GL_SILENCE_DEPRECATION=1)
endif()

if(WIN32)
    # Windows specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# Install Rules (Optional)
# ============================================================================

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(DIRECTORY shaders/ DESTINATION shaders)

# ============================================================================
# Custom Targets
# ============================================================================

# Add a custom target to run the game
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Running ${PROJECT_NAME}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Add a custom target for development
add_custom_target(dev
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${PROJECT_NAME}
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Building and running ${PROJECT_NAME}"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# ============================================================================
# Print Configuration Summary
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Snake Game Configuration")
message(STATUS "========================================")
message(STATUS "Project Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Third Party Path: ${THIRD_PARTY_DIR}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")