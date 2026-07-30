#pragma once

#include "Window.h"
#include "Cube/Event/Event.h"
#include "Cube/Scene/RenderServer.h"
#include "Cube/Scene/SceneManager.h"

namespace Cube {

    class Application {
	public:
		Application();
		Application(const WindowPros& windowPros);
		virtual ~Application();

		virtual void run();

		void stop() {
		    running = false;
		}
		Window* getWindow() {
		    return mainWindow;
		}
		RenderServer& getRenderServer() {
		    return renderServer;
		}
        SceneManager& getSceneManager() {
		    return sceneManager;
		}
		EventDispatcher& getEventDispatcher() {
		    return eventDispatcher;
		}

		bool onWindowClose(const Event& e);

	protected:
		Window* mainWindow;
		bool running;
		RenderServer renderServer;
        SceneManager sceneManager;
		EventDispatcher eventDispatcher;
	};
}