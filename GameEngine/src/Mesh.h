#pragma once
#include <vector>
#include <string>
#include <glm/vec3.hpp>

class Mesh
{
public:
	Mesh();
	~Mesh();

	void LoadMesh(std::string file);

protected:
	std::vector<float> vertices;
	std::vector<int> triangls;
};

