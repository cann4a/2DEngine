#include "sprite_renderer.h"

SpriteRenderer::SpriteRenderer(Shader& shader)
{
	this->shader = shader;
	this->initRenderData();
}

SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &this->quad_VAO);
}

void SpriteRenderer::initRenderData() {
	// configure VAO/VBO
	// -----------------
	unsigned int VBO;
	float vertices[] = {
		// pos	    // tex
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenVertexArrays(1, &quad_VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(quad_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void SpriteRenderer::drawSprite(Texture2D& texture, glm::vec2 position,
	glm::vec2 size, float rotate, glm::vec3 color) {
	
	// prepare tranformations
	// ----------------------

	shader.use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));

	model = glm::translate(model, glm::vec3(0.5 * size.x, 0.5 * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5 * size.x, -0.5 * size.y, 0.0));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	
	shader.setMatrix4("model", model);
	shader.setVector3f("spriteColor", color);

	glActiveTexture(GL_TEXTURE0);
	texture.bind();

	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void SpriteRenderer::drawSpriteNoTexture(glm::vec2 position,
	glm::vec2 size, float rotate, glm::vec3 color) {

	// prepare tranformations
	// ----------------------

	shader.use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));

	model = glm::translate(model, glm::vec3(0.5 * size.x, 0.5 * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5 * size.x, -0.5 * size.y, 0.0));
	model = glm::scale(model, glm::vec3(size, 1.0f));

	shader.setMatrix4("model", model);
	shader.setVector3f("spriteColor", color);

	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// Renders a sprite from position and size given by Box2D objects. The render_scale serves the 
// purpose of scaling the box2D dimensions such that they are drawable on the viewPort. 
void SpriteRenderer::drawSpriteBox2D(float render_scale, Texture2D& texture, glm::vec2 position,
	glm::vec2 size, float rotate, glm::vec3 color) {
	
	// prepare tranformations
	// ----------------------
	glm::vec2 _position = render_scale * position;
	glm::vec2 _size = render_scale * size;
	shader.use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(_position, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5 * _size.x, -0.5 * _size.y, 0.0));
	model = glm::scale(model, glm::vec3(_size, 1.0f));

	shader.setMatrix4("model", model);
	shader.setVector3f("spriteColor", color);

	glActiveTexture(GL_TEXTURE0);
	texture.bind();

	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

}