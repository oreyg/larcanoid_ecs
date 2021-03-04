#include "Engine.h"
#include "FMath.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

void Engine::process()
{
	process_os_events();

	// This part of code could be moved to another thread in real-time OS.
	// (For example on IOS your app could be killed if it fails to respond in short time)
	{
		const uint64_t perf_freq = SDL_GetPerformanceFrequency();
		const uint64_t perf_tick = SDL_GetPerformanceCounter();
		const uint64_t delta_tick = perf_tick - m_prev_tick;
		const float    delta_time = delta_tick / (float)perf_freq;

		constexpr float e = 1e-15F;
		if (delta_time > e)
		{
			update(delta_time);
		}

		const uint64_t fixed_tick_duration = (uint64_t)(g_fixed_delta_time * perf_freq);
		const uint64_t target_frame = (perf_tick - m_start_tick) / fixed_tick_duration;
		while (m_fixed_last_frame < target_frame)
		{
			++m_fixed_last_frame;
			fixed_update();
		}

		m_prev_tick = perf_tick;
	}
}

bool Engine::is_quit_requested() const
{
    return m_should_quit;
}

void Engine::request_quit()
{
    m_should_quit = true;
}

void Engine::update(float delta_time)
{
	for (auto& m : m_actors)
	{
		m->on_update(delta_time);
	}

	// Rendering could be done in separate thread
	// In our case this doesn't matter
	render();
}

void Engine::fixed_update()
{
	for (auto& m : m_actors)
	{
		m->on_fixed_update();
	}

	// Inputs could be processed in regular update aswell, 
	// just pass in delta_time
	// In our case this doesn't matter
	process_inputs();
}

void Engine::render()
{
	SDL_RenderClear(m_sdl_renderer);

	for (auto& m : m_actors)
	{
		m->on_render(m_sdl_renderer);
	}

	SDL_SetRenderDrawColor(m_sdl_renderer, 0, 0, 0, 255);
	SDL_RenderPresent(m_sdl_renderer);
}

void Engine::process_inputs()
{
	int length = 0;
	const Uint8* kb_state = SDL_GetKeyboardState(&length);
	if (kb_state[SDL_SCANCODE_LEFT])
	{
		call_on_input(EInputEvent::left, m_kb_state[SDL_SCANCODE_LEFT] != kb_state[SDL_SCANCODE_LEFT]);
	}
	if (kb_state[SDL_SCANCODE_RIGHT])
	{
		call_on_input(EInputEvent::right, m_kb_state[SDL_SCANCODE_RIGHT] != kb_state[SDL_SCANCODE_RIGHT]);
	}
	if (kb_state[SDL_SCANCODE_SPACE])
	{
		call_on_input(EInputEvent::space, m_kb_state[SDL_SCANCODE_SPACE] != kb_state[SDL_SCANCODE_SPACE]);
	}
	if (kb_state[SDL_SCANCODE_ESCAPE])
	{
		call_on_input(EInputEvent::escape, m_kb_state[SDL_SCANCODE_ESCAPE] != kb_state[SDL_SCANCODE_ESCAPE]);
	}
	memcpy(m_kb_state, kb_state, fmath::min(length, (int)c_kb_size));
}

void Engine::call_on_input(EInputEvent e, bool changed)
{
	for (auto& m : m_actors)
	{
		m->on_input(e, changed);
	}
}

void Engine::process_os_events()
{
	SDL_Event e;
	if (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			m_should_quit = true;
			break;
		}
	}
}

Engine::Engine()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		m_should_quit = true;
		return;
	}

	if (TTF_Init() != 0)
	{
		m_should_quit = true;
		return;
	}
	
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		m_should_quit = true;
		return;
	}

	const int mix_flags = 0;
	if ((Mix_Init(mix_flags) & mix_flags) != mix_flags)
	{
		m_should_quit = true;
		return;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
	{
		m_should_quit = true;
		return;
	}

	m_start_tick = SDL_GetPerformanceCounter();
	m_prev_tick  = m_start_tick;

	m_sdl_window   = SDL_CreateWindow("Arcanoid", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, (int)g_screen_area_s.x, (int)g_screen_area_s.y, SDL_WINDOW_SHOWN );
	m_sdl_renderer = SDL_CreateRenderer(m_sdl_window, -1, 0);
}

Engine::~Engine()
{
	Mix_Quit();
	TTF_Quit();

	// If I had a render thread, here I would've waited for it to finish
	SDL_DestroyRenderer(m_sdl_renderer);
	SDL_DestroyWindow(m_sdl_window);
	SDL_Quit();
}