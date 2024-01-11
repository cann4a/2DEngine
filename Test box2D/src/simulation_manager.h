#pragma once
#include <box2d/box2d.h>
#include <vector>
#include <string>
#include <map>
#include "sprite_renderer.h"
#include "texture.h"
#include "box2DObject.h"
#include <random>

class SimulationManager
{
public:

	bool play = false;
	bool reset = false;
	bool populate = false;
	bool simulate = true;

	b2World* m_world;
	bool gravityOn;

	std::vector<Box2DObject> m_objects;

	//std::vector<Box> m_boxes;
	int boxNumber = 20;
	//std::vector<Wall> m_walls;
	//std::vector<Circle> m_circles;

	SimulationManager(const float RENDER_SCALE, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT);
	~SimulationManager();
	void generateRandomBox(const int boxNumber);
	void renderBoxes();
	void setGravity(const b2Vec2 gravity);
	void enableGravity();
	// deletes all the boxes in the world
	//void clearBoxes();
	// deletes the last box added to the world
	//void clearLastBox();
	// deletes all the walls
	//void clearWalls();
	void clearLastObject();
	void clearObjects();

private:
	b2Vec2 m_gravity;
	float RENDER_SCALE;
	unsigned int SCREEN_WIDTH;
	unsigned int SCREEN_HEIGHT;
	std::mt19937 randGenerator;

	void initRandomGeneration();
};

