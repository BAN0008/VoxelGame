#pragma once

class TextureArray
{
public:
	TextureArray(const char *fileName, int depth);
	~TextureArray();

	void Bind(int slot);

	int width  = 0;
	int height = 0;
	int depth  = 0;

private:
	unsigned int textureID = 0;
};