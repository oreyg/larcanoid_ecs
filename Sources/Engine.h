#pragma once
#include "Config.h"
#include "Actor.h"

#include <stdint.h>
#include <type_traits>
#include <algorithm>
#include <memory>
#include <vector>

#include <entt/entt.hpp>

class Engine final
{
private:
	// Time counters
	uint64_t m_start_tick = 0;
	uint64_t m_prev_tick  = 0;
	uint64_t m_delta_tick = 0;

	// Last processed frame number
	uint64_t m_fixed_last_frame = 0;

	bool m_should_quit = false;

	struct SDL_Window* m_sdl_window = nullptr;
	struct SDL_Renderer* m_sdl_renderer = nullptr;

	std::vector<std::shared_ptr<Actor>> m_actors;

	// Store scancodes from the last frame
	static constexpr size_t c_kb_size = 512;
	uint8_t m_kb_state[c_kb_size]{};

public:
	entt::registry registry;

	// Game lifetime
	void process();
	bool is_quit_requested() const;
	void request_quit();

	void update(float delta_time);
	void fixed_update();
	void render();

	void process_inputs();
	void call_on_input(EInputEvent e, bool changed);
	
	// SDL logic
	void process_os_events();

	template<class t, class ... params>
	inline std::shared_ptr<t> create_actor(params ... args)
	{
		auto m = std::make_shared<t>(args...);
		m_actors.push_back(m);
		m->on_construct(m_sdl_renderer, &registry);
		return m;
	}

	inline bool release_actor(std::shared_ptr<Actor> Module)
	{
		auto it = std::find(std::begin(m_actors), std::end(m_actors), Module);
		if (it != std::end(m_actors))
		{
			m_actors.erase(it);
			return true;
		}
		return false;
	}

	Engine();
	~Engine();
	Engine(Engine&) = delete;
};

	