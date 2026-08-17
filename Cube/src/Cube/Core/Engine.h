#pragma once

#include <memory>

namespace Cube {

	class Application;

	// manage all global state
	class Engine final {
	public:
		static void init();

		// app 必须是堆区对象, Engine 会接管其生命周期.
		static void setApp(Application* app);
		static Application* getApp();

		// 析构 application, 推荐显式调用以控制析构顺序, 避免日志器在 application 之前析构.
		static void shutdown();

	private:
		static std::unique_ptr<Application> application;
	};

}