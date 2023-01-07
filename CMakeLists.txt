cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "D:/software/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(zmv LANGUAGES CXX)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

file(GLOB srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)

add_executable(zmv ${srcs})

target_link_libraries(zmv PRIVATE glad::glad)
target_link_libraries(zmv PRIVATE glfw)
target_link_libraries(zmv PRIVATE glm::glm)
target_link_libraries(zmv PRIVATE imgui::imgui)
target_link_libraries(zmv PRIVATE assimp::assimp)

target_include_directories(zmv PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(zmv PUBLIC include)