#pragma once
class FrameBuffer
{
public:
	FrameBuffer() {};
	~FrameBuffer();
	void init(float width, float height);

	unsigned int getFrameTexture();
	void rescaleFrameBuffer(float width, float height);
	void bind() const;
	void unbind() const;
private:
	unsigned int FBO, RBO;
	unsigned int textureID;
};

