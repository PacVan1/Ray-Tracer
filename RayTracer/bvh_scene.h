#pragma once

#include <../lib/assimp/Importer.hpp>
#include <../lib/assimp/scene.h>  
#include <../lib/assimp/postprocess.h> 
#include <../lib/tiny_bvh.h>

#include "materials.h" 

struct alignas(32) Tri
{
	float3 points[3];
	float3 tangents[3];   
	float3 normals[3];
	float3 biTangents[3];  
	float2 texCoords[3];
	// 24 bytes of unused padding
};

struct Mesh
{
	std::vector<Tri>	tris;	// per-triangle attributes
	std::vector<float4> points; // for BVH traversal purely
	Material2*			mat;   
};

class Model
{
public:
	std::vector<Mesh>		mMeshes;
	std::vector<Material2>	mMats;    

public:
					Model(char const* path);  

private:
	void			Load(char const* path);  
	void			ConvertAIMesh(aiMesh const& aiMesh, Mesh& mesh) const;  
	void			ConvertAIMaterial(aiScene const& scene, aiMesh const& aiMesh, Mesh& mesh); 
	Texture<float3>	LoadAIMaterialTexture(aiMaterial const& aiMat, aiTextureType const type);  
	void			TraverseAINode(aiNode const& node, aiScene const& scene);
};

struct BVHSceneResources
{
	std::vector<Model>				models;
	std::vector<Mesh*>				meshes;
	std::vector<tinybvh::BVHBase*>	blasses;
};

class BVHScene 
{
public:
	tinybvh::BVH						mTlas;    
	BVHSceneResources					mResources;
	std::vector<tinybvh::BLASInstance>	mInstances;  
	std::vector<Material2>				mMatCopies; // per instance materials   

public:
			BVHScene();
	bool	Intersect(tinybvh::Ray& ray); 
	void	AddResource(char const* path); 
	void	AddInstance(uint32_t const blasIdx);
	void	SetInstanceMaterial(uint32_t const instIdx, uint32_t const matType);  
	void	ResetInstanceMaterial(uint32_t const instIdx); 
	void	Rebuild(); 

	[[nodiscard]] inline tinybvh::BLASInstance&			GetInstance(uint32_t const instIdx)			{ return mInstances[instIdx]; }
	[[nodiscard]] inline tinybvh::BLASInstance const&	GetInstance(uint32_t const instIdx) const	{ return mInstances[instIdx]; }
	[[nodiscard]] inline Mesh&							GetMesh(uint32_t const blasIdx)				{ return *mResources.meshes[blasIdx]; } 
	[[nodiscard]] inline Mesh const&					GetMesh(uint32_t const blasIdx)	const		{ return *mResources.meshes[blasIdx]; }
	[[nodiscard]] inline Material2&						GetMaterial(uint32_t const instIdx)			{ return mMatCopies[instIdx]; }
	[[nodiscard]] inline Material2 const&				GetMaterial(uint32_t const instIdx) const	{ return mMatCopies[instIdx]; }
	[[nodiscard]] inline Tri&							GetTriangle(tinybvh::Ray& ray)				{ return GetMesh(GetInstance(ray.instIdx).blasIdx).tris[ray.hit.prim]; }    
	[[nodiscard]] inline Tri const&						GetTriangle(tinybvh::Ray& ray) const		{ return GetMesh(GetInstance(ray.instIdx).blasIdx).tris[ray.hit.prim]; }  
	[[nodiscard]] inline uint32_t						GetBlasIdx(uint32_t const instIdx) const	{ return GetInstance(instIdx).blasIdx; }
};

