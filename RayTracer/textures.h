#pragma once

enum textureSampleModes : uint8_t
{
	TEXTURE_SAMPLE_MODES_NONE,   
	TEXTURE_SAMPLE_MODES_UNSAFE, 
	TEXTURE_SAMPLE_MODES_LOOPED, 
	TEXTURE_SAMPLE_MODES_CLAMPED,  
};

enum textureFilterModes : uint8_t
{
	TEXTURE_FILTER_MODES_NONE,  
	TEXTURE_FILTER_MODES_NEAREST, 
	TEXTURE_FILTER_MODES_LINEAR  
};

template <typename T> 
class Texture
{
public:
	T*			mData;
	int			mWidth;
	int			mHeight;
	bool		mOwnData;  
	int8_t		mSampleMode; 
	int8_t		mFilterMode;  

public:
					Texture();
					Texture(Texture<T> const& other);
					Texture(int const width, int const height);
					Texture(T* data, int const width, int const height);
					~Texture();
	[[nodiscard]] T Sample(float2 const uv) const;  
	[[nodiscard]] T SampleNearest(float2 const uv) const; 
	[[nodiscard]] T SampleLinear(float2 const uv) const;  
	void			Clear(); 

	T&				operator[](int index);
	T const&		operator[](int index) const; 

private:
	[[nodiscard]] T SampleNearestUnsafe(float2 const uv) const;
	[[nodiscard]] T SampleNearestLooped(float2 const uv) const; 
	[[nodiscard]] T SampleNearestClamped(float2 const uv) const;  
	[[nodiscard]] T SampleLinearUnsafe(float2 uv) const; 
	[[nodiscard]] T SampleLinearLooped(float2 uv) const; 
	[[nodiscard]] T SampleLinearClamped(float2 uv) const; 
};

Texture<float3> loadTexture(char const* path);  

template <typename T> 
static void swap(Texture<T>& a, Texture<T>& b) noexcept 
{ 
	// copilot helped with a custom swap
	std::swap(a.mData, b.mData);  
}

template <typename T>
Texture<T>::Texture() :
	mData(nullptr),
	mWidth(0), 
	mHeight(0), 
	mOwnData(false),
	mSampleMode(TEXTURE_SAMPLE_MODES_NONE),  
	mFilterMode(TEXTURE_FILTER_MODES_NONE) 
{}

template <typename T>
Texture<T>::Texture(Texture<T> const& other) : 
	mData(other.mData), 
	mWidth(other.mWidth),
	mHeight(other.mHeight),
	mOwnData(other.mOwnData),
	mSampleMode(other.mSampleMode),
	mFilterMode(other.mFilterMode)
{}

template <typename T>
Texture<T>::Texture(int const width, int const height) :
	mData(static_cast<T*>(MALLOC64(sizeof(T) * width * height))),
	mWidth(width),
	mHeight(height),
	mOwnData(false),
	mSampleMode(TEXTURE_SAMPLE_MODES_NONE), 
	mFilterMode(TEXTURE_FILTER_MODES_NONE) 
{}

template <typename T>
Texture<T>::Texture(T* data, int const width, int const height) :
	mData(data),
	mWidth(width),
	mHeight(height),
	mOwnData(false),
	mSampleMode(TEXTURE_SAMPLE_MODES_NONE),  
	mFilterMode(TEXTURE_FILTER_MODES_NONE) 
{}

template <typename T> 
Texture<T>::~Texture()  
{
	if (mOwnData) FREE64(mData);   
}

template <typename T>
inline T Texture<T>::Sample(float2 const uv) const   
{
	switch (mFilterMode) 
	{
	case TEXTURE_FILTER_MODES_NEAREST:	return SampleNearest(uv);	break;   
	case TEXTURE_FILTER_MODES_LINEAR:	return SampleLinear(uv);	break;      
	default: return SampleNearest(uv); break;   
	} 
} 

template <typename T>
inline T Texture<T>::SampleNearest(float2 const uv) const
{
	switch (mSampleMode)
	{
	case TEXTURE_SAMPLE_MODES_UNSAFE:	return SampleNearestUnsafe(uv);		break;
	case TEXTURE_SAMPLE_MODES_LOOPED:	return SampleNearestLooped(uv);		break; 
	case TEXTURE_SAMPLE_MODES_CLAMPED:	return SampleNearestClamped(uv);	break;
	default: return SampleNearestClamped(uv); break;  
	}
}

template <typename T>
inline T Texture<T>::SampleLinear(float2 const uv) const 
{
	switch (mSampleMode)
	{
	case TEXTURE_SAMPLE_MODES_UNSAFE:	return SampleLinearUnsafe(uv);	break;
	case TEXTURE_SAMPLE_MODES_LOOPED:	return SampleLinearLooped(uv);	break; 
	case TEXTURE_SAMPLE_MODES_CLAMPED:	return SampleLinearClamped(uv);	break;
	default: return SampleLinearClamped(uv); break; 
	}
}

