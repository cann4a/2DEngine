#pragma once
#include <box2d/box2d.h>
#include <vector>
#include <string>
#include <map>
#include "sprite_renderer.h"
#include "texture.h"
#include "box2DObject.h"
#include <random>

enum class SimulationState
{
	PLAY,
	PAUSE,
	STOP
};

class SimulationManager
{
public:

	bool play = false;
	bool stop = false;
	bool simulate = true;

	SimulationState simulation_state;

	b2World* m_world;
	bool gravity_on;

	std::map<int, std::vector<Box2DObject>> m_objects;

	SimulationManager(const float RENDER_SCALE, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT);
	~SimulationManager();
	void setGravity(const b2Vec2 gravity);
	void enableGravity();
	void clearLastObject();
	void clearObjects();

private:
	b2Vec2 m_gravity;
	float RENDER_SCALE;
	unsigned int SCREEN_WIDTH;
	unsigned int SCREEN_HEIGHT;
	std::mt19937 rand_generator;
};

