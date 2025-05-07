#include "precomp.h"
#include "resources.h"

#include "noise.h" 
#include <../lib/assimp/Importer.hpp>
#include <../lib/assimp/scene.h>  
#include <../lib/assimp/postprocess.h> 

void ResourceManager::Init()
{
	BlueNoise::GetInstance(); 
}
