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
						~HdrTexture();
	[[nodiscard]] color	Sample(float2 const uv) const;
};

