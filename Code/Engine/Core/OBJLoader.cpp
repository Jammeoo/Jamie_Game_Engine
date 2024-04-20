#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"

void OBJLoader::GetAFloatVec3FromAStrings(std::string const& lineString, std::vector<Vec3>& vertexes)
{
	Strings stringList = SplitStringOnDelimiter(lineString, ' ');
	size_t size = size_t((int)stringList.size() - 3);
	float numX = (float)atof(stringList[(size + 0)].c_str());
	float numY = (float)atof(stringList[(size + 1)].c_str());
	float numZ = (float)atof(stringList[(size + 2)].c_str());
	Vec3 posi = Vec3(numX, numY, numZ);
	vertexes.push_back(posi);
}

void OBJLoader::GetAFloatVec2FromAStrings(std::string const& lineString, std::vector<Vec2>& vertexeUVs)
{
	Strings stringList = SplitStringOnDelimiter(lineString, ' ');
	size_t size = size_t((int)stringList.size() - 3);
	float numX = (float)atof(stringList[(size + 0)].c_str());
	float numY = (float)atof(stringList[(size + 1)].c_str());
	Vec2 uv = Vec2(numX, numY);
	vertexeUVs.push_back(uv);
}

void OBJLoader::GetAFloatVec3NormalizedFromAStrings(std::string const& lineString, std::vector<Vec3>& vertexes)
{
	Strings stringList = SplitStringOnDelimiter(lineString, ' ');
	size_t size = size_t((int)stringList.size() - 3);
	float numX = (float)atof(stringList[(size + 0)].c_str());
	float numY = (float)atof(stringList[(size + 1)].c_str());
	float numZ = (float)atof(stringList[(size + 2)].c_str());
	Vec3 posi = Vec3(numX, numY, numZ);
	Vec3 posiNormal = posi.GetNormalized();
	vertexes.push_back(posiNormal);
}



