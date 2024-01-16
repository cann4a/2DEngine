#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

// instantiate static variables
std::map<std::string, Shader> ResourceManager::shaders;
std::map<std::string, Texture2D> ResourceManager::textures;

Shader ResourceManager::loadShader(const char* v_shader_file, const char* f_shader_file, const char* g_shader_file, std::string name) {
	shaders[name] = loadShaderFromFile(v_shader_file, f_shader_file, g_shader_file);
	return shaders[name];
}

Shader& ResourceManager::getShader(std::string name) {
	return shaders[name];
}

Texture2D ResourceManager::loadTexture(const char* file, bool alpha, std::string name) {
	textures[name] = loadTextureFromFile(file, alpha);
	return textures[name];
}

Texture2D& ResourceManager::getTexture(std::string name) {
	return textures[name];
}

void ResourceManager::clear() {
	// (properly) delete all shaders
	for (auto iter : shaders)
		glDeleteProgram(iter.second.ID);
	// (properly) delete all textures
	for (auto iter : textures)
		glDeleteProgram(iter.second.ID);
}

Shader ResourceManager::loadShaderFromFile(const char* v_shader_file, const char* f_shader_file, const char* g_shader_file) {
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertex_code;
	std::string fragment_code;
	std::string geometry_code;
	try {
		// open files
		std::ifstream vertex_shader_file(v_shader_file);
		std::ifstream fragment_shader_file(f_shader_file);
		std::stringstream v_shader_stream, f_shader_stream;
		// read file's buffer contents into streams
		v_shader_stream << vertex_shader_file.rdbuf();
		f_shader_stream << fragment_shader_file.rdbuf();
		// close file handlers
		vertex_shader_file.close();
		fragment_shader_file.close();
		// convert stream into string
		vertex_code = v_shader_stream.str();
		fragment_code = f_shader_stream.str();
		// if geometry shader path is present, also load geometry shader
		if (g_shader_file != nullptr) {
			std::ifstream geometry_shader_file(g_shader_file);
			std::stringstream g_shader_stream;
			g_shader_stream << geometry_shader_file.rdbuf();
			geometry_shader_file.close();
			geometry_code = g_shader_stream.str();
		}
	}
	catch (std::exception e) {
		std::cout << "ERROR:SHADER: Failed to read shader files" << std::endl;
	}
	const char* v_shader_code = vertex_code.c_str();
	const char* f_shader_code = fragment_code.c_str();
	const char* g_shader_code = geometry_code.c_str();
	// 2. now create shader object from source code
	Shader shader;
	shader.compile(v_shader_code, f_shader_code, g_shader_file != nullptr ? g_shader_code : nullptr);
	return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha) {
	// create texture object 
	Texture2D texture;
	if (alpha) {
		texture.internal_format = GL_RGBA;
		texture.image_format = GL_RGBA;
	}
	// load image
	int width, height, nr_channels;
	unsigned char* data = stbi_load(file, &width, &height, &nr_channels, 0);
	// generate texture
	texture.generate(width, height, data);
	// free image data
	stbi_image_free(data);
	return texture;
}