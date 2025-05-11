#include "precomp.h"
#include "bvh_scene.h"

Model::Model(char const* path)
{
	Load(path); 
}

std::string directory; 
void Model::Load(char const* path)
{
	static uint constexpr PROCESS_FLAGS =
		aiProcess_Triangulate |
		//aiProcess_DropNormals |  
		aiProcess_FixInfacingNormals | 
		aiProcess_GenNormals |   
		//aiProcess_GenSmoothNormals |  
		//aiProcess_MakeLeftHanded	|
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace;  

	printLoading(path);
	Assimp::Importer importer;
	aiScene const* scene = importer.ReadFile(path, PROCESS_FLAGS); 
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		printFailure(path);
		printf(importer.GetErrorString());
		return;
	}
	std::string p = path;  
	directory = p.substr(0, p.find_last_of('/') + 1);   
	TraverseAINode(*scene->mRootNode, *scene);  
	printSuccess(path);
}

void Model::ConvertAIMesh(aiMesh const& aiMesh, Mesh& mesh) const
{
	mesh.tris.resize(aiMesh.mNumFaces); 
	mesh.tris.reserve(aiMesh.mNumFaces);
	mesh.points.resize(aiMesh.mNumFaces * 3);   
	mesh.points.reserve(aiMesh.mNumFaces * 3);   
	for (int tri = 0; tri < aiMesh.mNumFaces; tri++)
 	{
		aiFace const& face = aiMesh.mFaces[tri];
		for (int vtx = 0; vtx < 3; vtx++)
		{
			int const idx = face.mIndices[vtx];
			// points:
			mesh.tris[tri].points[vtx].x = aiMesh.mVertices[idx].x; 
			mesh.tris[tri].points[vtx].y = aiMesh.mVertices[idx].y;
			mesh.tris[tri].points[vtx].z = aiMesh.mVertices[idx].z; 
			mesh.points[tri * 3 + vtx] = mesh.tris[tri].points[vtx];  
			// normals:
			mesh.tris[tri].normals[vtx].x = aiMesh.mNormals[idx].x;
			mesh.tris[tri].normals[vtx].y = aiMesh.mNormals[idx].y;
			mesh.tris[tri].normals[vtx].z = aiMesh.mNormals[idx].z;
			// texture coordinates:
			if (aiMesh.HasTextureCoords(0))
			{
				mesh.tris[tri].texCoords[vtx].x = aiMesh.mTextureCoords[0][idx].x; 
				mesh.tris[tri].texCoords[vtx].y = aiMesh.mTextureCoords[0][idx].y;  
			}
			// tangents and bitangents:
			if (aiMesh.HasTangentsAndBitangents())  
			{
				mesh.tris[tri].tangents[vtx].x		= aiMesh.mTangents[idx].x; 
				mesh.tris[tri].tangents[vtx].y		= aiMesh.mTangents[idx].y; 
				mesh.tris[tri].tangents[vtx].z		= aiMesh.mTangents[idx].z;  
				mesh.tris[tri].biTangents[vtx].x	= aiMesh.mBitangents[idx].x; 
				mesh.tris[tri].biTangents[vtx].y	= aiMesh.mBitangents[idx].y; 
				mesh.tris[tri].biTangents[vtx].z	= aiMesh.mBitangents[idx].z;  
			}
		}
	}
}

void Model::ConvertAIMaterial(aiScene const& scene, aiMesh const& aiMesh, Mesh& mesh) 
{
	if (aiMesh.mMaterialIndex >= 0)
	{
		aiMaterial& aiMat = *scene.mMaterials[aiMesh.mMaterialIndex]; 
		AlbedoTexture albedo = LoadAIMaterialTexture(aiMat, aiTextureType_DIFFUSE);
		//NormalTexture normal = LoadAIMaterialTexture(aiMat, aiTextureType_NORMALS); 
		//Texture<float> roughness; 
		//Texture<float> alpha;
		mMats.emplace_back(MATERIAL_TYPES_TEXTURED);
		mMats.back().textured.texture = packTexture(albedo);
	}
	else
	{
		mMats.emplace_back(MATERIAL_TYPES_GLOSSY);
	}
	mesh.mat = &mMats.back();  
}

Texture<float3> Model::LoadAIMaterialTexture(aiMaterial const& aiMat, aiTextureType const type) 
{
	if (aiMat.GetTextureCount(type) > 0)
	{
		aiString str; aiMat.GetTexture(type, 0, &str); 
		Texture<float3> texture = loadTextureF((directory + str.C_Str()).c_str()); 
		return texture;  
	}
	return Texture<float3>();
}

