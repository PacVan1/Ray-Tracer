#pragma once

#include <sstream>
#include "textures.h"

FILE*			openForRead(char const* path); 
void			printSaving(char const* path);
void			printLoading(char const* path); 
void			printSuccess(char const* path); 
void			printFailure(char const* path);  
void			fileNotFound(char const* path); 
Texture<float3>	loadTextureF(char const* path);    
Texture<float3>	loadTextureI(char const* path);
