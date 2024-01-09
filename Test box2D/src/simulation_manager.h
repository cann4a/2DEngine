#pragma once
#include <box2d/box2d.h>
#include <vector>
#include <string>
#include <map>
#include "sprite_renderer.h"
#include "texture.h"
#include "box.h"
#include "wall.h"
#include <random>

class SimulationManager
{
public:
	bool render = false;
	bool reset = false;
	bool populate = false;
	bool simulate = true;

	b2World* m_world;
	bool gravityOn;

	std::vector<Box> m_boxes;
	int boxNumber = 20;
	std::vector<Wall> m_walls;

	SimulationManager(const float RENDER_SCALE, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT);
	~SimulationManager();
	void generateRandomBox(const int boxNumber);
	void renderBoxes();
	void setGravity(b2Vec2 gravity);
	void enableGravity();
	// deletes all the boxes in the world
	void clearBoxes();
	// deletes the last box added to the world
	void clearLastBox();
	
private:
	b2Vec2 m_gravity;
	float RENDER_SCALE;
	unsigned int SCREEN_WIDTH;
	unsigned int SCREEN_HEIGHT;
	std::mt19937 randGenerator;

	void initRandomGeneration();
};

