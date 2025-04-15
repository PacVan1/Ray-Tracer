#pragma once

class HdrTexture
{
private:
	float3* mPixels;
	int		mWidth;
	int		mHeight;

public:
						HdrTexture(); 
						HdrTexture(char const* path);
	[[nodiscard]] color	Sample(float2 const uv) const;

	// TODO TEMP
	[[nodiscard]] color Sample(float3 const& direction) const;
};

