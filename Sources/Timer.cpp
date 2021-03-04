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
		// List could be erased in lambda
		if (m_funcs.size() > 0)
		{
			m_funcs.erase(m_funcs.begin());
		}
	}
}

Scheduler::Scheduler(bool paused) : m_paused(paused)
{
}

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
}
