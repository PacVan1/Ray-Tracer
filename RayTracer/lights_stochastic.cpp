#include "precomp.h"
#include "lights_stochastic.h" 

#include "renderer.h"  

typedef color(*lightFunc)(LightData const& light, Intersection const& hit); 
typedef color(*lightFunc2)(LightData2 const& light, Intersection const& hit); 

static color evaluatePointLight(LightData const& light, Intersection const& hit) 
{
	float3 dir			= hit.point - light.mPointLight.mPosition;    
	float const dist	= length(dir); 
	dir					= normalize(dir); 

	Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);  
	if (hit.scene->IsOccluded(shadow)) return BLACK;   

	float const cosa		= max(0.0f, dot(hit.normal, -dir));  
	float const attenuation = 1.0f / (dist * dist); 

	return cosa * attenuation * light.mPointLight.mColor * light.mPointLight.mIntensity; 
}

static color evaluateSpotlight(LightData const& light, Intersection const& hit) 
{
	float3 dir = hit.point - light.mSpotlight.mPosition;  
	float const dist = length(dir); 
	dir = normalize(dir); 

	Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	if (hit.scene->IsOccluded(shadow)) return BLACK; 

	float const theta		= dot(dir, light.mSpotlight.mDirection); 
	float const fallOff		= clamp((theta - SPOTLIGHT_OUTER_ANGLE) / SPOTLIGHT_EPSILON, 0.0f, 1.0f);
	float const cosa		= max(0.0f, dot(hit.normal, -dir)); 
	float const attenuation = 1.0f / (dist * dist);

	return attenuation * cosa * fallOff * light.mSpotlight.mColor * light.mSpotlight.mIntensity; 
}

static color evaluatePointLights4(LightData2 const& light, Intersection const& hit)  
{
	color result = BLACK; 
	for (int i = 0; i < 4; i++)
	{
		float3 dir = hit.point - light.mPs[i].mPosition;
		float const dist = length(dir);
		dir = normalize(dir);

		Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
		if (hit.scene->IsOccluded(shadow)) return BLACK;

		float const cosa = max(0.0f, dot(hit.normal, -dir)); 
		float const attenuation = 1.0f / (dist * dist);

		return cosa * attenuation * light.mPs[i].mColor * light.mPs[i].mIntensity; 
	}
	return result;  
}

static color evaluatePointLights4SIMD(LightData2 const& light, Intersection const& hit)
{
	return BLACK;
}

static color evaluateSpotlights2(LightData2 const& light, Intersection const& hit)
{
	for (int i = 0; i < 2; i++)
	{
		float3 dir = hit.point - light.mSs[i].mPosition;
		float const dist = length(dir);
		dir = normalize(dir);

		Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps);
		if (hit.scene->IsOccluded(shadow)) return BLACK;

		float const theta = dot(dir, light.mSs[i].mDirection);
		float const fallOff = clamp((theta - SPOTLIGHT_OUTER_ANGLE) / SPOTLIGHT_EPSILON, 0.0f, 1.0f);
		float const cosa = max(0.0f, dot(hit.normal, -dir)); 
		float const attenuation = 1.0f / (dist * dist);

		return attenuation * cosa * fallOff * light.mSs[i].mColor * light.mSs[i].mIntensity; 
	}
}

static color evaluateTexturedSpotlight(LightData2 const& light, Intersection const& hit)
{
	float3 dir			= hit.point - light.mTs.mPosition; 
	float const dist	= length(dir);  
	dir					= normalize(dir); 
	
	Ray shadow = Ray(hit.point - dir * Renderer::sEps, -dir, dist - Renderer::sEps); 
	if (hit.scene->IsOccluded(shadow)) return BLACK; 
	
	float const cosa		= max(0.0f, dot(hit.normal, -dir)); 
	float const attenuation = 1.0f / (dist * dist); 
	
	float const dLeft	= distanceToFrustum(light.mTs.mFrustum.mPlanes[0], hit.point); 
	float const dRight	= distanceToFrustum(light.mTs.mFrustum.mPlanes[1], hit.point); 
	float const dTop	= distanceToFrustum(light.mTs.mFrustum.mPlanes[2], hit.point); 
	float const dBottom = distanceToFrustum(light.mTs.mFrustum.mPlanes[3], hit.point); 
	float const x = dLeft / (dLeft + dRight);
	float const y = dTop / (dTop + dBottom);
	return x >= 0.0f && x <= 1.0f && y >= 0.0f && y <= 1.0f ?   
		cosa * attenuation * light.mTs.mTexture.Sample(float2(x, y)) * light.mTs.mIntensity : BLACK;
}

lightFunc lightDispatchTable[lightTypes2::LIGHT_TYPES_COUNT] = 
{
	evaluatePointLight,
	evaluateSpotlight 
};