template <typename T>
T Texture<T>::SampleNearestUnsafe(float2 const uv) const
{
	int const x = static_cast<int>(uv.x * static_cast<float>(mWidth));
	int const y = static_cast<int>(uv.y * static_cast<float>(mHeight));
	return mData[x + y * mWidth]; 
}

template <typename T> 
T Texture<T>::SampleNearestLooped(float2 const uv) const
{
	int const ix = static_cast<int>(uv.x * static_cast<float>(mWidth));
	int const iy = static_cast<int>(uv.y * static_cast<float>(mHeight));
	int const x = (ix % (mWidth - 1));   
	int const y = (iy % (mHeight - 1));  
	return mData[x + y * mWidth]; 
}

template <typename T>
T Texture<T>::SampleNearestClamped(float2 const uv) const
{
	int const ix = static_cast<int>(uv.x * static_cast<float>(mWidth));
	int const iy = static_cast<int>(uv.y * static_cast<float>(mHeight));
	int const x = clamp(ix, 0, mWidth - 1);
	int const y = clamp(iy, 0, mHeight - 1); 
	return mData[x + y * mWidth];
}

template <typename T>
T Texture<T>::SampleLinearUnsafe(float2 uv) const
{
	uv.u = uv.u * static_cast<float>(mWidth); 
	uv.v = uv.v * static_cast<float>(mHeight); 

	// calculate weight factors:
	float2 const	frac	= fracf(uv);     
	float const		w1		= (1.0f - frac.u) * (1.0f - frac.v); 
	float const		w2		= frac.u * (1.0f - frac.v); 
	float const		w3		= (1.0f - frac.u) * frac.v; 
	float const		w4		= frac.u * frac.v;  

	// fetch four texels:
	int const addr = static_cast<int>(uv.x) + static_cast<int>(uv.x) * mWidth;      
	T const c1 = mData[addr];   
	T const c2 = mData[addr + 1];  
	T const c3 = mData[addr + mWidth];   
	T const c4 = mData[addr + mWidth + 1];   

	// blend:
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;  
}  

template <typename T>
T Texture<T>::SampleLinearLooped(float2 uv) const
{
	uv.u = uv.u * static_cast<float>(mWidth);
	uv.v = uv.v * static_cast<float>(mHeight);
	int const u1 = static_cast<int>(uv.x) % mWidth; 
	int const v1 = static_cast<int>(uv.y) % mHeight;  
	int const u2 = (u1 + 1) % mWidth;  
	int const v2 = (v1 + 1) % mHeight;   

	// calculate weight factors: 
	float2 const	frac	= fracf(uv); 
	float const		w1		= (1.0f - frac.u) * (1.0f - frac.v);
	float const		w2		= frac.u * (1.0f - frac.v);
	float const		w3		= (1.0f - frac.u) * frac.v;
	float const		w4		= frac.u * frac.v;

	// fetch four texels: 
	T const c1 = mData[u1 + v1 * mWidth];
	T const c2 = mData[u2 + v1 * mWidth]; 
	T const c3 = mData[u1 + v2 * mWidth]; 
	T const c4 = mData[u2 + v2 * mWidth]; 

	// blend:
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
} 

template <typename T>
T Texture<T>::SampleLinearClamped(float2 uv) const
{
	uv.u = uv.u * static_cast<float>(mWidth); 
	uv.v = uv.v * static_cast<float>(mHeight); 
	int const iu = static_cast<int>(uv.u);   
	int const iv = static_cast<int>(uv.v);   
	int const u1 = clamp(iu, 0, mWidth - 1);
	int const v1 = clamp(iv, 0, mHeight - 1);   
	int const u2 = clamp(iu + 1, 0, mWidth - 1);  
	int const v2 = clamp(iv + 1, 0, mHeight - 1);  

	// calculate weight factors:
	float2 const	frac = fracf(uv);     
	float const		w1 = (1.0f - frac.u) * (1.0f - frac.v);
	float const		w2 = frac.u * (1.0f - frac.v);
	float const		w3 = (1.0f - frac.u) * frac.v;
	float const		w4 = frac.u * frac.v;

	// fetch four texels: 
	T const c1 = mData[u1 + v1 * mWidth];
	T const c2 = mData[u2 + v1 * mWidth]; 
	T const c3 = mData[u1 + v2 * mWidth];
	T const c4 = mData[u2 + v2 * mWidth]; 

	// blend:
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
}  
 
template <typename T>
void Texture<T>::Clear()
{
	memset(mData, 0, mWidth * mHeight * sizeof(T));    
}

template <typename T>
inline T& Texture<T>::operator[](int index)
{
	return mData[index];
}

template <typename T> 
inline T const& Texture<T>::operator[](int index) const 
{
	return mData[index];  
} 
