cmake_minimum_required(VERSION 3.10)

project(Game VERSION 0.0.1 DESCRIPTION "Game testing." LANGUAGES CXX)

set(WARN_FLAGS "-Wall;-Wextra;-Wpedantic" CACHE STRING "Compiler warning flags" FORCE)

find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(game WIN32 MACOSX_BUNDLE src/linux_main.cpp)

target_include_directories(game PRIVATE src)
target_link_libraries(game PRIVATE X11::X11 GLX OpenGL::GL glm::glm-header-only)

target_compile_features(game PRIVATE cxx_std_23)
target_compile_options(game PRIVATE ${WARN_FLAGS})
