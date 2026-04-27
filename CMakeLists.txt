cmake_minimum_required(VERSION 3.10)
project(SnakeGame VERSION 1.0.0)

# ============================================================================
# 1. CORE SETTINGS
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put the final compiled game (.exe) into a "bin" folder so it's easy to find
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================================================
# 2. THIRD-PARTY LIBRARIES
# ============================================================================
set(TP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# --- FreeType (Text Rendering) ---
# Since FreeType has its own CMakeLists.txt inside its folder, we can just 
# tell CMake to build it alongside our game from scratch!
add_subdirectory(${TP_DIR}/freetype)

# --- SoLoud (Audio) ---
# Gather the specific SoLoud source files we need, and compile them into a static library
file(GLOB_RECURSE SOLOUD_SRC 
    ${TP_DIR}/soloud/src/core/*.cpp 
    ${TP_DIR}/soloud/src/audiosource/wav/*.c* ${TP_DIR}/soloud/src/backend/miniaudio/*.cpp
)
add_library(soloud STATIC ${SOLOUD_SRC})
target_include_directories(soloud PUBLIC ${TP_DIR}/soloud/include ${TP_DIR}/soloud)
target_compile_definitions(soloud PUBLIC WITH_MINIAUDIO)

# --- GLFW (Window & Input) ---
# We will point directly to your pre-compiled Visual Studio 2022 binaries.
set(GLFW_DIR ${TP_DIR}/GLFW/glfw-3.4.bin.WIN64)
if(WIN32)
    set(GLFW_LIB ${GLFW_DIR}/lib-vc2022/glfw3dll.lib)
    set(PLATFORM_LIBS opengl32)
else() # Fallbacks just in case you ever compile this on Linux
    set(GLFW_LIB ${GLFW_DIR}/lib/libglfw.so.3)
    set(PLATFORM_LIBS GL dl pthread)
endif()

# ============================================================================
# 3. OUR GAME BUILD
# ============================================================================
# 'GLOB_RECURSE' automatically finds every single .cpp and .h file in your src folder.
# This means you NEVER have to manually type out new files here when you make them!
file(GLOB_RECURSE GAME_SOURCES src/*.cpp src/*.h)

# Build the actual executable using our vacuumed game files and GLAD
add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${TP_DIR}/glad/src/glad.c)

# Tell the compiler exactly where to look when you type #include "something.h"
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${TP_DIR}/glad/include
    ${TP_DIR}/glm
    ${GLFW_DIR}/include
    ${TP_DIR}/stb_image
    ${TP_DIR}/freetype/include
)

# Glue the third-party libraries we configured in Section 2 into our final executable
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${GLFW_LIB} 
    ${PLATFORM_LIBS} 
    soloud 
    freetype
)

# ============================================================================
# 4. POST-BUILD / QUALITY OF LIFE
# ============================================================================
# After the game compiles, automatically copy our images, shaders, and DLLs into the "bin" folder.
# If we don't do this, the .exe won't be able to find the files and will crash instantly!
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GLFW_DIR}/lib-vc2022/glfw3.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/
)