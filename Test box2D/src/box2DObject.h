#pragma once

#include "box2d/box2d.h"
#include <glm/glm.hpp>
class Box2DObject
{
public:
	Box2DObject() {};
	~Box2DObject() {};
	b2Body* getBody() { return body; }
	b2Fixture* getFixture() { return fixture; }
	glm::vec2& getDimensions() { return m_dimensions; }
	glm::vec3& getColor() { return color; }
	void setRotation(float angle) { body->SetTransform(body->GetPosition(), angle); }
	void setColor(glm::vec3 color) { this->color = color; }
protected:
	b2Body* body = nullptr;
	b2Fixture* fixture = nullptr;
	glm::vec2 m_dimensions;
	glm::vec3 color;


};

class Box: public Box2DObject
{
public:
	void init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const b2BodyType& type, const glm::vec3& color = glm::vec3(1.0))
	{
		b2BodyDef bodyDef;
		bodyDef.type = type;
		bodyDef.position.Set(position.x, position.y);
		body = world->CreateBody(&bodyDef);
		m_dimensions = dimensions;

		b2PolygonShape shape;
		shape.SetAsBox(dimensions.x / 2, dimensions.y / 2);
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;
		fixture = body->CreateFixture(&fixtureDef);

		setColor(color);
	}
};

class Wall: public Box2DObject
{
public:
	void init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const b2BodyType& type, const glm::vec3& color = glm::vec3(1.0))
	{
		b2BodyDef bodyDef;
		bodyDef.type = type;
		bodyDef.position.Set(position.x, position.y);
		body = world->CreateBody(&bodyDef);
		m_dimensions = dimensions;

		b2PolygonShape shape;
		shape.SetAsBox(dimensions.x / 2, dimensions.y / 2);
		body->CreateFixture(&shape, 0.0f);
		setColor(color);
	}
};

