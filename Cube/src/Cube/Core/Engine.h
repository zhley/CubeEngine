#pragma once

namespace Cube {

	class Application;

	// manage all global state
	class Engine final {
	public:
		static void init();

		// app 必须是堆区对象, Engine 会接管其生命周期.
		static void setApp(Application* app);
		static Application* getApp();

		// 析构 application, 需在程序结束时调用
		static void shutdown();

	private:
		static Application* application;
	};

}