void Model::TraverseAINode(aiNode const& node, aiScene const& scene)
{
	for (int i = 0; i < node.mNumMeshes; i++) 
	{
		aiMesh const& aiMesh = *scene.mMeshes[node.mMeshes[i]];
		mMeshes.emplace_back(); ConvertAIMesh(aiMesh, mMeshes.back());
		//ConvertAIMaterial(scene, aiMesh, mMeshes.emplace_back());
		mMats.emplace_back(MATERIAL_TYPES_DIFFUSE);
		mMeshes.back().mat = &mMats.back(); 
	}
	for (int i = 0; i < node.mNumChildren; i++)
	{
		TraverseAINode(*node.mChildren[i], scene);  
	}
}

BVHScene::BVHScene()
{
	//AddResource("../assets/models/stanford_dragon_pbr/scene.gltf");  
	AddResource("../assets/models/marble_bust_01_4k.gltf/marble_bust_01_4k.gltf");   
	//AddResource("../assets/models/stanford_dragon/Dragon.obj");    
	//AddResource("../assets/models/the-utah-teapot/source/teapot/teapot.obj");      
	//AddResource("../assets/models/high_poly_sphere/ico_sphere_high_poly.obj");      
	//AddResource("../assets/models/Katanas/scene.gltf");     
	//AddResource("../assets/models/vampire/dancing_vampire.dae");    
	//AddResource("../assets/models/RAINIER AK/scene.gltf");    
	//AddResource("../assets/models/Combat Robot/scene.gltf");    
	//AddResource("../assets/models/cube/cube.gltf");  
	AddModelInstance(0); 
	//for (int x = 0; x < 10; x++) for (int y = 0; y < 10; y++) for (int z = 0; z < 10; z++)
	//{
	//	AddModelInstance(1);  
	//	mInstances.back().transform[ 3] = x * 2.0f; 
	//	mInstances.back().transform[ 7] = y * 2.0f; 
	//	mInstances.back().transform[11] = z * 2.0f;
	//}
	Rebuild();  
}

