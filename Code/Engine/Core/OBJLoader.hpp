#pragma once
#include <vector>
#include <string>
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
struct Triangle
{
	Triangle(int a, int b, int c)
	{
		v[0] = a;
		v[1] = b;
		v[2] = c;
	}
	int v[3] = { 0 };
	Vec3 n = Vec3(0.f, 0.f, 0.f);
	int nIndex = 0;
};

struct IndexedVertex
{
	int v = 0;
	int t = -1;
	int n = 0;
};

struct Face
{
	std::vector<IndexedVertex> vertexesList;
};

class OBJLoader
{
public:
	OBJLoader() = default;
	static void GetAFloatVec3FromAStrings(std::string const& lineString, std::vector<Vec3>& vertexes);
	static void GetAFloatVec2FromAStrings(std::string const& lineString, std::vector<Vec2>& vertexeUVs);
	static void GetAFloatVec3NormalizedFromAStrings(std::string const& lineString, std::vector<Vec3>& vertexes);
	static Vertex_PNCU MakePNCU(Vec3 const& position, Vec3 const& normal = Vec3::WORLD_ORIGIN, Vec2 const& uv = Vec2::ZERO, Rgba8 const& color = Rgba8::WHITE);
	static void AddPNCUFromTrianglesWithoutVN(std::vector<Vertex_PNCU>& vertexes, Triangle const& triangle, std::vector<Vec3>const& vertexPosi);
	static void AddPNCUFromTrianglesWithVN(std::vector<Vertex_PNCU>& vertexes, Triangle const& triangle, std::vector<Vec3>const& vertexPosi, std::vector<Vec3>const& vertexNormals);
	//Divide f to numbers(gonna use convert points to triangle), input: string of f-line, output: several triangles  
	static void GetIndexedVertexesFromAStrings(std::string const& lineString, std::vector<Vec3> const& vertexPosition, std::vector<Vec3>const& vertexNormals, std::vector<Vec2>const& vertexUVs, std::vector<Vertex_PNCU>& vertexes, std::vector<unsigned int>& indexes);

	//Convert multiple points to triangle
	static void DividePolygonToTriangles(std::vector<int> polygon, std::vector<Triangle>& triangles);
	static void DividePolygonToTriangles(std::vector<IndexedVertex> polygon, std::vector<Triangle>& triangles);

	//Add normals to triangles
	static void AddNormalsToTriangles(std::vector<Vec3>const& vertexPosi, std::vector<Triangle>& triangles);
	static void AddNormalsToATriangle(std::vector<Vec3>const& vertexPosi, Triangle& triangle);
	static Vec3 GetNormalsFromATriangle(std::vector<Vec3>const& vertexPosi, Triangle const& triangle);

	static bool LoadOBJFilePNCU(std::string const& name, Mat44 const& transformFixupMatrix, std::vector<Vertex_PNCU>& vertexes, std::vector<unsigned int>& indexes);

	static Vertex_PCUTBN MakeEmptyPCUTBN(Vec3 const& position, Vec3 const& normal = Vec3::WORLD_ORIGIN, Vec2 const& uv = Vec2::ZERO, Rgba8 const& color = Rgba8::WHITE);

	static bool LoadOBJFile(std::string const& name, Mat44 const& transformFixupMatrix, std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes);


private:

};