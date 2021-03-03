#include "Arcanoid.h"

#include "Engine.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <random>

static inline const SDL_Rect make_sdl_rect(const Rect& rect)
{
	return { (int)(rect.position.x - rect.dimensions.x / 2.0f),
			 (int)(rect.position.y - rect.dimensions.y / 2.0f),
			 (int)rect.dimensions.x,
			 (int)rect.dimensions.y };
}

static inline const SDL_Rect make_sdl_rect(const Circle& rect)
{
	return { (int)(rect.position.x - rect.radius),
			 (int)(rect.position.y - rect.radius),
			 (int)rect.radius * 2,
			 (int)rect.radius * 2 };
}

void Arcanoid::spawn_block_grid(Vector2 offset, uint32_t cols, uint32_t rows, Vector2 block_dims, Vector2 block_offset, float HP)
{
	// Random generators for block color
	static std::default_random_engine gen(SDL_GetTicks());
	static std::uniform_int_distribution<int> index(0, EBLOCKCOLOR_NUMBER - 1);

	uint32_t width  = (uint32_t)(block_dims.x + block_offset.x) * cols;
	uint32_t height = (uint32_t)(block_dims.y + block_offset.y) * rows;
	for (uint32_t i = 0; i < rows; ++i)
	{
		for (uint32_t j = 0; j < cols; ++j)
		{
			const Vector2 position{
				(block_dims / 2 + offset + (block_dims + block_offset) * Vector2 { (float)i, (float)j }) + m_game_bounds.min
			};

			if (position.x < m_game_area.dimensions.x && position.y < m_game_area.dimensions.y)
			{
				entt::entity entity = m_registry->create();
				m_registry->emplace<Rect>(entity, position, block_dims);
				m_registry->emplace<Block>(entity);
				m_registry->emplace<Sprite>(entity, m_block_texture[index(gen)]);
				m_registry->emplace<Life>(entity, HP);
				m_registry->emplace<Collider>(entity);
			}
		}
	}
}

entt::entity Arcanoid::spawn_platform(entt::registry* registry, SDL_Texture* platform_texture)
{
	const Vector2 position{ g_game_center_s.x, g_game_area_s.y - g_platform_elevation };
	const Vector2 dimensions{ g_platform_dimensions };

	entt::entity entity = registry->create();
	registry->emplace<Platform>(entity);
	registry->emplace<Rect>(entity, position, dimensions);
	registry->emplace<Sprite>(entity, platform_texture);
	registry->emplace<Collider>(entity);
	return entity;
}

entt::entity Arcanoid::spawn_pickup(entt::registry* registry, SDL_Texture* pickup_texture)
{
	// Random generators for x position and pickup type
	static std::default_random_engine gen(SDL_GetTicks());
	static std::uniform_int_distribution<int> rand_x((int)m_game_bounds.min.x, (int)m_game_bounds.max.x);
	static std::uniform_int_distribution<int> rand_t(0, (int)EPickupType::number - 1);

	const Vector2 position{ (float)rand_x(gen), 0 };
	const Vector2 dimensions{ 20, 20 };

	entt::entity entity = registry->create();
	registry->emplace<Pickup>(entity, (EPickupType)rand_t(gen));
	registry->emplace<Rect>(entity, position, dimensions);
	registry->emplace<Sprite>(entity, pickup_texture);
	registry->emplace<Collider>(entity);
	registry->emplace<Movable>(entity, Vector2 { 0, 320 });
	return entity;
}

entt::entity Arcanoid::spawn_ball(entt::registry* registry, const Vector2& position, const Vector2 velocity, SDL_Texture* platform_texture)
{
	// Create ball in the middle of the platform, slightly above
	entt::entity entity = registry->create();
	registry->emplace<Ball>(entity);
	registry->emplace<Circle>(entity, position, g_ball_radius);
	registry->emplace<Sprite>(entity, platform_texture);
	registry->emplace<Collider>(entity);
	registry->emplace<Movable>(entity, velocity);
	return entity;
}

