#include "Timer.h"

void Scheduler::schedule(float in_seconds, std::function<void()>&& fun)
{
	add({ m_accum + in_seconds, fun });
}

void Scheduler::add(TimedPrecedure&& function)
{
	auto upper_bound = std::upper_bound(m_funcs.begin(), m_funcs.end(), function, 
		[](const TimedPrecedure& first, const TimedPrecedure& second) 
		{ return first.when < second.when; }
	);
	m_funcs.insert(upper_bound, function);
}

void Scheduler::pause(bool paused)
{
	m_paused = paused;
}

void Scheduler::reset()
{
	m_funcs.clear();
}

void Scheduler::on_construct(SDL_Renderer* renderer, entt::registry* registry)
{
}

void Scheduler::on_update(float delta_time)
{
	if (m_paused)
	{
		return;
	}

	m_accum += delta_time;
	while (m_funcs.size() > 0 && m_accum > m_funcs[0].when)
	{
		m_funcs[0].fun();
		m_funcs.erase(m_funcs.begin());
	}
}

void Scheduler::on_fixed_update()
{
}

void Scheduler::on_render(SDL_Renderer* renderer)
{
}

void Scheduler::on_input(EInputEvent evt, bool changed)
{
}

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
}
