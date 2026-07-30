cmake_minimum_required(VERSION 3.15)
project(Cube VERSION 1.0.0 LANGUAGES CXX)

add_library(Cube STATIC)

file(GLOB_RECURSE CUBE_SRC
    src/*.cpp
    src/*.h
)
target_sources(Cube PRIVATE ${CUBE_SRC})

target_include_directories(Cube PUBLIC
    src
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/nlohmann-json
    ${CMAKE_SOURCE_DIR}/thirdparty/spdlog-1.15.3/include
)

find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
target_link_libraries(Cube PUBLIC
    glad
    stb_image
    glfw
    Freetype::Freetype
)

target_compile_definitions(Cube PRIVATE
    $<$<CONFIG:Debug>:CB_DEBUG>
    $<$<CONFIG:Release>:CB_RELEASE>
)

if(MSVC)
    target_compile_options(Cube PRIVATE /GR-)
else()
    target_compile_options(Cube PRIVATE -fno-rtti)
endif()