void OBJLoader::GetIndexedVertexesFromAStrings(std::string const& lineString, std::vector<Vec3> const& vertexPosition, std::vector<Vec3>const& vertexNormals, std::vector<Vec2>const& vertexUVs, std::vector<Vertex_PNCU>& vertexes, std::vector<unsigned int>& indexes)
{
	Strings stringList = SplitStringOnDelimiter(lineString, ' ');
	int listLength = (int)stringList.size();
	bool hasForward = false;
	for (int listIndex = 0; listIndex < listLength; listIndex++) 
	{
		Strings checkList = SplitStringOnDelimiter(stringList[listIndex], '/');
		size_t checkLength = checkList.size();
		if (checkLength > 1) 
		{
			hasForward = true;
			break;
		}
	}
	
	if (!hasForward)									// in the form of f x x x x or f x x x
 														//Not in the form of x//x or x/x/x
	{
		// [f] [65] [55] [44] [12]
		//get rid of extra space
		
		std::vector<int> currentPolygon;
		for (int elementIndex = 1; elementIndex < listLength; elementIndex++) //index starts from 1 to ignore [f]
		{
			if (stringList[elementIndex] != "")//ignore ""
			{
				int a = atoi(stringList[elementIndex].c_str()) - 1;
				currentPolygon.push_back(a);
			}
		}
		// [65] [55] [44] [12]

		if (currentPolygon.size() > 3) // not triangle
		{
			Triangle tri(currentPolygon[0], currentPolygon[1], currentPolygon[2]);
			Vec3 sharedNormal = GetNormalsFromATriangle(vertexPosition, tri);

// 			for (int vertIndex = 0; vertIndex < (int)currentPolygon.size(); vertIndex++) 
// 			{
// 				int posiIndex = currentPolygon[vertIndex];
// 				Vec3 posi = vertexPosition[posiIndex];
// 				Vertex_PNCU vert = MakePNCU(posi, sharedNormal);
// 				vertexes.push_back(vert);
// 			}
			
			std::vector<Triangle> triangleIndexesFromPolygon;
			DividePolygonToTriangles(currentPolygon, triangleIndexesFromPolygon);

			for (int triIndex = 0; triIndex < (int)triangleIndexesFromPolygon.size(); triIndex++) 
			{
				int posiIndexA = triangleIndexesFromPolygon[triIndex].v[0];
				Vec3 posiA = vertexPosition[posiIndexA];
				Vertex_PNCU vertA = MakePNCU(posiA, sharedNormal);

				int posiIndexB = triangleIndexesFromPolygon[triIndex].v[1];
				Vec3 posiB = vertexPosition[posiIndexB];
				Vertex_PNCU vertB = MakePNCU(posiB, sharedNormal);

				int posiIndexC = triangleIndexesFromPolygon[triIndex].v[2];
				Vec3 posiC = vertexPosition[posiIndexC];
				Vertex_PNCU vertC = MakePNCU(posiC, sharedNormal);

				vertexes.push_back(vertA);
				vertexes.push_back(vertB);
				vertexes.push_back(vertC);
			}

			for (int triIndex = 0; triIndex < (int)triangleIndexesFromPolygon.size(); triIndex++)
			{
				//Triangle currentTri = triangleIndexesFromPolygon[triIndex];
				int sizeOfIndex = (int)indexes.size();
				int startOfIndex = sizeOfIndex;
				int indexA = startOfIndex + 0;
				int indexB = startOfIndex + 1;
				int indexC = startOfIndex + 2;

				indexes.push_back(indexA);
				indexes.push_back(indexB);
				indexes.push_back(indexC);
			}


		}
		else							// Triangle
		{
			std::vector<int> currentTri;
			for (int elementIndex = 1; elementIndex < listLength; elementIndex++) //index starts from 1 to ignore [f]
			{
				if (stringList[elementIndex] != "")//ignore " "
				{
					int a = atoi(stringList[elementIndex].c_str()) - 1;
					currentTri.push_back(a);
				}
			}
			int currentIndexSize = (int)indexes.size();
			for (int pointIndex = 0; pointIndex < (int)currentTri.size(); pointIndex++) 
			{
				indexes.push_back(currentIndexSize + pointIndex);
			}

			int triangleIndexA = currentTri[0];
			int triangleIndexB = currentTri[1];
			int triangleIndexC = currentTri[2];
			Vec3 pointA = vertexPosition[triangleIndexA];
			Vec3 pointB = vertexPosition[triangleIndexB];
			Vec3 pointC = vertexPosition[triangleIndexC];

			Vec3 aToB = pointB - pointA;
			Vec3 bToC = pointC - pointB;
			Vec3 normalABC = CrossProduct3D(aToB, bToC);

			Vertex_PNCU vertA = MakePNCU(pointA, normalABC);
			Vertex_PNCU vertB = MakePNCU(pointB, normalABC);
			Vertex_PNCU vertC = MakePNCU(pointC, normalABC);
			vertexes.push_back(vertA);
			vertexes.push_back(vertB);
			vertexes.push_back(vertC);
		}

	}
	else												//In the form of f x//x or x/x/x //Not in the form of x x x x or f x x x										
	{
		Strings cleanStringList;//without f or " ", only numbers or ' '
		
		for (int elementIndex = 1; elementIndex < listLength; elementIndex++) //index starts from 1 to ignore [f]
		{
			if (stringList[elementIndex] != "") //ignore " "
			{
				cleanStringList.push_back(stringList[elementIndex]);
			}
// 			if (stringList[elementIndex] != " ")
// 			{
// 				//int a = atoi(stringList[elementIndex].c_str());
// 				//currentPolygon.push_back(a);
// 			}
		}

		int cleanStringListLength = (int)cleanStringList.size();

		std::vector<IndexedVertex> currentPolygon;
		//if (cleanStringListLength > 3)
		//{
			for (int elementIndex = 0; elementIndex < cleanStringListLength; elementIndex++) 
			{
				std::string unitElementString = cleanStringList[elementIndex];

				Strings stringDividedList = SplitStringOnDelimiter(unitElementString, '/');//[9][][2] or [9][1][2]

				IndexedVertex iVert;
				iVert.v = atoi(stringDividedList[0].c_str()) - 1;
				if (stringDividedList[1] != "") 
				{
					iVert.t = atoi(stringDividedList[1].c_str()) - 1;
				}
				iVert.n = atoi(stringDividedList[2].c_str()) - 1;
				currentPolygon.push_back(iVert);
			}

// 			for (int elementIndex = 0; elementIndex < (int)currentPolygon.size(); elementIndex++) 
// 			{
// 				int posiIndex = currentPolygon[elementIndex].v;
// 				int normalIndex = currentPolygon[elementIndex].n;
// 
// 				Vec2 textureUV = Vec2::ZERO;
// 				if (currentPolygon[elementIndex].t != -1)
// 				{
// 					int uvIndex = currentPolygon[elementIndex].t;
// 					textureUV = vertexUVs[uvIndex];
// 				}
// 
// 				Vec3 posi = vertexPosition[posiIndex];
// 				Vec3 norm = vertexNormals[normalIndex];
// 
// 				Vertex_PNCU vert = MakePNCU(posi, norm, textureUV);
// 				vertexes.push_back(vert);
// 			}


			for (int vertsIndex = 0; vertsIndex < (int)currentPolygon.size() - 2; vertsIndex++) 
			{
				//[0][1][2][3]
				//[22][23][24][25]
				//loop twice. Each for one triangle
				//for one triangle, make them PNCU and push back those three points

				int indexA = 0;
				int indexB = vertsIndex + 1;
				int indexC = vertsIndex + 2;


				int posiIndexA = currentPolygon[indexA].v;
				int normalIndexA = currentPolygon[indexA].n;

				Vec2 textureUVA = Vec2::ZERO;
				if (currentPolygon[indexA].t != -1)
				{
					int uvIndexA = currentPolygon[indexA].t;
					textureUVA = vertexUVs[uvIndexA];
				}

				Vec3 posiA = vertexPosition[posiIndexA];
				Vec3 normA = vertexNormals[normalIndexA];

				Vertex_PNCU vertA = MakePNCU(posiA, normA, textureUVA);
				vertexes.push_back(vertA);


				int posiIndexB = currentPolygon[indexB].v;
				int normalIndexB = currentPolygon[indexB].n;

				Vec2 textureUVB = Vec2::ZERO;
				if (currentPolygon[indexB].t != -1)
				{
					int uvIndexB = currentPolygon[indexB].t;
					textureUVB = vertexUVs[uvIndexB];
				}

				Vec3 posiB = vertexPosition[posiIndexB];
				Vec3 normB = vertexNormals[normalIndexB];

				Vertex_PNCU vertB = MakePNCU(posiB, normB, textureUVB);
				vertexes.push_back(vertB);

				int posiIndexC = currentPolygon[indexC].v;
				int normalIndexC = currentPolygon[indexC].n;

				Vec2 textureUVC = Vec2::ZERO;
				if (currentPolygon[indexC].t != -1)
				{
					int uvIndexC = currentPolygon[indexC].t;
					textureUVC = vertexUVs[uvIndexC];
				}

				Vec3 posiC = vertexPosition[posiIndexC];
				Vec3 normC = vertexNormals[normalIndexC];

				Vertex_PNCU vertC = MakePNCU(posiC, normC, textureUVC);
				vertexes.push_back(vertC);
			}
			
			
			for (int index = 0; index < (int)currentPolygon.size() - 2; index++) 
			{
				int sizeOfIndex = (int)indexes.size();
				int startOfIndex = sizeOfIndex;
				int indexA = startOfIndex + 0;
				int indexB = startOfIndex + 1;
				int indexC = startOfIndex + 2;

				indexes.push_back(indexA);
				indexes.push_back(indexB);
				indexes.push_back(indexC);

			}

	}

	


// 	std::vector<Triangle> triangle;
// 	int listLength = (int)stringList.size();
// 	if (int(stringList[2].size()) == 1) 
// 	{
// 
// 		
// 
// 		//Convert to triangles
// 		std::vector<int> currentPolygon;
// 		
// 		//remove "f"
	// 		for (int elementIndex = 1; elementIndex < listLength; elementIndex++) //index starts from 1 to ignore [f]
// 		{
// 			if (stringList[elementIndex] != " ")//ignore " "
// 			{
// 				int a = atoi(stringList[elementIndex].c_str());
// 				currentPolygon.push_back(a);
// 			}
// 		}
// 
// 		std::vector<Triangle> triangleIndexesFromPolygon;
// 		DividePolygonToTriangles(currentPolygon, triangleIndexesFromPolygon);
// 		AddNormalsToTriangles(vertexPosition, triangleIndexesFromPolygon);
// 
// 
// 		//For each triangle, save them into indexes vector
// 		int triangleNumber = (int)triangleIndexesFromPolygon.size();
// 		indexes.reserve(indexes.size() + triangleNumber * 3);
// 
// 		for (int triIndex = 0; triIndex < triangleNumber; triIndex++)
// 		{
// 			Triangle currentTri = triangleIndexesFromPolygon[triIndex];
// 			int a = currentTri.v[0];
// 			int b = currentTri.v[1];
// 			int c = currentTri.v[2];
// 
// 
// 
// 			indexes.push_back(a);
// 			indexes.push_back(b);
// 			indexes.push_back(c);
// 
// 		}
// 	}
// 	else
// 	{
// 		//[f] [1//2] [7//2] [5//2] [4//2]
// 
// 		//Convert to triangles first
// 
// 		for (int elementIndex = 1; elementIndex < listLength; elementIndex++)	//index starts from 1 to ignore [f]
// 		{
// 			Strings unitVertexList = SplitStringOnDelimiter(lineString, '/');				//[1][ ][2]
// 
// 			int a = atoi(unitVertexList[0].c_str());
// 			int b = 0;
// 			if (unitVertexList[1] == "") //#ToDo: check is "" or " "
// 			{
// 				b = 0;
// 			}
// 			else
// 			{
// 				b = atoi(unitVertexList[1].c_str());
// 			}
// 			
// 			int c = atoi(unitVertexList[2].c_str());
// 			//a, b, c (for each vertex)are int are indexes for vector of vertexPosition, vertexUVs, vertexNormals
// 			//so add those in to the 
// 
// 			for (int vertexIndex = 0; vertexIndex < (int)unitVertexList.size(); vertexIndex++) 
// 			{
// 
// 				
// 				//unitVertexList[vertexIndex]
// 
// 			}
// 
// 
// 		}
// 		
// 	}




}

