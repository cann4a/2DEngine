#include <iostream>
#include "shader.hpp"

Shader& Shader::use() {
	glUseProgram(this->ID);
	return *this;
}

void Shader::compile(const char* vertex_source, const char* fragment_source, const char* geometry_source) {
	unsigned int s_vertex, s_fragment, g_shader;
	//vertex Shader
	s_vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(s_vertex, 1, &vertex_source, NULL);
	glCompileShader(s_vertex);
	checkCompilationErrors(s_vertex, "VERTEX");
	// fragment Shader
	s_fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(s_fragment, 1, &fragment_source, NULL);
	glCompileShader(s_fragment);
	checkCompilationErrors(s_fragment, "FRAGMENT");
	// if geometry shader source code is given, also compile geometry shader
	if (geometry_source != nullptr) {
		g_shader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(g_shader, 1, &geometry_source, NULL);
		glCompileShader(g_shader);
		checkCompilationErrors(g_shader, "GEOMETRY");
	}
	// shader program 
	this->ID = glCreateProgram();
	glAttachShader(this->ID, s_vertex);
	glAttachShader(this->ID, s_fragment);
	if (geometry_source != nullptr)
		glAttachShader(this->ID, g_shader);
	glLinkProgram(this->ID);
	checkCompilationErrors(this->ID, "PROGRAM");

	glDeleteShader(s_vertex);
	glDeleteShader(s_fragment);
	if (geometry_source != nullptr)
		glDeleteShader(g_shader);
}

void Shader::checkCompilationErrors(unsigned int object, std::string type) {
	int success;
	char infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(object, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(object, 1024, NULL, infoLog);
			std::cout << "|ERROR::SHADER: Compile-time error: Type: " << type << '\n'
				<< infoLog << "\n -- -------------------------------------------------- --"
				<< std::endl;
		}
	}
	else {
		glGetProgramiv(object, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(object, 1024, NULL, infoLog);
			std::cout << "|ERROR::SHADER: Link-time error: Type: " << type << '\n'
				<< infoLog << "\n -- -------------------------------------------------- --"
				<< std::endl;
		}
	}
}
void Shader::setFloat(const char* name, float value, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform1f(glGetUniformLocation(this->ID, name), value);
}
void Shader::setInteger(const char* name, int value, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform1i(glGetUniformLocation(this->ID, name), value);
}
void Shader::setVector2f(const char* name, float x, float y, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform2f(glGetUniformLocation(this->ID, name), x, y);
}
void Shader::setVector2f(const char* name, const glm::vec2& value, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
}
void Shader::setVector3f(const char* name, float x, float y, float z, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
}
void Shader::setVector3f(const char* name, const glm::vec3& value, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
}
void Shader::setVector4f(const char* name, float x, float y, float z, float w, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
}
void Shader::setVector4f(const char* name, const glm::vec4& value, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
}
void Shader::setMatrix4(const char* name, const glm::mat4& matrix, bool use_shader)
{
	if (use_shader)
		this->use();
	glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
}
