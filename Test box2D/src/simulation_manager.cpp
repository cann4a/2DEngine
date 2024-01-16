#include "simulation_manager.h"
#include "box2DObject.h"
#include <random>
#include <map>
#include "resource_manager.h"



SimulationManager::SimulationManager(const float RENDER_SCALE, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT)
{
    this->RENDER_SCALE = RENDER_SCALE;
    this->SCREEN_WIDTH = SCREEN_WIDTH;
    this->SCREEN_HEIGHT = SCREEN_HEIGHT;

    gravityOn = true;
	m_gravity = b2Vec2(0.0f, 10.0f);
	m_world = new b2World(m_gravity);
}
void SimulationManager::setGravity(const b2Vec2 gravity) 
{
    m_gravity = gravity;
    m_world->SetGravity(m_gravity);
}
void SimulationManager::enableGravity()
{
    if (gravityOn)
        m_world->SetGravity(m_gravity);
    else
        m_world->SetGravity(b2Vec2_zero);
}

SimulationManager::~SimulationManager() 
{
    delete m_world;
}

void SimulationManager::clearObjects()
{
    m_objects.clear();
    for (b2Body* b = m_world->GetBodyList(); b != nullptr;)
    {
        b2Body* next = b->GetNext();
        m_world->DestroyBody(b);
        b = next;
    }
}

void SimulationManager::clearLastObject()
{
    if(m_world->GetBodyCount())
        m_world->DestroyBody(m_world->GetBodyList());
}

