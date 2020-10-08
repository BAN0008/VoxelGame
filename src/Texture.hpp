#pragma once

class Texture
{
public:
	Texture(const char *fileName);
	~Texture();

	void Bind(int slot);

	int width  = 0;
	int height = 0;
private:
	unsigned int textureID = 0;
};