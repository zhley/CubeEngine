#pragma once

#include <memory>

namespace Cube {

	class Application;

	// manage all global state
	class Engine final {
	public:
		static void init();
		static void setApp(Application* app);
		static Application* getApp();

	private:
		static std::unique_ptr<Application> application;
	};

}