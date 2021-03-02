#pragma once

#include <entt.hpp>
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
	virtual void on_construct(SDL_Renderer* renderer, entt::registry* registry) = 0;
	virtual void on_update(float delta_time) = 0;
	virtual void on_fixed_update() = 0;
	virtual void on_render(SDL_Renderer* renderer) = 0;
	virtual void on_input(EInputEvent evt, bool changed) = 0;
};
