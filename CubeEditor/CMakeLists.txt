add_executable(CubeEditor)

target_sources(CubeEditor PRIVATE
    src/main.cpp
    src/App/EditorApp.cpp
    src/App/EditorApp.h
    src/App/EditorPage.cpp
    src/App/EditorPage.h
    src/App/GuidancePage.cpp
    src/App/GuidancePage.h
    src/App/Page.h
    src/Project/AssetExplorer.cpp
    src/Project/AssetExplorer.h
    src/Project/Project.cpp
    src/Project/Project.h
    src/Scene/EditorCamera.h
    src/Utils/EditorTextureCache.h
    src/Utils/FileDialog.cpp
    src/Utils/FileDialog.h
    src/Utils/ImGuiExternal.cpp
    src/Utils/ImGuiExternal.h
    src/Utils/misc.cpp
    src/Utils/misc.h
    src/Views/AnimationEditor.cpp
    src/Views/AnimationEditor.h
    src/Views/AssetInspector.cpp
    src/Views/AssetInspector.h
    src/Views/EntityPropertyPanel.cpp
    src/Views/EntityPropertyPanel.h
    src/Views/HierarchyView.cpp
    src/Views/HierarchyView.h
    src/Views/LogView.cpp
    src/Views/LogView.h
    src/Views/ResourcePickerDialog.cpp
    src/Views/ResourcePickerDialog.h
    src/Views/ResourcesPanel.cpp
    src/Views/ResourcesPanel.h
    src/Views/SceneView.cpp
    src/Views/SceneView.h
    src/Views/ThumbnailManager.cpp
    src/Views/ThumbnailManager.h
    src/Views/View.h
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