entt::entity Arcanoid::spawn_laser(entt::registry* registry, entt::entity platform_entity, SDL_Texture* laser_texture)
{
	Rect& platform = registry->get<Rect>(platform_entity);

	const Vector2 position { platform.position.x, platform.position.y - g_game_area_s.y / 2 };
	const Vector2 dimensions{ 15 * g_scale, g_game_area_s.y };

	entt::entity entity = registry->create();
	registry->emplace<Rect>(entity, position, dimensions);
	registry->emplace<Sprite>(entity, laser_texture);
	registry->emplace<Laser>(entity);
	registry->emplace<Attach>(entity, platform_entity, Vector2{ 0.0f, -g_game_area_s.y / 2 });
	return entity;
}

void Arcanoid::spawn_random_pickup()
{
	spawn_pickup(m_registry, m_pickup_texture);
	m_scheduler->schedule(5, std::bind(&Arcanoid::spawn_random_pickup, this));
}

void Arcanoid::reset_to_start(bool full)
{
	is_waiting_for_next_level = false;
	is_waiting_for_restart    = false;
	is_restart_allowed        = false;
	is_restart_requested      = false;

	if (full)
	{
		m_registry->clear();
	}
	else
	{
		remove_balls(m_registry);
		remove_pickups(m_registry);
	}
	
	m_scheduler->reset();
	m_scheduler->schedule(5, std::bind(&Arcanoid::spawn_random_pickup, this));

	if (m_registry->size<Platform>() == 0)
	{
		m_platform = spawn_platform(m_registry, m_platform_texture);
	}
	else
	{
		auto platform_view = m_registry->view<Platform, Rect>();
		for (auto [entity, rect] : platform_view.each()) {
			rect.position.x = g_game_center_s.x;
			rect.position.y = g_game_area_s.y - g_platform_elevation;
			rect.dimensions = g_platform_dimensions;
		}
	}

	const float platform_top = g_game_area_s.y - g_platform_elevation - g_platform_dimensions.y / 2;
	const Vector2 ball_position{ g_game_center_s.x,  platform_top - g_ball_radius - 5 * g_scale };

	m_aim_ball = spawn_ball(m_registry, ball_position, { 0, -g_ball_start_velocity }, m_ball_texture);
	m_state = EGameState::game_aim;
}

void Arcanoid::check_win_conditions()
{
	if (m_registry->size<Ball>() == 0)
	{
		--m_player_state.lives;
		if (m_player_state.lives >= 0)
		{
			// Consume life and spawn a new ball
			reset_to_start(false);
		}
		else
		{
			is_waiting_for_restart    = true;
			is_waiting_for_next_level = true;
			m_state = EGameState::score;
		}
	}

	if (m_registry->size<Block>() == 0)
	{
		is_waiting_for_next_level = true;
		m_state = EGameState::score;
	}
}

bool Arcanoid::progress_to_next_level()
{
	reset_to_start(true);
	return true;
}

void Arcanoid::reset_player_state()
{
	m_player_state = {};
}

void Arcanoid::on_construct(SDL_Renderer* renderer, entt::registry* registry)
{
	m_registry = registry;

	// Load resources
	char* base_path_cstr = SDL_GetBasePath();
	const std::string base_path{ base_path_cstr };
	SDL_free(base_path_cstr);

	constexpr std::string_view block_paths[EBLOCKCOLOR_NUMBER] {
		"Resources/images/soft_block_cyan.png",
		"Resources/images/soft_block_green.png",
		"Resources/images/soft_block_purple.png",
		"Resources/images/soft_block_red.png",
		"Resources/images/soft_block_yellow.png",
	};

	constexpr std::string_view crack_paths[ECRACKCOLOR_NUMBER] {
		"Resources/images/hard_block_cr1.png",
		"Resources/images/hard_block_cr2.png"
	};

	for (size_t i = 0; i < EBLOCKCOLOR_NUMBER; ++i)
	{
		const std::string path{ base_path + block_paths[i].data() };
		m_block_texture[i] = IMG_LoadTexture(renderer, path.c_str());
	}

	for (size_t i = 0; i < ECRACKCOLOR_NUMBER; ++i)
	{
		const std::string path{ base_path + crack_paths[i].data() };
		m_crack_texture[i] = IMG_LoadTexture(renderer, path.c_str());
	}

	m_font = TTF_OpenFont("Resources/fonts/Roboto-Regular.ttf", 12);

	{
		const std::string path{ base_path + "Resources/images/ball.png" };
		m_ball_texture = IMG_LoadTexture(renderer, path.c_str());
	}

	{
		const std::string path{ base_path + "Resources/images/platform.png" };
		m_platform_texture = IMG_LoadTexture(renderer, path.c_str());
	}

	{
		const std::string path{ base_path + "Resources/images/laser.png" };
		m_laser_texture = IMG_LoadTexture(renderer, path.c_str());
	}

	{
		const std::string path{ base_path + "Resources/images/pickup.png" };
		m_pickup_texture = IMG_LoadTexture(renderer, path.c_str());
	}

	// Start game
	reset_to_start(true);
}

