#include "precomp.h"

float3 catmullRom(float3 const a, float3 const b, float3 const c, float3 const d, float const t) 
{
	float const t2 = t * t, t3 = t2 * t;   
	return 0.5f * ((-t3 + 2 * t2 - t) * a +(3 * t3 - 5 * t2 + 2) * b +(-3 * t3 + 4 * t2 + t) * c +(t3 - t2) * d); 
}

void save(Spline const& spline, char const* path)
{
	printSaving(path);  
	std::ofstream file; file.open(path);
	for (SplineNode const& node : spline.nodes) 
	{
		file << node.position.x << ",";
		file << node.position.y << ",";
		file << node.position.z << ",";
		file << node.target.x << ",";
		file << node.target.y << ",";
		file << node.target.z;
		file << endl; 
	}
	file.close();
	printSuccess(path);  
}

void load(Spline& spline, char const* path)
{
	printLoading(path); 
	std::ifstream file = std::ifstream(path);  
	std::string line;
	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		SplineNode node;
		ss >> node.position.x; ss.ignore();
		ss >> node.position.y; ss.ignore();
		ss >> node.position.z; ss.ignore();
		ss >> node.target.x; ss.ignore();
		ss >> node.target.y; ss.ignore();
		ss >> node.target.z;
		spline.nodes.push_back(node);
	}
	file.close();

	spline.nodes[2].skydomeEnabled = true; 
	spline.nodes[4].skydomeEnabled = true; 
	spline.nodes[6].skydomeEnabled = true; 
	spline.nodes[8].skydomeEnabled = true; 
	spline.nodes[1].skydomeEnabled = false; 
	spline.nodes[3].skydomeEnabled = false; 
	spline.nodes[5].skydomeEnabled = false; 
	spline.nodes[7].skydomeEnabled = false; 

	printSuccess(path); 
}

void clear(Spline& spline) 
{
	spline.nodes.clear();
}
