#pragma once

#include "tmpl8math.h" 

float3 catmullRom(float3 const a, float3 const b, float3 const c, float3 const d, float const t);  

struct SplineNode
{
	float3	position;
	float3	target;
	bool	skydomeEnabled; 
};

struct Spline
{
	std::vector<SplineNode> nodes;
};

void save(Spline const& spline, char const* path);
void load(Spline& spline, char const* path);
void clear(Spline& spline); 

class SplineAnimator
{
public:
	Spline*		spline;
	SplineNode	interpNode;
	uint32_t	nodeIdx = 0;
	float		time;
	float		speed = 0.002f;  

public:
	SplineAnimator() : 
		spline(nullptr)
	{}
	SplineAnimator(Spline* spline) : 
		spline(spline)
	{
		Reset();
	}
	bool Play(float const deltaTime) 
	{
		std::vector<SplineNode> const& n = spline->nodes;

		time += deltaTime * speed;
		if (time >= 1.0f)
		{
			time -= 1.0f;
			nodeIdx++; 
			if (nodeIdx >= n.size() - 2)
			{
				End(); 
				return true;
			}
			InitNextNode(nodeIdx); 
		}

		int const innerIdx = nodeIdx - 1 < 0 ? 0 : nodeIdx - 1;
		int const outerIdx = nodeIdx + 2 > n.size() - 1 ? n.size() - 1 : nodeIdx + 2; 
		interpNode.position = catmullRom(n[innerIdx].position, n[nodeIdx].position, n[nodeIdx + 1].position, n[outerIdx].position, time);
		interpNode.target	= catmullRom(n[innerIdx].target, n[nodeIdx].target, n[nodeIdx + 1].target, n[outerIdx].target, time); 

		return false;
	}
	void InitNextNode(uint32_t const nextIdx)
	{
		//interpNode.target = spline->nodes[nextIdx].target;
		interpNode.skydomeEnabled = spline->nodes[nextIdx].skydomeEnabled;
	}
	void End()
	{
		//for (int i = 0; i < sizeof(SplineNode::floats3) / sizeof(float3); i++)
		//{
		//	interpNode.floats3[i] = spline->nodes[spline->nodes.size() - 1].floats3[i];
		//}
	}
	void Reset()
	{
		time = 0.0f;
		nodeIdx = 1;
		if (spline && !spline->nodes.empty())
		{
			interpNode = spline->nodes[nodeIdx];
		}
	}
};