void Arcanoid::on_update(float delta_time)
{
	// In regular game we want to have everything in regular update,
	// but this game is too simple for this
}

void Arcanoid::on_fixed_update()
{
	m_scheduler->pause(m_state != EGameState::game);
	if (m_state == EGameState::game)
	{
		update_balls(m_registry, m_platform, m_crack_texture);
		update_laser(m_registry);
		update_lifes(m_registry, m_player_state);
		update_pickups(m_registry, m_scheduler, m_platform, m_laser_texture);
		update_movable(m_registry);
		update_attach(m_registry);
		update_destroys(m_registry);

		check_win_conditions();
	}
}

void Arcanoid::on_render(SDL_Renderer* renderer)
{
	switch (m_state)
	{
	case EGameState::game_aim:
		render_space_hint(renderer, m_font);
		[[fallthrough]];
	case EGameState::game:
	case EGameState::pause:
		render_sprites(m_registry, renderer);
		render_player_state(renderer, m_font, m_player_state);
		break;
	case EGameState::score:
		render_final_score(renderer, m_font, m_player_state);
		break;
	}
}

void Arcanoid::on_input(EInputEvent e, bool changed)
{
	if (m_state == EGameState::game_aim)
	{
		if (m_aim_ball == entt::null || m_platform == entt::null)
		{
			return;
		}

		constexpr float ball_velocity = 120.0f * (float)g_scale * (float)g_fixed_delta_time;
		Rect&    platform = m_registry->get<Rect>(m_platform);
		Circle&  ball     = m_registry->get<Circle>(m_aim_ball);
		Movable& ball_mov = m_registry->get<Movable>(m_aim_ball);

		const Bounds plbounds = fmath::rect_to_bounds(platform);
		switch (e)
		{
		case EInputEvent::left:
			ball.position.x = ball.position.x - ball.radius > plbounds.min.x ? ball.position.x - ball_velocity : ball.position.x;
			break;
		case EInputEvent::right:
			ball.position.x = ball.position.x + ball.radius < plbounds.max.x ? ball.position.x + ball_velocity : ball.position.x;
			break;
		case EInputEvent::space:
			{
				constexpr float platform_range = 100.0f * fmath::conv_to_rad / 2.0f;
				const float delta_x = platform.position.x - ball.position.x;
				ball_mov.velocity = fmath::proj_to_hemi(platform_range, delta_x, platform.dimensions.x) * fmath::magnitude(ball_mov.velocity);
				m_state = EGameState::game;
			}
			break;
		}
	}
	else if (m_state == EGameState::game)
	{
		Rect& platform = m_registry->get<Rect>(m_platform);

		constexpr float platform_velocity = g_platform_velocity * (float) g_fixed_delta_time;
		const Bounds plbounds = fmath::rect_to_bounds(platform);
		const Bounds gabounds = fmath::rect_to_bounds(m_game_area);
		const float left_border  = gabounds.min.x + platform.dimensions.x / 2;
		const float right_border = gabounds.max.x - platform.dimensions.x / 2;
		switch (e)
		{
		case EInputEvent::left:
			platform.position.x = fmath::clamp(platform.position.x - platform_velocity, left_border, right_border);
			break;
		case EInputEvent::right:
			platform.position.x = fmath::clamp(platform.position.x + platform_velocity, left_border, right_border);
			break;
		case EInputEvent::space:
			break;
		case EInputEvent::escape:
			if (changed)
			{
				m_state = EGameState::pause;
			}
			break;
		}
	}
	else if (m_state == EGameState::pause)
	{
		if (e == EInputEvent::escape && changed)
		{
			m_state = EGameState::game;
		}
	}
	else if (m_state == EGameState::score)
	{
		if (e == EInputEvent::space && changed && is_restart_allowed)
		{
			reset_player_state();
			is_restart_requested = true;
		}
	}
}