Vertex_PNCU OBJLoader::MakePNCU(Vec3 const& position, Vec3 const& normal /*= Vec3::WORLD_ORIGIN*/, Vec2 const& uv/* = Vec2::ZERO*/, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vertex_PNCU result = Vertex_PNCU(position, normal, color, uv);
	return result;
}

void OBJLoader::AddPNCUFromTrianglesWithoutVN(std::vector<Vertex_PNCU>& vertexes, Triangle const& triangle, std::vector<Vec3>const& vertexPosi)
{
	Vec3 normal = triangle.n;

	int indexA = triangle.v[0];
	Vec3 positionA = vertexPosi[indexA];
	int indexB = triangle.v[1];
	Vec3 positionB = vertexPosi[indexB];
	int indexC = triangle.v[2];
	Vec3 positionC = vertexPosi[indexC];

	Vertex_PNCU verA = MakePNCU(positionA, normal);
	Vertex_PNCU verB = MakePNCU(positionB, normal);
	Vertex_PNCU verC = MakePNCU(positionC, normal);

	vertexes.push_back(verA);
	vertexes.push_back(verB);
	vertexes.push_back(verC);
}



void OBJLoader::AddPNCUFromTrianglesWithVN(std::vector<Vertex_PNCU>& vertexes, Triangle const& triangle, std::vector<Vec3>const& vertexPosi, std::vector<Vec3>const& vertexNormals)
{
	int normalIndex = triangle.nIndex;
	Vec3 normal = vertexNormals[normalIndex];

	int indexA = triangle.v[0];
	Vec3 positionA = vertexPosi[indexA];
	int indexB = triangle.v[1];
	Vec3 positionB = vertexPosi[indexB];
	int indexC = triangle.v[2];
	Vec3 positionC = vertexPosi[indexC];

	Vertex_PNCU verA = MakePNCU(positionA, normal);
	Vertex_PNCU verB = MakePNCU(positionB, normal);
	Vertex_PNCU verC = MakePNCU(positionC, normal);

	vertexes.push_back(verA);
	vertexes.push_back(verB);
	vertexes.push_back(verC);
}

