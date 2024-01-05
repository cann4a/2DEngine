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

	// constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath);
	// use/acativate the shader 
	void use();
	// utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float vaule) const; 
	void setVec3(const std::string& name, float v1, float v2, float v3) const;
	void setVec3(const std::string& name, glm::vec3 v) const;
	void setVec3(const std::string& name, float v[3]) const;
	void setVec4(const std::string& name, float v1, float v2, float v3, float v4) const;
	void setVec4(const std::string& name, int v1, int v2, int v3, int v4) const;
	void setVec4(const std::string& name, glm::vec4 v) const;
	void setVec4(const std::string& name, float v[4]) const;
	void setMat3(const std::string& name, glm::mat3 mat);
	void setMat4(const std::string& name, glm::mat4 mat);
};

#endif