bool BVHScene::Intersect(tinybvh::Ray& ray)
{
	mTlas.Intersect(ray); 
	if (!didHit(ray)) return false; 

	Tri const& tri	= GetTriangle(ray);
	Material2& mat	= GetMaterial(ray.hit.inst);  

	float const w1 = ray.hit.u, w2 = ray.hit.v, w3 = 1.0f - w1 - w2; 
	//float3 const nInterp = normalize(berp(w1, w2, w3, tri.normals[1], tri.normals[2], tri.normals[0]));
	float3 const nInterp = calcFlatNormal(tri.points[0], tri.points[1], tri.points[2]);  

	if (mat.type == MATERIAL_TYPES_TEXTURED)   
	{
		float2 const tcInterp	= berp(w1, w2, w3, tri.texCoords[1], tri.texCoords[2], tri.texCoords[0]);      
		PackedTexel const texel = mat.textured.texture.Sample(tcInterp);  
		//float3 const texelN		= normalize(texel.normal * 2.0f - 1.0f);

		//float3 tInterp	= normalize(berp(w1, w2, w3, tri.tangents[1], tri.tangents[2], tri.tangents[0]));    
		//float3 const btInterp	= normalize(berp(w1, w2, w3, tri.biTangents[1], tri.biTangents[2], tri.biTangents[0])); 
		//tInterp = normalize(tInterp - dot(tInterp, nInterp) * nInterp); 
		//float3 const btInterp = cross(nInterp, tInterp); 
		//mat4 const tbn = { tInterp, btInterp, nInterp };    
		//ray.hit.normal = { tInterp * texelN.x + btInterp * texelN.y + nInterp * texelN.z };  
		//ray.hit.normal = normalize(ray.hit.normal); 
		//ray.hit.normal = texelN;   
		
		//float3 tangent = normalize(cross(float3(0.0f, 1.0f, 0.0f), nInterp));
		//float3 bitangent = normalize(cross(tInterp, nInterp)); 
		//mat4 const tbn = { tInterp, bitangent, nInterp }; 
		//ray.hit.normal = normalize(float3(	)); 
		//ray.hit.normal = normalize(TransformVector(texelN, tbn));     

		//mat4 normalMat = mat4::Identity(); 
		//normalMat = normalMat.Inverted(); 
		//normalMat = normalMat.Transposed();   
		//ray.hit.normal = TransformVector(ray.hit.normal, normalMat);   

		//ray.hit.normal = nInterp; 
		//float3 tangent; float3 biTangent; 
		//calcTangentSpace(tri.points[0], tri.points[1], tri.points[2], tri.texCoords[0], tri.texCoords[1], tri.texCoords[2], tangent, biTangent);
		//mat4 const tbn = { tangent, biTangent, calcFlatNormal(tri.points[0], tri.points[1], tri.points[2]) }; 

		// LearnOpenGL method:
		//float3 tangent, bitangent, normal = normalize(berp(w3, w1, w2, tri.normals[0], tri.normals[1], tri.normals[2])); 
		//calcTangentSpace(tri.points[0], tri.points[1], tri.points[2], tri.texCoords[0], tri.texCoords[1], tri.texCoords[2], tangent, bitangent); 
		//tangent = normalize(berp(w3, w1, w2, tri.tangents[0], tri.tangents[1], tri.tangents[2]));  
		//tangent = normalize(tangent - dot(tangent, normal) * normal); 
		//bitangent = cross(normal, tangent); 
		//ray.hit.normal = normalize(float3(tangent * texelN.x + bitangent * texelN.y + normal * texelN.z));    
		//ray.hit.normal = normal;  

		//float const u = ray.hit.u, v = ray.hit.v; 
		//float2 const texCoord	= (1.0f - u - v) * tri.texCoords[0] + u * tri.texCoords[1] + v * tri.texCoords[2]; 
		//PackedTexel const texel = mat.textured.texture.Sample(texCoord); 
		//float3 texelN			= normalize(normalize(texel.normal) * 2.0f - 1.0f);
		//texelN.y = -texelN.y;
		//float3 tangent			= normalize((1.0f - u - v) * tri.tangents[0] + u * tri.tangents[1]	 + v * tri.tangents[2]);   
		//float3 const normal		= normalize((1.0f - u - v) * tri.normals[0]	 + u * tri.normals[1]	 + v * tri.normals[2]);   
		//tangent = normalize(tangent - dot(tangent, normal) * normal);
		//float3 bitangent = -cross(normal, tangent);  
		//float handedness = (dot(-cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f; 
		//bitangent *= handedness; 
		//ray.hit.normal = normalize(tangent * texelN.x + bitangent * texelN.y + normal * texelN.z); 
		ray.hit.normal = calcFlatNormal(tri.points[0], tri.points[1], tri.points[2]); 

		//ray.hit.normal = normalize(TransformVector(texel.normal * 2.0f - 1.0f, tbn)); 
		//ray.hit.normal = normalize(tbn * (texel.normal * 2.0f - 1.0f));  
		//ray.hit.normal = texel.normal; 
		//ray.hit.normal = nInterp; 
		ray.hit.albedo = texel.albedo;  
	}
	else
	{
		ray.hit.normal = nInterp;
		ray.hit.albedo = WHITE;  
	}
	// multiply by normal matrix 
	ray.hit.mat = &mat; 
	ray.hit.point = calcIntersectionPoint(ray);
	return true;
}

void BVHScene::AddResource(char const* path)
{
	mResources.models.emplace_back(path);
	std::vector<Mesh>& meshes = mResources.models.back().mMeshes;
	for (Mesh& mesh : meshes)
	{
		mResources.meshes.push_back(&mesh); 
		tinybvh::BVH8_CPU* bvh = new tinybvh::BVH8_CPU();   
		bvh->Build(mesh.points.data(), mesh.tris.size());  
		mResources.blasses.push_back(bvh);    
		mesh.blasIdx = mResources.blasses.size() - 1;
	}
	printf("[RESOURCE ADDED]\t%s\n", path);
}

void BVHScene::AddModelInstance(uint32_t const modelIdx) 
{
	std::vector<Mesh> const& meshes = mResources.models[modelIdx].mMeshes; 
	for (int i = 0; i < meshes.size(); i++)
	{
		mInstances.emplace_back(meshes[i].blasIdx);
		mMatCopies.emplace_back(*meshes[i].mat);
	}
}

void BVHScene::AddInstance(uint32_t const blasIdx)
{
	mInstances.emplace_back(blasIdx); 
	mMatCopies.emplace_back(*GetMesh(blasIdx).mat);     
}

void BVHScene::SetInstanceMaterial(uint32_t const instIdx, uint32_t const matType)
{
	if (mMatCopies[instIdx].type != matType)
	{
		new (&mMatCopies[instIdx]) Material2(matType);
	}
}

void BVHScene::ResetInstanceMaterial(uint32_t const instIdx)
{
	mMatCopies[instIdx] = *GetMesh(GetBlasIdx(instIdx)).mat;
}

void BVHScene::Rebuild()
{
	mTlas.Build(mInstances.data(), mInstances.size(), mResources.blasses.data(), mResources.blasses.size());   
}