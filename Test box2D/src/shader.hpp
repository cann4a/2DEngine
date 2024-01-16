#ifndef SHADER_H
#define SHADER_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

class Shader {
public:
	// program ID
	unsigned int ID;
	Shader() {}
	Shader& use();
	void compile(const char* vertex_source, const char* fragment_source, const char* geometry_source);
	// utility uniform functions
	void    setFloat(const char* name, float value, bool use_shader = false);
	void    setInteger(const char* name, int value, bool use_shader = false);
	void    setVector2f(const char* name, float x, float y, bool use_shader = false);
	void    setVector2f(const char* name, const glm::vec2& value, bool use_shader = false);
	void    setVector3f(const char* name, float x, float y, float z, bool use_shader = false);
	void    setVector3f(const char* name, const glm::vec3& value, bool use_shader = false);
	void    setVector4f(const char* name, float x, float y, float z, float w, bool use_shader = false);
	void    setVector4f(const char* name, const glm::vec4& value, bool use_shader = false);
	void    setMatrix4(const char* name, const glm::mat4& matrix, bool use_shader = false);
private:
	void checkCompilationErrors(unsigned int object, std::string type);
};

#endif