lightFunc2 lightDispatchTable2[lightTypes3::LIGHT_TYPES_COUNT2] =   
{
	evaluatePointLights4, 
	evaluateSpotlights2, 
	evaluateTexturedSpotlight 
};

static color evaluateLight(LightData const& light, int const type, Intersection const& hit)
{
	return lightDispatchTable[type](light, hit);   
}

static color evaluateLight2(LightData2 const& light, int const type, Intersection const& hit) 
{
	return lightDispatchTable2[type](light, hit); 
}

void transformTexturedSpotlight(LightData2& light, float3 const position, float3 const target) 
{
	light.mTs.mPosition = position; 
	light.mTs.mAhead	= normalize(target - position); 
	float3 right		= normalize(cross(float3(0.0f, 1.0f, 0.0f), light.mTs.mAhead));
	float3 up			= normalize(cross(light.mTs.mAhead, right));

	float const viewportWidth	= TEXTURED_SPOTLIGHT_VIEWPORT_HEIGHT * light.mTs.mTexture.mAspectRatio;
	float3 const viewportU		= viewportWidth * right;  
	float3 const viewportV		= TEXTURED_SPOTLIGHT_VIEWPORT_HEIGHT * -up;  

	light.mTs.mTopLeft		= light.mTs.mPosition + TEXTURED_SPOTLIGHT_VIEWPORT_DISTANCE * light.mTs.mAhead - viewportU * 0.5f - viewportV * 0.5f; 
	float3 const topRight	= light.mTs.mTopLeft + viewportU;
	float3 const bottomLeft = light.mTs.mTopLeft + viewportV;

	Frustum frustum = {};
	frustum.mPlanes[0].mNormal = cross(light.mTs.mTopLeft - bottomLeft, light.mTs.mTopLeft - light.mTs.mPosition);  // left 
	frustum.mPlanes[1].mNormal = cross(topRight - light.mTs.mPosition, light.mTs.mTopLeft - bottomLeft);			// right  
	frustum.mPlanes[2].mNormal = cross(topRight - light.mTs.mTopLeft, light.mTs.mTopLeft - light.mTs.mPosition);    // top   
	frustum.mPlanes[3].mNormal = cross(bottomLeft - light.mTs.mPosition, topRight - light.mTs.mTopLeft);			// bottom   
	for (int i = 0; i < 4; i++)
	{
		frustum.mPlanes[i].mDistance = distanceToFrustum(frustum.mPlanes[i], light.mTs.mPosition); 
	}
	light.mTs.mFrustum = frustum; 
}

LightData::LightData(int const type)  
{
	switch (type)
	{
	case LIGHT_TYPES_POINT_LIGHT:	new (&mPointLight)	PointLight2();		break;   
	case LIGHT_TYPES_SPOTLIGHT:		new (&mSpotlight)	Spotlight2();		break;  
	default: new (&mPointLight) PointLight2(); break;  
	}
}

LightData2::LightData2(int const type) 
{
	//switch (type)
	//{
	//case LIGHT_TYPES_POINT_LIGHT2:			new (&mPs)	PointLight2();			break;
	//case LIGHT_TYPES_SPOTLIGHT2:			new (&mSs)	Spotlight2();			break; 
	//case LIGHT_TYPES_TEXTURED_SPOTLIGHT2:	new (&mTs)	TexturedSpotlight2();	break; 
	//default: new (&mPs) PointLight2(); break; 
	//}

	memset(mDummy, 0, 128);  
}

LightData2::~LightData2()
{
	mTs.~TexturedSpotlight2(); 
}

LightData2::LightData2(LightData2 const& other)
{
	new (&mTs) TexturedSpotlight2(other.mTs);  
}

Lights::Lights() : 
	mCount(0)
{}

void Lights::Add(int const type)
{
	mData.emplace_back(type);   
	mTypes.emplace_back(type);   
	mCount = mData.size(); 
}

color Lights::Evaluate(Intersection const& hit) const
{
	color result = BLACK;
	for (int i = 0; i < mCount; i++)
	{
		result += evaluateLight(mData[i], mTypes[i], hit);
	}
	return result;
}

color Lights::EvaluateStochastic(Intersection const& hit) const
{
	color result = BLACK; 
	for (int i = 0; i < mSamplesPerPixel; i++)
	{
		uint8_t const randomIdx = RandomUInt() % mCount;  
		result += evaluateLight(mData[randomIdx], mTypes[randomIdx], hit) * static_cast<float>(mCount) / static_cast<float>(mSamplesPerPixel); 
	}
	return result; 
}

Lights2::Lights2() : 
	mCount(0)
{}

void Lights2::Add(int const type)
{
	mData.emplace_back(type);
	mTypes.emplace_back(type);
	mCount = mData.size(); 
}

color Lights2::Evaluate(Intersection const& hit) const  
{
	color result = BLACK;
	for (int i = 0; i < mCount; i++)
	{
		result += evaluateLight2(mData[i], mTypes[i], hit); 
	}
	return result;
}
