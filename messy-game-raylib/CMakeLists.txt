cmake_minimum_required(VERSION 3.10)
project(MessyGame VERSION 1.0)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(WITH_PLATFORM_WEB "Build for web platform using Emscripten" OFF)
option(WITH_PLATFORM_DESKTOP "Build for desktop platforms" ON)
option(WITH_PLATFORM_MOBILE "Build for mobile platforms" OFF)

# Find required packages
find_package(raylib 4.0 QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(raylib)
endif()

# Source files
set(SOURCES
    src/main.c
    src/core/game.c
    src/core/config.c
    src/graphics/renderer.c
    src/graphics/camera.c
    src/graphics/textures.c
    src/input/input.c
    src/entities/entity.c
    src/entities/player.c
    src/entities/ball.c
    src/entities/enemy.c
    src/world/world.c
    src/world/room.c
    src/world/tile.c
    src/utils/utils.c
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${raylib_INCLUDE_DIRS}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Platform-specific configurations
if(WITH_PLATFORM_WEB)
    # Web platform (Emscripten) settings
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/platforms/web/shell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    
    # Copy necessary assets
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
elseif(WITH_PLATFORM_DESKTOP)
    # Desktop platform settings
    if(APPLE)
        # macOS specific settings
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
    elseif(WIN32)
        # Windows specific settings
        set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
        target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
    else()
        # Linux specific settings
        target_link_libraries(${PROJECT_NAME} PRIVATE m pthread dl)
    endif()
    
    # Copy necessary assets
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
elseif(WITH_PLATFORM_MOBILE)
    # Mobile platform settings
    if(ANDROID)
        # Android specific settings
        # Additional configuration will be needed for Android
    elseif(APPLE)
        # iOS specific settings
        # Additional configuration will be needed for iOS
    endif()
endif()

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/${PROJECT_NAME}/assets)

# Enable testing
enable_testing()

# Add tests
add_executable(game_tests tests/main_test.c)
target_link_libraries(game_tests PRIVATE raylib)
add_test(NAME GameTests COMMAND game_tests)