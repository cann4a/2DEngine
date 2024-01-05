#pragma once
#include <box2d/box2d.h>
#include <vector>
#include <string>
#include <map>
#include "sprite_renderer.h"
#include "texture.h"

class SimulationManager
{
public:
	b2World* m_world;
	bool gravityOn;
	SimulationManager(float RENDER_SCALE, unsigned int SCREEN_WIDTH, unsigned int SCREEN_HEIGHT);
	~SimulationManager();
	void setGravity(b2Vec2 gravity);
	void enableGravity();
	void reset();
	
private:
	b2Vec2 m_gravity;
};

