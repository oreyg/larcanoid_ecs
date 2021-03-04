#pragma once
#include <entt/entt.hpp>

struct SDL_Renderer;

enum class EInputEvent
{
	left,
	right,
	space,
	escape
};

class Actor
{
public:
	virtual void on_construct(SDL_Renderer* renderer, entt::registry* registry) {};
	virtual void on_update(float delta_time) {};
	virtual void on_fixed_update() {};
	virtual void on_render(SDL_Renderer* renderer) {};
	virtual void on_input(EInputEvent evt, bool changed) {};
};