Vector2 Arcanoid::get_entity_position(entt::registry* registry, entt::entity entity)
{
	// We have 2 types of dimensions, one for Circle and other for Rect
	Vector2 position{ NAN, NAN };
	if (Rect* rect = registry->try_get<Rect>(entity))
	{
		position = rect->position;
	}
	else if (Circle* circle = registry->try_get<Circle>(entity))
	{
		position = fmath::circle_to_rect(*circle).position;
	}

	return position;
}

bool Arcanoid::set_entity_position(entt::registry* registry, entt::entity entity, Vector2 position)
{
	if (Rect* rect = registry->try_get<Rect>(entity))
	{
		rect->position = position;
		return true;
	}
	else if (Circle* circle = registry->try_get<Circle>(entity))
	{
		circle->position = position;
		return true;
	}

	return false;
}

Arcanoid::Arcanoid(std::shared_ptr<Scheduler> scheduler) : m_scheduler(scheduler)
{
}

Arcanoid::~Arcanoid()
{
}

void Arcanoid::remove_balls(entt::registry* registry)
{
	auto ball_view = registry->view<Ball>();
	for (auto [entity] : ball_view.each())
	{
		registry->destroy(entity);
	}
}

void Arcanoid::remove_pickups(entt::registry* registry)
{
	auto pickup_view = registry->view<Pickup>();
	for (auto [entity, pickup] : pickup_view.each())
	{
		registry->destroy(entity);
	}

	auto laser_view = registry->view<Laser>();
	for (auto [entity] : laser_view.each())
	{
		registry->destroy(entity);
	}
}

void Arcanoid::update_balls(entt::registry* registry, entt::entity platform_entity, std::array<SDL_Texture*, ECRACKCOLOR_NUMBER>& crtextures)
{
	Rect platform{};
	if (registry->has<Rect>(platform_entity))
	{
		platform = registry->get<Rect>(platform_entity);
	}
	
	auto ball_view  = registry->view<Ball, Circle, Movable, Collider>();
	for (auto [entity, ball, ball_mov] : ball_view.each())
	{
		// Ball cannot exit game area
		ball.position = { fmath::clamp(ball.position.x, m_game_bounds.min.x, m_game_bounds.max.x), fmath::clamp(ball.position.y, m_game_bounds.min.y, m_game_bounds.max.y) };

		// We want to check against predicted position of the ball
		const Circle ball_nf{ ball.position + ball_mov.velocity * (float)g_fixed_delta_time, ball.radius };

		// Check the ball against the walls
		if (ball_nf.position.y < m_game_bounds.min.y)
		{
			ball_mov.velocity.y *= -1;
		}
		if (ball_nf.position.y > m_game_bounds.max.y || isnan(ball_nf.position.x) || isnan(ball_nf.position.y))
		{
			// Mark for destruction and continue with next ball
			registry->emplace<Destroy>(entity);
			continue;
		}

		if (ball_nf.position.x < m_game_bounds.min.x || ball_nf.position.x > m_game_bounds.max.x)
		{
			ball_mov.velocity.x *= -1;
		}

		const float magnitude = fmath::magnitude(ball_mov.velocity);
		const Vector2 direction = ball_mov.velocity / magnitude;

		// We want to keep our ball faster than certain amount
		if (magnitude < g_ball_start_velocity)
		{
			ball_mov.velocity = direction * g_ball_start_velocity;
		}

		// We want to avoid right angle for our ball
		if (fabs(direction.x) > 0.9 || fabs(direction.y) > 0.98)
		{
			ball_mov.velocity = fmath::rotated(ball_mov.velocity, std::copysign(1.0f, direction.x) * fmath::conv_to_rad);
		}

		auto block_view = registry->view<Block, Rect, Life, Collider>();
		for (auto [entity, block, block_life] : block_view.each())
		{
			if (!fmath::has_intersection(ball_nf, block))
			{
				continue;
			}

			Vector2 delta = ball.position - block.position;
			if (fabsf(delta.y) < block.dimensions.y)
			{
				ball_mov.velocity.x *= -1;
			}
			else if (fabsf(delta.x) < block.dimensions.x)
			{
				ball_mov.velocity.y *= -1;
			}
			else if (fabsf(delta.y) > fabsf(delta.x))
			{
				ball_mov.velocity.x *= -1;
			}
			else
			{
				ball_mov.velocity.y *= -1;
			}

			// Random generators for cracks
			if (Sprite* sprite = registry->try_get<Sprite>(entity))
			{
				static std::default_random_engine gen(SDL_GetTicks());
				static std::uniform_int_distribution<int> index(0, ECRACKCOLOR_NUMBER - 1);
				sprite->texture = crtextures[index(gen)];
			}
			
			// One hit is 1 HP
			block_life.life -= 1;
			break;
		}

		if (fmath::has_intersection(ball_nf, platform))
		{
			constexpr float platform_range = 100.0f * fmath::conv_to_rad / 2.0f;
			const float delta_x = platform.position.x - ball.position.x;
			ball_mov.velocity = fmath::proj_to_hemi(platform_range, delta_x, platform.dimensions.x) * g_ball_start_velocity;
		}
	}
}

