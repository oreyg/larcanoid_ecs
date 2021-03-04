#pragma once
#include "Actor.h"
#include <functional>
#include <algorithm>

struct TimedPrecedure
{
	float when;
	std::function<void()> fun;
};

class Scheduler final : public Actor
{
private:
	bool m_paused{ true };
	float m_accum{ 0.0f };
	std::vector<TimedPrecedure> m_funcs;

public:
	void schedule(float in_seconds, std::function<void()>&& fun);
	void add(TimedPrecedure&& function);
	void pause(bool paused);
	void reset();

	virtual void on_update(float delta_time) override;

	Scheduler(bool paused);
	Scheduler();
	virtual ~Scheduler();
	Scheduler(Scheduler&) = delete;
};