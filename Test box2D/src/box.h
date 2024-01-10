#pragma once
#include <box2d/box2d.h>
#include <glm/glm.hpp>
class Box
{
public:
	Box() {};
	~Box() {};
	void init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const glm::vec3& color = glm::vec3(1.0));
	b2Body* getBody() { return body; }
	b2Fixture* getFixture() { return fixture; }
	glm::vec2& getDimensions() { return m_dimensions; }
	glm::vec3& getColor() { return color; }
	void setRotation(float angle) { body->SetTransform(body->GetPosition(), angle); }
	void setColor(glm::vec3 color) { this->color = color; }
private:
	b2Body* body = nullptr;
	b2Fixture* fixture = nullptr;
	glm::vec2 m_dimensions;
	glm::vec3 color;
};

