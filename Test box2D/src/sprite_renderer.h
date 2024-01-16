#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include "shader.hpp"
#include "texture.h"
#include <glm/glm.hpp>

class SpriteRenderer {
public:
	SpriteRenderer(Shader& shader);
	~SpriteRenderer();

	void drawSprite(Texture2D& texture, glm::vec2 position,
		glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f,
		glm::vec3 color = glm::vec3(1.0f));
	void drawSpriteNoTexture(glm::vec2 position,
		glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f,
		glm::vec3 color = glm::vec3(1.0f));
	void drawSpriteBox2D(float renderScale, Texture2D& texture, glm::vec2 position,
		glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f,
		glm::vec3 color = glm::vec3(1.0f));
private:
	Shader shader;
	unsigned int quad_VAO;

	void initRenderData();
};
#endif

