#pragma once

#include "Core.h"
#include "Window.h"
#include "EventStack.h"
#include "Task.h"
#include "iw/events/eventbus.h"
#include "InitOptions.h"
#include "Layers/ImGuiLayer.h"
#include "iw/asset/AssetManager.h"
#include "iw/input/InputManager.h"
#include "iw/entity/Space.h"
#include "iw/graphics/Renderer.h"
#include "iw/util/queue/blocking_queue.h"
#include <vector>
#include <thread>

namespace IW {
inline namespace Engine {
	class Application {
	private:
		bool               m_running;
		IWindow*           m_window;
		EventStack<Layer*> m_layers;

		iw::ref<IW::IDevice> m_device;

		std::thread  m_renderThread;
		Task<void()> m_updateTask;
		ImGuiLayer*  m_imguiLayer;

	protected:
		iw::ref<IW::Space>        Space;
		iw::ref<IW::Renderer>     Renderer;
		iw::ref<IW::AssetManager> Asset;
		iw::ref<IW::InputManager> Input;
		iw::ref<iw::eventbus>     Bus;

	public:
		IWENGINE_API
		Application();

		IWENGINE_API
		virtual ~Application();

		IWENGINE_API
		virtual int Initialize(
			InitOptions& options);

		IWENGINE_API
		virtual void Run();

		IWENGINE_API
		virtual void Destroy();

		IWENGINE_API
		virtual void Update();

		IWENGINE_API
		virtual void FixedUpdate();
		//virtual void Pause();

		IWENGINE_API
		virtual void HandleEvent(
			iw::event& e);

		IWENGINE_API
		inline IWindow& GetWindow() {
			return *m_window;
		}

		template<
			typename L = Layer>
		L* GetLayer(
			const char* name)
		{
			return (L*)m_layers.Get(name);
		}

		template<
			typename L,
			typename... Args>
		L* PushLayer(
			Args&&... args)
		{
			L* layer = new L(std::forward<Args>(args)...);
			layer->SetApplicationVars(Space, Renderer, Asset, Bus);

			m_layers.PushBack(layer);
			return layer;
		}

		template<
			typename L,
			typename... Args>
		L* PushOverlay(
			Args&& ... args)
		{
			L* layer = new L(std::forward<Args>(args)...);
			layer->SetApplicationVars(Space, Renderer, Asset, Bus);

			m_layers.PushFront(layer);
			return layer;
		}

		template<
			typename L>
		void PopLayer(
			L* layer)
		{
			m_layers.Pop(layer);
		}

	};
}
}

IW::Application* CreateApplication(
	IW::InitOptions& options);
