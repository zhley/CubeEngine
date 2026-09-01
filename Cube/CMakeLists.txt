add_library(Cube STATIC)

target_sources(Cube PRIVATE 
    src/Cube/Animation/Animation.cpp
    src/Cube/Animation/AnimationClip.cpp
    src/Cube/Core/Application.cpp
    src/Cube/Core/Engine.cpp
    src/Cube/Core/Input.cpp
    src/Cube/Core/Log.cpp
    src/Cube/Core/Path.cpp
    src/Cube/Core/Timer.cpp
    src/Cube/Core/TypeRegister.cpp
    src/Cube/Core/Window.cpp
    src/Cube/Event/Event.cpp
    src/Cube/Renderer/Buffer.cpp
    src/Cube/Renderer/BufferLayout.cpp
    src/Cube/Renderer/Camera.cpp
    src/Cube/Renderer/Context.cpp
    src/Cube/Renderer/Font.cpp
    src/Cube/Renderer/FrameBuffer.cpp
    src/Cube/Renderer/Renderer.cpp
    src/Cube/Renderer/Shader.cpp
    src/Cube/Renderer/Texture.cpp
    src/Cube/Renderer/VertexArray.cpp
    src/Cube/Resource/Atlas.cpp
    src/Cube/Resource/ResourceManager.cpp
    src/Cube/Resource/Script.cpp
    src/Cube/Resource/Sprite.cpp
    src/Cube/Scene/Camera2D.cpp
    src/Cube/Scene/Entity.cpp
    src/Cube/Scene/RenderServer.cpp
    src/Cube/Scene/Scene.cpp
    src/Cube/Scene/SceneManager.cpp
    src/Cube/Scene/SpriteRender.cpp
    src/Cube/Scene/Transform.cpp
    src/Cube/Script/ScriptBindings.cpp
    src/Cube/Script/ScriptComponent.cpp
    src/Cube/Script/ScriptRuntime.cpp
    src/Cube/UI/UILayer.cpp
    src/Cube/UI/UIWidget.cpp
    src/Cube/Utils/Utils.cpp
)

target_include_directories(Cube PUBLIC
    src
    ${CMAKE_SOURCE_DIR}/thirdparty/glm
    ${CMAKE_SOURCE_DIR}/thirdparty/nlohmann-json
    ${CMAKE_SOURCE_DIR}/thirdparty/spdlog-1.15.3/include
    D:/GithubRepo/Zeta/build/include # temp
)

find_package(glfw3 REQUIRED)
find_package(Freetype REQUIRED)
target_link_libraries(Cube PUBLIC
    glad
    stb_image
    glfw
    Freetype::Freetype
    D:/GithubRepo/Zeta/build/lib/zeta_core.lib # temp
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

# game
add_executable(CubeGame)
target_sources(CubeGame PRIVATE src/Game/main.cpp)
if (MSVC)
    target_sources(CubeGame PRIVATE src/Game/cube_game_utf8.manifest)
endif()
    
target_link_libraries(CubeGame PRIVATE Cube)
