add_executable(CubeEditor)

file(GLOB_RECURSE EDITOR_SRC
    src/*.cpp
    src/*.h
)
target_sources(CubeEditor PRIVATE ${EDITOR_SRC})

target_include_directories(CubeEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CubeEditor PRIVATE
    Cube
    imgui
)

if (MSVC) 
    target_sources(CubeEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cube_editor_utf8.manifest)
endif()

add_custom_command(TARGET CubeEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:CubeEditor>/assets
    COMMENT "Copying assets to output directory..."
)