void OBJLoader::DividePolygonToTriangles(std::vector<int> polygon, std::vector<Triangle>& triangles)
{
	GUARANTEE_OR_DIE(((int)polygon.size() > 3), "Wrong Polygon");
	for (int vertIndex = 1; vertIndex < (int)polygon.size() - 1; vertIndex++)
	{
		Triangle tri(polygon[0], polygon[vertIndex], polygon[vertIndex + 1]);
		triangles.push_back(tri);
	}
}

void OBJLoader::AddNormalsToTriangles(std::vector<Vec3>const& vertexPosi, std::vector<Triangle>& triangles)
{
	for (int triIndex = 0; triIndex < (int)triangles.size(); triIndex++) 
	{
		int triangleIndexA = triangles[triIndex].v[0];
		int triangleIndexB = triangles[triIndex].v[1];
		int triangleIndexC = triangles[triIndex].v[2];
		Vec3 pointA = vertexPosi[triangleIndexA];
		Vec3 pointB = vertexPosi[triangleIndexB];
		Vec3 pointC = vertexPosi[triangleIndexC];

		Vec3 aToB = pointB - pointA;
		Vec3 bToC = pointC - pointB;

		Vec3 normalABC = CrossProduct3D(aToB, bToC);

		triangles[triIndex].n = normalABC;
	}
}

