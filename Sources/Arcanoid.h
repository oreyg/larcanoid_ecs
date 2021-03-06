#pragma once
#include "Config.h"
#include "Actor.h"
#include "FMath.h"
#include "Timer.h"

#include <type_traits>
#include <string>
#include <memory>

#include <entt/entt.hpp>

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;
struct Mix_Chunk;
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

// Enums for resources
enum EBlockColor
{
	EBLOCKCOLOR_CYAN = 0,
	EBLOCKCOLOR_GREEN,
	EBLOCKCOLOR_PURPLE,
	EBLOCKCOLOR_RED,
	EBLOCKCOLOR_YELLOW,
	EBLOCKCOLOR_NUMBER
};

enum ECrackColor
{
	ECRACKCOLOR_1 = 0,
	ECRACKCOLOR_2,
	ECRACKCOLOR_NUMBER
};

enum EHitSound
{
	EHITSOUND_TOUCH = 0,
	EHITSOUND_BREAK,
	EHITSOUND_WALLS,
	EHITSOUND_PLATFORM,
	EHITSOUND_BONUS,
	EHITSOUND_GROUND,
	EHITSOUND_FAILURE,
	EHITSOUND_NUMBER
};

enum class EPickupType
{
	triplet = 0,
	platform_enlarge,
	laser,
	number
};

enum class EGameState
{
	game_aim,
	game,
	pause,
	score
};

struct PlayerState
{
	int lives{ 3 };
	int score{};
	int level{};
};

struct Life
{
	float    life  { 1.0f };
	uint32_t reward{ 200 };
};

struct Movable
{
	Vector2 velocity;
};

struct Sprite
{
	SDL_Texture* texture{ nullptr };
	float        alpha{ 1.0f };
};

struct Pickup
{
	EPickupType type;
};

struct Collider {};
struct Block {};
struct Platform {};
struct Ball {};
struct Destroy {};
struct Laser {};

struct Attach 
{
	entt::entity parent;
	Vector2      offset;
};

struct Resources
{
	// Resources
	TTF_Font*    ttf_font{ nullptr };
	SDL_Texture* tex_block[EBLOCKCOLOR_NUMBER]{};
	SDL_Texture* tex_crack[ECRACKCOLOR_NUMBER]{};

	SDL_Texture* tex_ball{};
	SDL_Texture* tex_platform{};
	SDL_Texture* tex_laser{};
	SDL_Texture* tex_pickup{};

	Mix_Chunk* mix_hit[EHITSOUND_NUMBER]{};
	Mix_Chunk* mix_laser_on{};

	Mix_Music* music{};

	void construct(SDL_Renderer* renderer, entt::registry* registry);
};

class Arcanoid final : public Actor
{
private:
	std::shared_ptr<Scheduler> m_scheduler;

	static constexpr Rect   m_game_area  { g_game_center_s, g_game_area_s     };
	static constexpr Bounds m_game_bounds{ fmath::rect_to_bounds(m_game_area) };

	EGameState   m_state{ EGameState::game_aim };
	PlayerState  m_player_state;

	entt::registry* m_registry{ nullptr };

	entt::entity m_platform{ entt::null };
	entt::entity m_aim_ball{ entt::null };

	Resources res;

public:
	bool is_restart_allowed = false;
	bool is_restart_requested = false;
	bool is_waiting_for_next_level = false;
	bool is_waiting_for_restart = false;

	static void render_text(SDL_Renderer* renderer, TTF_Font* font, Vector2 offset, Vector2 anchor, const char* text);
	void render_player_state(SDL_Renderer* renderer, TTF_Font* font, PlayerState& player_state);
	void render_final_score(SDL_Renderer* renderer, TTF_Font* font, PlayerState& player_state);
	void render_space_hint(SDL_Renderer* renderer, TTF_Font* font);
	
	void spawn_block_grid(Vector2 offset, uint32_t cols, uint32_t rows, Vector2 block_dims, Vector2 block_offset, float HP);

	void spawn_random_pickup();
	void reset_to_start(bool full);
	void check_win_conditions();

	bool progress_to_next_level();
	void reset_player_state();

	virtual void on_construct(SDL_Renderer* renderer, entt::registry* registry) override;
	virtual void on_update(float delta_time) override;
	virtual void on_fixed_update() override;
	virtual void on_render(SDL_Renderer* renderer) override;
	virtual void on_input(EInputEvent e, bool changed) override;

	static Vector2 get_entity_position(entt::registry* registry, entt::entity entity);
	static bool    set_entity_position(entt::registry* registry, entt::entity entity, Vector2 position);

	// Those functions could be moved into separate files, if you want to refactor it that way
	static entt::entity spawn_platform(entt::registry* registry, SDL_Texture* platform_texture);
	static entt::entity spawn_pickup(entt::registry* registry, SDL_Texture* pickup_texture);
	static entt::entity spawn_ball(entt::registry* registry, const Vector2& position, const Vector2 velocity, SDL_Texture* platform_texture);
	static entt::entity spawn_laser(entt::registry* registry, entt::entity platform_entity, SDL_Texture* laser_texture);

	static void remove_balls(entt::registry* registry);
	static void remove_pickups(entt::registry* registry);

	static void update_balls(entt::registry* registry, entt::entity platform_entity, Resources& res);
	static void update_lifes(entt::registry* registry, PlayerState& player_state);
	static void update_pickups(entt::registry* registry, std::shared_ptr<Scheduler> scheduler, entt::entity platform_entity, Resources& res);
	static void update_destroys(entt::registry* registry);
	static void update_movable(entt::registry* registry);
	static void update_laser(entt::registry* registry);
	static void update_attach(entt::registry* registry);

	static void render_sprites(entt::registry* registry, SDL_Renderer* renderer);

	Arcanoid(std::shared_ptr<Scheduler> scheduler);
	virtual ~Arcanoid();
	Arcanoid(Arcanoid&) = delete;
};