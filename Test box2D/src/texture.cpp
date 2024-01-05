#include "texture.h"

#include <iostream>

Texture2D::Texture2D() :
	width(0), height(0),
		wrap_s(GL_REPEAT), wrap_t(GL_REPEAT), internal_format(GL_RGB), image_format(GL_RGB), filter_min(GL_LINEAR),
			filter_max(GL_LINEAR) {
	glGenTextures(1, &this->ID);
}

void Texture2D::generate(unsigned int width, unsigned int height, unsigned char* data) {
	this->width = width;
	this->height = height;
	// create Texture
	glBindTexture(GL_TEXTURE_2D, this->ID);
	if (this->image_format == 6407)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	else if (this->image_format == 6408)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);


	// set Texture wrap and filter modes
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrap_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filter_max);
	// undind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture2D::bind() const {
	glBindTexture(GL_TEXTURE_2D, this->ID);
}
