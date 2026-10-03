add_executable(CubeEditor)

target_sources(CubeEditor PRIVATE
    src/main.cpp
    src/App/EditorApp.cpp
    src/App/EditorPage.cpp
    src/App/GuidancePage.cpp
    src/Project/AssetExplorer.cpp
    src/Project/Project.cpp
    src/Utils/FileDialog.cpp
    src/Utils/ImGuiExternal.cpp
    src/Utils/misc.cpp
    src/Views/AnimationEditor.cpp
    src/Views/AssetInspector.cpp
    src/Views/EntityPropertyPanel.cpp
    src/Views/HierarchyView.cpp
    src/Views/LogView.cpp
    src/Views/ResourcePickerDialog.cpp
    src/Views/ResourcesPanel.cpp
    src/Views/SceneView.cpp
    src/Views/ThumbnailManager.cpp
)

target_include_directories(CubeEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(CubeEditor PRIVATE
    Cube
    imgui
)

add_custom_command(TARGET CubeEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:CubeEditor>/assets
    COMMENT "Copying assets to output directory..."
)
