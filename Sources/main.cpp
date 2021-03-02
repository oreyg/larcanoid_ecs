#include "Engine.h"
#include "Arcanoid.h"

enum EArcanoidLevel
{
	ELEVEL1,
	ELEVEL2,
	ELEVEL_NUMBER
};

void level1(Arcanoid& arcanoid)
{
	arcanoid.spawn_block_grid(Vector2{ 10, 10 } *g_scale, 2, 6, Vector2{ 32, 12 } * g_scale, Vector2{ 5, 5 } *g_scale, 1);
	arcanoid.spawn_block_grid(Vector2{ 40, 100 } *g_scale, 2, 6, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 2);
	arcanoid.spawn_block_grid(Vector2{ 70, 190 } *g_scale, 2, 6, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 1);
}

void level2(Arcanoid& arcanoid)
{
	arcanoid.spawn_block_grid(Vector2{ 60, 60 } *g_scale, 2, 8, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 1);
	arcanoid.spawn_block_grid(Vector2{ 60, 95 } *g_scale, 8, 1, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 3);
	arcanoid.spawn_block_grid(Vector2{ 281, 95 } *g_scale, 8, 1, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 3);
	arcanoid.spawn_block_grid(Vector2{ 60, 230 } *g_scale, 1, 7, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 1);
	arcanoid.spawn_block_grid(Vector2{ 120, 190 } *g_scale, 1, 4, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 2);
	arcanoid.spawn_block_grid(Vector2{ 120, 120 } *g_scale, 1, 4, Vector2{ 32, 12 } *g_scale, Vector2{ 5, 5 } *g_scale, 2);
}

int main(int argc, char* argv[])
{
	Engine engine;

	auto scheduler = engine.create_actor<Scheduler>();
	auto arcanoid  = engine.create_actor<Arcanoid>(scheduler);

	engine.call_construct();

	level1(*arcanoid);
	EArcanoidLevel next_level = ELEVEL2;

	while (!engine.is_quit_requested())
	{
		engine.process();

		if (arcanoid->is_waiting_for_next_level())
		{
			switch (next_level)
			{
			case ELEVEL2:
				level2(*arcanoid);
				arcanoid->reset_to_start();
				next_level = ELEVEL_NUMBER;
				break;
			default:
				break;
			}
		}
	}

	return 0;
}