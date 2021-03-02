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

	// Inherited via Actor
	virtual void on_construct(SDL_Renderer* renderer, entt::registry* registry) override;
	virtual void on_update(float delta_time) override;
	virtual void on_fixed_update() override;
	virtual void on_render(SDL_Renderer* renderer) override;
	virtual void on_input(EInputEvent evt, bool changed) override;

	Scheduler();
	virtual ~Scheduler();
	Scheduler(Scheduler&) = delete;
};