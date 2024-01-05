#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture2D {
public:
	// holds the ID of the texture object, used for all texture operations to reference to this particular texture
	unsigned int ID;
	unsigned int width, height;
	// texture format
	GLuint internal_format; // format of texture object
	GLuint image_format; //format of the loaded image
	// texture configuration
	unsigned int wrap_s; // wrapping mode on s axis
	unsigned int wrap_t; // wrapping mode on t axis
	unsigned int filter_min; // filtering mode if texture pixels < screen pixels 
	unsigned int filter_max; // filtering mode if texture pixels < screen pixels 
	Texture2D(); 
	// generate texture from image data
	void generate(unsigned int Width, unsigned int Height, unsigned char* data);
	// binds the texture as the current active GL_TEXTURE_2D texture object
	void bind() const;
};

#endif