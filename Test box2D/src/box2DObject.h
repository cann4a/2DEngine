#pragma once

#include "box2d/box2d.h"
#include <glm/glm.hpp>
#include <string>
class Box2DObject
{
public:
	Box2DObject() {};
	~Box2DObject() {};
	b2Body* getBody() { return body; }
	b2Fixture* getFixture() { return fixture; }
	glm::vec2& getDimensions() { return m_dimensions; }
	glm::vec3& getColor() { return color; }
	std::string& getName() { return m_name; }
	void setRotation(float angle) { body->SetTransform(body->GetPosition(), angle); }
	void setColor(glm::vec3 color) { this->color = color; }
	void setName(const std::string& name) { m_name = name; }
protected:
	b2Body* body = nullptr;
	b2Fixture* fixture = nullptr;
	glm::vec2 m_dimensions;
	glm::vec3 color;
	std::string m_name;
};

class Box: public Box2DObject
{
public:
	void init(b2World* world, const std::string& name, const glm::vec2& position, const glm::vec2& dimensions, const b2BodyType& type, const float rotation, const glm::vec3& color = glm::vec3(1.0))
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
		setName(name);
		setRotation(rotation);
	}
};

class Wall: public Box2DObject
{
public:
	void init(b2World* world, const std::string& name, const glm::vec2& position, const glm::vec2& dimensions, const b2BodyType& type, const float rotation, const glm::vec3& color = glm::vec3(1.0f))
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
		setName(name);
		setRotation(rotation);

	}
};

class Circle : public Box2DObject
{
public:
	void init(b2World* world, const std::string& name, const glm::vec2& position, const float radius, const b2BodyType& type,const  const glm::vec3& color = glm::vec3(1.0f))
	{
		b2BodyDef bodyDef;
		bodyDef.type = type;
		bodyDef.position.Set(position.x, position.y);
		body = world->CreateBody(&bodyDef);
		m_radius = radius;
		m_dimensions = glm::vec2(2 * m_radius , 2 * m_radius);

		b2CircleShape shape;
		shape.m_p.Set(0.0f, 0.0f);
		shape.m_radius = radius;
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.3f;
		fixtureDef.restitution = 0.5f;
		fixture = body->CreateFixture(&fixtureDef);
		setColor(color);
		setName(name);
	}

	float getRadius() { return m_radius; }
private:
	float m_radius;
};

