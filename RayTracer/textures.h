#pragma once

template <typename T> 
class Texture
{
public:
	T*		mData;
	int		mWidth;
	int		mHeight;
	bool	mOwnData;  

public:
	Texture() :
		mData(nullptr),
		mWidth(0),
		mHeight(0),
		mOwnData(false)
	{}
	Texture(int const width, int const height) :
		mData(malloc(sizeof(T)* width* height)),
		mWidth(width),
		mHeight(height),
		mOwnData(false)
	{}
	Texture(T* data, int const width, int const height) :
		mData(data),
		mWidth(width),
		mHeight(height),
		mOwnData(false)
	{}
	~Texture()
	{
		if (mOwnData) delete[] mData;
	}
	[[nodiscard]] T Sample(float2 const uv) const
	{
		int const ix = static_cast<int>(uv.x * static_cast<float>(mWidth));
		int const iy = static_cast<int>(uv.y * static_cast<float>(mHeight));
		int const lx = (ix % (mWidth - 1));  // looped 
		int const ly = (iy % (mHeight - 1));
		return mData[lx + ly * mWidth];
	} 
	[[nodiscard]] T SampleFiltered(float2 const uv) const 
	{
		// bilinear filtering
		float const fu = uv.x * mWidth;
		float const fv = uv.y * mHeight;
		int const	u1 = static_cast<int>(fu) % mWidth;
		int const	v1 = static_cast<int>(fv) % mHeight;
		int const	u2 = (u1 + 1) % mWidth;
		int const	v2 = (v1 + 1) % mHeight;

		// calculate fractional parts:
		float const fracu = fu - floorf(fu);
		float const fracv = fv - floorf(fv);

		// calculate weight factors:
		float const w1 = (1 - fracu) * (1 - fracv);
		float const w2 = fracu * (1 - fracv);
		float const w3 = (1 - fracu) * fracv;
		float const w4 = fracu * fracv;

		// fetch four texels: 
		T const c1 = mData[u1 + v1 * mWidth];
		T const c2 = mData[u2 + v1 * mWidth];
		T const c3 = mData[u1 + v2 * mWidth];
		T const c4 = mData[u2 + v2 * mWidth];

		return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
	}
};

class HdrTexture
{
public: 
	float3* mPixels;
	int		mWidth;
	int		mHeight;

public:
						HdrTexture(); 
						HdrTexture(char const* path);
						~HdrTexture();
	[[nodiscard]] color	Sample(float2 const uv) const;
};
