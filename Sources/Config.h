#pragma once
#include "FMath.h"
#include <stdint.h>

constexpr float g_scale{ 1.5f };
constexpr Vector2 g_screen_area { 400, 500 };
constexpr Vector2 g_game_area   { 350, 400 };

// Center game are on the screen
constexpr Vector2 g_game_center { g_screen_area / 2 };

// Apply static scaling
constexpr Vector2 g_screen_area_s{ g_screen_area * g_scale };
constexpr Vector2 g_game_area_s{ g_game_area * g_scale };
constexpr Vector2 g_game_center_s{ g_game_center * g_scale };

constexpr float   g_ball_start_velocity{ 220.0f * g_scale };
constexpr float   g_ball_radius{ 6.0f * g_scale };

constexpr float   g_platform_velocity{ 460.0f * g_scale };
constexpr float   g_platform_elevation{ 10.0f * g_scale };
constexpr Vector2 g_platform_dimensions{ 42.0f * g_scale, 10.0f * g_scale };

static constexpr uint64_t g_fixed_frame_rate = 120;
static constexpr double   g_fixed_delta_time = 1.0 / g_fixed_frame_rate;