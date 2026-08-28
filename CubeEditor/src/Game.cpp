#include "Game.h"

#include "Cube/Core/Application.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Scene.h"
#include "Cube/Core/Engine.h"

#include "App/EditorApp.h"
#include "Project/Project.h"

using namespace Cube;

// TODO: 这里直接预览最终效果，改成独立进程
void gameThreadFunction(EditorApp* app, Project* project, Scene* scene, bool* isGameOver) {

}