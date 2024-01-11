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

    // map connecting wall position and dimensions ---> map<position, dimension>
    /*std::map<std::vector<float>, std::vector<float>> wallsData = {
        {{(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, (float)SCREEN_HEIGHT / RENDER_SCALE + 10.0f} , {(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, 10.0f}},  // bottom
        {{(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, -10.0f}                                      , {(float)SCREEN_WIDTH / RENDER_SCALE / 2.0f, 10.0f}},  // top
        {{-10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}                                     , {10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}}, // left
        {{(float)SCREEN_WIDTH / RENDER_SCALE + 10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}  , {10.0f, (float)SCREEN_HEIGHT / RENDER_SCALE / 2.0f}}, // right
    };

    std::map< std::vector<float>, std::vector<float>>::iterator it;
    for (it = wallsData.begin(); it != wallsData.end(); it++) {
        b2BodyDef groundBodyDef;
        groundBodyDef.position.Set(it->first[0], it->first[1]);
        b2Body* groundBody = m_world->CreateBody(&groundBodyDef);
        // ground fixture
        b2PolygonShape groundBox;
        groundBox.SetAsBox(it->second[0], it->second[1]);
        groundBody->CreateFixture(&groundBox, 0.0f);
    }/*

    //random generator for boxes positions
    static std::vector<Box> m_boxes;
    std::mt19937 randGenerator;
    std::uniform_real_distribution<float> xPos(9.0f, 18.0f); std::uniform_real_distribution<float> yPos(0.0f, 10.0f);
    std::uniform_real_distribution<float> size(0.5f, 2.5f);
    std::uniform_real_distribution<float> rotation(0.0f, 45.0f);

    std::uniform_real_distribution<float> r(0.0f, 1.0f); std::uniform_real_distribution<float> g(0.0f, 1.0f); std::uniform_real_distribution<float> b(0.0f, 1.0f);

    // boxes creation
    const int NUM_BOXES = 20;
    for (int i = 0; i < NUM_BOXES; i++) {
        Box newBox;
        newBox.init(m_world, glm::vec2(xPos(randGenerator), yPos(randGenerator)), glm::vec2(size(randGenerator), size(randGenerator)));
        newBox.setRotation(glm::radians(rotation(randGenerator)));
        newBox.setColor(glm::vec3(r(randGenerator), g(randGenerator), b(randGenerator)));
        m_boxes.push_back(newBox);
    }*/

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

void SimulationManager::generateRandomBox(const int boxNumber) 
{
    // random generator for boxes positions
    std::uniform_real_distribution<float> xPos(100 / RENDER_SCALE, (SCREEN_WIDTH - 100) / RENDER_SCALE); std::uniform_real_distribution<float> yPos(0.0f, 10.0f);
    std::uniform_real_distribution<float> size(0.5f, 2.5f);
    std::uniform_real_distribution<float> rotation(0.0f, 45.0f);

    std::uniform_real_distribution<float> r(0.0f, 1.0f); std::uniform_real_distribution<float> g(0.0f, 1.0f); std::uniform_real_distribution<float> b(0.0f, 1.0f);

    // boxes creation
    /*for (int i = 0; i < boxNumber; i++)
    {
        Box newBox;
        newBox.init(m_world, glm::vec2(xPos(randGenerator), yPos(randGenerator)), glm::vec2(size(randGenerator), size(randGenerator)), b2_dynamicBody);
        newBox.setRotation(glm::radians(rotation(randGenerator)));
        newBox.setColor(glm::vec3(r(randGenerator), g(randGenerator), b(randGenerator)));
        m_boxes.push_back(newBox);
    }*/
}

/*void SimulationManager::clearBoxes()
{
    m_boxes.clear();
    for (b2Body* b = m_world->GetBodyList(); b != nullptr;)
    {
        b2Body* next = b->GetNext();
        if (b->GetType() == b2_dynamicBody)
            m_world->DestroyBody(b);
        b = next;
    }
}*/

/*void SimulationManager::clearLastBox()
{
    // the last body created in the world is the first one retrieved with GetBodyList()
    m_world->DestroyBody(m_world->GetBodyList());
}*/

/*void SimulationManager::clearWalls()
{
    m_walls.clear();
    for (b2Body* b = m_world->GetBodyList(); b != nullptr;)
    {
        b2Body* next = b->GetNext();
        if (b->GetType() == b2_staticBody)
            m_world->DestroyBody(b);
        b = next;
    }
}*/

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


void SimulationManager::renderBoxes() 
{
    
}

void SimulationManager::initRandomGeneration() 
{
    
   
}