void Arcanoid::update_lifes(entt::registry* registry, PlayerState& player_state)
{
	auto block_view = registry->view<Block, Life>();
	for (auto [entity, life] : block_view.each())
	{
		constexpr float e = 1e-15F;
		if (life.life < e)
		{
			player_state.score += life.reward;
			registry->emplace<Destroy>(entity);
		}
	}
}

void Arcanoid::update_pickups(entt::registry* registry, std::shared_ptr<Scheduler> scheduler, entt::entity platform_entity, SDL_Texture* laser_texture)
{
	Rect& platform = registry->get<Rect>(platform_entity);

	auto pickup_view = registry->view<Rect, Pickup, Collider>();
	for (auto [entity, rect, pickup] : pickup_view.each())
	{
		if (rect.position.y > m_game_bounds.max.y)
		{
			registry->emplace<Destroy>(entity);
			continue;
		}

		if (fmath::has_intersection(rect, platform))
		{
			// Those pickups could be refactored into its own components
			switch (pickup.type)
			{
			case EPickupType::platform_enlarge:
				platform.dimensions = { platform.dimensions.x + 30, platform.dimensions.y };
				scheduler->schedule(5, [registry, platform_entity]() {
					if (registry->valid(platform_entity))
					{
						Rect& platform = registry->get<Rect>(platform_entity);
						platform.dimensions = { platform.dimensions.x - 30, platform.dimensions.y };
					}
				});
				break;
			case EPickupType::triplet:
				{
					auto ball_view = registry->view<Ball, Circle, Sprite, Movable>();
					for (auto [entity, circle, sprite, movable] : ball_view.each())
					{
						// We process first entity, and then break
						spawn_ball(registry, circle.position, fmath::rotated(movable.velocity,  17 * fmath::conv_to_rad), sprite.texture);
						spawn_ball(registry, circle.position, fmath::rotated(movable.velocity, -17 * fmath::conv_to_rad), sprite.texture);
						break;
					}
				}
				break;
			case EPickupType::laser:
				{
					entt::entity laser_entity = spawn_laser(registry, platform_entity, laser_texture);
					scheduler->schedule(3, [registry, laser_entity]() {
						if (registry->valid(laser_entity))
						{
							registry->destroy(laser_entity);
						}
					});
				}
				break;
			}

			registry->emplace<Destroy>(entity);
		}
	}
}

void Arcanoid::update_destroys(entt::registry* registry)
{
	auto pickup_view = registry->view<Destroy>();
	for (auto [entity] : pickup_view.each())
	{
		registry->destroy(entity);
	}
}

void Arcanoid::update_movable(entt::registry* registry)
{
	auto circle_view = registry->view<Movable>();
	for (auto [entity, mov] : circle_view.each())
	{
		const Vector2 old_position{ get_entity_position(registry, entity) };
		const Vector2 new_position{ old_position + mov.velocity * (float)g_fixed_delta_time };
		set_entity_position(registry, entity, new_position);
	}
}