void OBJLoader::AddNormalsToATriangle(std::vector<Vec3>const& vertexPosi, Triangle & triangle)
{
	int triangleIndexA = triangle.v[0];
	int triangleIndexB = triangle.v[1];
	int triangleIndexC = triangle.v[2];
	Vec3 pointA = vertexPosi[triangleIndexA];
	Vec3 pointB = vertexPosi[triangleIndexB];
	Vec3 pointC = vertexPosi[triangleIndexC];

	Vec3 aToB = pointB - pointA;
	Vec3 bToC = pointC - pointB;

	Vec3 normalABC = CrossProduct3D(aToB, bToC);

	triangle.n = normalABC;
}

Vec3 OBJLoader::GetNormalsFromATriangle(std::vector<Vec3>const& vertexPosi, Triangle const& triangle)
{
	int triangleIndexA = triangle.v[0];
	int triangleIndexB = triangle.v[1];
	int triangleIndexC = triangle.v[2];
	Vec3 pointA = vertexPosi[triangleIndexA];
	Vec3 pointB = vertexPosi[triangleIndexB];
	Vec3 pointC = vertexPosi[triangleIndexC];

	Vec3 aToB = pointB - pointA;
	Vec3 bToC = pointC - pointB;

	Vec3 normalABC = CrossProduct3D(aToB, bToC);

	return normalABC;
}

