#include "wall.h"
#include "simulation_manager.h"

void Wall::init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions)
{
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(position.x, position.y);
    body = world->CreateBody(&groundBodyDef);
    m_dimensions = dimensions;
    // ground fixture
    b2PolygonShape groundBox;
    groundBox.SetAsBox(dimensions.x / 2.0, dimensions.y / 2.0);
    body->CreateFixture(&groundBox, 0.0f);
}