void Arcanoid::update_laser(entt::registry* registry)
{
	auto rect_view = registry->view<Rect, Laser, Attach>();
	for (auto [entity, rect, attach] : rect_view.each())
	{
		auto block_view = registry->view<Block, Rect, Life, Collider>();
		for (auto [entity, block, block_life] : block_view.each())
		{
			if (!fmath::has_intersection(rect, block))
			{
				continue;
			}

			block_life.life -= 5.0f * (float) g_fixed_delta_time;
			continue;
		}
	}
}

void Arcanoid::update_attach(entt::registry* registry)
{
	auto rect_view = registry->view<Attach>();
	for (auto [entity, attach] : rect_view.each())
	{
		if (!registry->valid(attach.parent)) 
		{
			registry->destroy(entity);
			continue;
		}

		const Vector2 new_position{ get_entity_position(registry, attach.parent) + attach.offset };
		set_entity_position(registry, entity, new_position);
	}
}

void Arcanoid::render_sprites(entt::registry* registry, SDL_Renderer* renderer)
{
	auto rect_view = registry->view<Sprite>();
	for (auto [entity, sprite] : rect_view.each())
	{
		// We have 2 types of dimensions, one for Circle and other for Rect
		SDL_Rect sdlrect{};
		if (Rect* rect = registry->try_get<Rect>(entity))
		{
			sdlrect = make_sdl_rect(*rect);
		}
		else if (Circle* circle = registry->try_get<Circle>(entity))
		{
			sdlrect = make_sdl_rect(*circle);
		}
		else
		{
			continue;
		}

		if (sprite.texture == nullptr)
		{
			SDL_SetRenderDrawColor(renderer, 244, 244, 244, 244);
			SDL_RenderFillRect(renderer, &sdlrect);
		}
		else
		{
			SDL_RenderCopy(renderer, sprite.texture, nullptr, &sdlrect);
		}
	}
}

void Arcanoid::render_text(SDL_Renderer* renderer, TTF_Font* font, Vector2 offset, Vector2 anchor, const char* text)
{
	constexpr float char_height = 20.0f * g_scale;
	constexpr float char_width  = 14.f  * g_scale;

	const size_t text_length = strlen(text);
	const int    text_width  = (int) (text_length * char_width);
	const int    text_height = (int) (char_height);

	const Vector2 text_offset{ offset.x - text_width * (1 - anchor.x), offset.y - text_height / 2 * (1 - anchor.y) };
	
	SDL_Surface* surface = TTF_RenderText_Shaded(font, text, SDL_Color{ 255, 255, 255, 255 }, {});
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect r{ (int)text_offset.x, (int)text_offset.y, text_width, text_height };
	SDL_RenderCopy(renderer, texture, nullptr, &r);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);
}

void Arcanoid::render_player_state(SDL_Renderer* renderer, TTF_Font* font, PlayerState& player_state)
{
	constexpr size_t max_text_size = 128;
	char score_text[max_text_size];
	sprintf_s(score_text, 128, "SCORE: %i", player_state.score);
	render_text(renderer, font, { 14, 14 }, {1.0f, 0.5f}, score_text);

	char lives_text[max_text_size];
	sprintf_s(lives_text, 128, "LIVES: %i", player_state.lives);
	render_text(renderer, font, { g_screen_area_s.x - (140 * g_scale), 14 }, {1.0, 0.5f}, lives_text);
}

void Arcanoid::render_final_score(SDL_Renderer* renderer, TTF_Font* font, PlayerState& player_state)
{
	if (m_registry->size<Block>() == 0)
	{
		render_text(renderer, font, g_game_center_s, { 0.5, 0.5f }, "!!!YOU WON!!!");
	}
	else
	{
		render_text(renderer, font, g_game_center_s, { 0.5, 0.5f }, "!!!GAME OVER!!!");
	}

	if (is_restart_allowed)
	{
		render_text(renderer, font, { g_game_center_s.x, m_game_bounds.max.y }, { 0.5, 0.5f }, "Press Space to restart");
	}
}

void Arcanoid::render_space_hint(SDL_Renderer* renderer, TTF_Font* font)
{
	render_text(renderer, font, { g_game_center_s.x, m_game_bounds.max.y }, { 0.5, 0.5f }, "Press Space to start");
}