bool OBJLoader::LoadOBJFilePNCU(std::string const& name, Mat44 const& transformFixupMatrix, std::vector<Vertex_PNCU>& vertexes, std::vector<unsigned int>& indexes)
{
	//Load file to string
	std::string bufferString;
	int is_ok = FileReadToString(bufferString, name);
	if (is_ok == EXIT_FAILURE) 
	{
		ERROR_AND_DIE("file loading failed");
		return false;
	}

	float timeNow = static_cast<float>(GetCurrentTimeSeconds());


	//Split with \r\n or just \r
	//Split with \n
	std::vector<std::string> linesBuffer;


	size_t found = bufferString.find("\n\r");
	if (found == bufferString.npos) 
	{
		linesBuffer = SplitStringOnDelimiter(bufferString, '\n');
	}
	else
	{
		linesBuffer = SplitStringOnDelimiter(bufferString, "\r\n");
	}

	//Create temp data struct to save buffer data
	

	float timeBeforeParse = static_cast<float>(GetCurrentTimeSeconds());
	float parseTime = timeBeforeParse - timeNow;
	float timeAfterParse = static_cast<float>(GetCurrentTimeSeconds());


	std::vector<Vec3> vertexPosition;
	std::vector<Vec2> vertexUVs;
	std::vector<Vec3> vertexNormals;
	int meshTriangleNum = 0;
	int meshFaceNum = 0;
	//std::vector<Triangle> triangle;
	//std::vector<Face> faces;

	//look at first 2 elements to determine elements
	for (int linesIndex = 0; linesIndex < (int)linesBuffer.size(); linesIndex++) 
	{
		std::string currentLine = linesBuffer[linesIndex];
		if (currentLine[0] == 'v') 
		{
			if (currentLine[1] == ' ') 
			{
				//vertex
				//get a line of string, and divide them in to 3 float numbers
				//save them to a vector
				GetAFloatVec3FromAStrings(currentLine, vertexPosition);
			}
			else if(currentLine[1] == 't')
			{
				//textual
				GetAFloatVec2FromAStrings(currentLine, vertexUVs);
			}
			else if (currentLine[1] == 'n')
			{
				//vertex normal
				GetAFloatVec3NormalizedFromAStrings(currentLine, vertexNormals);
			}

		}
		else if(currentLine[0]=='f' && currentLine[1]==' ')
		{
			//face
			GetIndexedVertexesFromAStrings(currentLine, vertexPosition, vertexNormals, vertexUVs, vertexes, indexes);
			meshFaceNum++;
		}




	}
	//Only v elements, so v = face
	if (vertexes.empty()&& vertexUVs.empty() && vertexNormals.empty()) 
	{
		int normalSize = (int)vertexPosition.size() / 3;
		for (int normalIndex = 0; normalIndex < normalSize; normalIndex++) \
		{
			int indexA = normalIndex * 3 + 0;
			int indexB = normalIndex * 3 + 1;
			int indexC = normalIndex * 3 + 2;

			Vec3 pointA = vertexPosition[indexA];
			Vec3 pointB = vertexPosition[indexB];
			Vec3 pointC = vertexPosition[indexC];

			Vec3 aToB = pointB - pointA;
			Vec3 bToC = pointC - pointB;

			Vec3 normalABC = CrossProduct3D(aToB, bToC);

			Vertex_PNCU pncuA = MakePNCU(pointA, normalABC);
			Vertex_PNCU pncuB = MakePNCU(pointB, normalABC);
			Vertex_PNCU pncuC = MakePNCU(pointC, normalABC);


			indexes.push_back(indexA);
			indexes.push_back(indexB);
			indexes.push_back(indexC);

			vertexes.push_back(pncuA);
			vertexes.push_back(pncuB);
			vertexes.push_back(pncuC);
		}
	}

	float timeAfterLoad = static_cast<float>(GetCurrentTimeSeconds());
	float createTime = timeAfterLoad - timeAfterParse;

	TransformVertexArray3D(vertexes, transformFixupMatrix);


	//Print statistics

	int vertexsNum = (int)vertexPosition.size();
	int texNum = (int)vertexUVs.size();
	int vertNormalNum = (int)vertexNormals.size();

	int meshVertsNum = (int)vertexes.size();
	int meshIndexNum = (int)indexes.size();

	meshTriangleNum = meshIndexNum / 3;

	DebuggerPrintf(" -----------------------------------------------------------------------------------------\n");
	DebuggerPrintf("Loaded obj. file %s \n", name.c_str());
	DebuggerPrintf("    [file data]    vertexes: %d texture coordinates: %d normals: %d faces: %d triangles: %d  \n", vertexsNum, texNum, vertNormalNum, meshFaceNum, meshTriangleNum);
	DebuggerPrintf("    [loaded mesh]  vertexes: %d indexes: %d\n", meshVertsNum, meshIndexNum);
	DebuggerPrintf("    [time]	       parse: %f seconds   create:  %f seconds\n", parseTime, createTime);

	DebuggerPrintf(" -----------------------------------------------------------------------------------------\n");
	return true;
}

bool OBJLoader::LoadOBJFile(std::string const& name, Mat44 const& transformFixupMatrix, std::vector<Vertex_PCUTBN>& vertexes, std::vector<unsigned int>& indexes)
{
	std::vector<Vertex_PNCU> pncuList;
	//std::vector<int> indexeList;
	bool result = LoadOBJFilePNCU(name, transformFixupMatrix, pncuList, indexes);
	for (int i = 0; i < (int)pncuList.size(); i++) 
	{
		Vertex_PCUTBN vertex(pncuList[i]);
		vertexes.push_back(vertex);
	}
	return result;
}


Vertex_PCUTBN OBJLoader::MakeEmptyPCUTBN(Vec3 const& position, Vec3 const& normal /*= Vec3::WORLD_ORIGIN*/, Vec2 const& uv /*= Vec2::ZERO*/, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vertex_PNCU pncu = Vertex_PNCU(position, normal, color, uv);
	Vertex_PCUTBN result = Vertex_PCUTBN(pncu);
	return result;
}
