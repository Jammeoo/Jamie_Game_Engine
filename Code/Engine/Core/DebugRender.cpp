#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/VertexUtils.hpp"

std::string s_fontFilePath = "Data/Images/SquirrelFixedFont";

DebugRenderConfig g_theDebugConfig;
BitmapFont* s_font = nullptr;
bool g_isVisible = true;
std::vector<Vertex_PCU> g_basisesVerts;
std::vector<Vertex_PCU> g_worldTextVerts;

struct DebugRenderAlways
{
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	StopWatch* m_alwaysStopWatch = nullptr;
	std::vector<Vertex_PCU> m_vertex;
};
struct DebugRenderDepth
{
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	StopWatch* m_depthStopWatch = nullptr;
	std::vector<Vertex_PCU> m_vertex;
};
struct DebugRenderDepthText
{
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	StopWatch* m_depthStopWatch = nullptr;
	Vec3 m_orign = Vec3::WORLD_ORIGIN;
	std::vector<Vertex_PCU> m_vertex;
};
struct DebugRenderScreenText
{
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	//float m_duration = 0.f;
	StopWatch* m_textStopwatch = nullptr;
	std::vector<Vertex_PCU> m_screenTextVertexes;
};


struct DebugRenderXRay
{
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	StopWatch* m_xRayStopWatch = nullptr;
	std::vector<Vertex_PCU> m_vertex;
};
std::vector<DebugRenderDepth> g_theDepthRenderVerts;								
std::vector<DebugRenderDepth> g_theDepthRenderWireframeVerts;					
std::vector<DebugRenderDepthText> g_theDepthRenderTextVerts;
std::vector< DebugRenderScreenText> g_theDepthRenderScreenTextVerts;
std::vector<DebugRenderAlways> g_theAlwaysRenderVerts;							
std::vector<DebugRenderXRay> g_theXRayRenderVerts;							
/// <summary>
/// Second way to store entities into a list
/// </summary>
// class DebugRenderObject
// {
// public:
// 	void Update();
// 	void Render(Camera camera, Renderer renderer);
// protected:
// private:
// 	Rgba8 m_startColor = Rgba8::WHITE;
// 	Rgba8 m_endColor = Rgba8::WHITE;
// 	StopWatch* m_stopWatch = nullptr;
// 	std::vector<Vertex_PCU> m_vertex;
// 	Vec3 m_position = Vec3::WORLD_ORIGIN;
// 	Vec3 
// };
// std::vector<DebugRenderObject*> g_worldObjects;
// std::vector<DebugRenderObject*> g_screenObjects;
// 
// void DebugRenderObject::Update()
// {
// 	//Update color;
// }
// 
// void DebugRenderObject::Render(Camera camera,Renderer renderer)
// {
// 	//set states && model constants
// 	//renderer->DrawVertxArray();
// }
void AddWorldBasisAndText()
{
	std::vector <Vertex_PCU> xTextVerts;
	s_font->AddVertsForFreeText3D(xTextVerts, Vec2(0.f, 0.f), 0.5f, " x - forward", Rgba8::RED, 0.8f, Vec2(0.f, 1.f));
	Vec3 textStart = Vec3(0.f, 0.f, 0.5f);

	Mat44 mat44;
	mat44.SetIJK3D(Vec3::X_AXIS, Vec3::Z_AXIS, -Vec3::Y_AXIS);
	mat44.SetTranslation3D(textStart);
	TransformVertexArray3D(xTextVerts, mat44);

	for (int vertsIndex = 0; vertsIndex < (int)xTextVerts.size(); vertsIndex++)
	{
		g_worldTextVerts.push_back(xTextVerts[vertsIndex]);
	}
	AddVertsForWorldArrow3D(g_basisesVerts, Vec3::WORLD_ORIGIN, 3.f * Vec3::X_AXIS, 0.7f, Rgba8::RED);

	std::vector <Vertex_PCU> yTextVerts;
	s_font->AddVertsForFreeText3D(yTextVerts, Vec2(0.f, 0.f), 0.5f, " y - left", Rgba8::GREEN, 0.8f, Vec2(0.f, 1.f));
	mat44.SetIJK3D(-Vec3::Y_AXIS, Vec3::Z_AXIS, -Vec3::X_AXIS);
	mat44.SetTranslation3D(textStart + Vec3(0.f, 4.f, 0.f));
	TransformVertexArray3D(yTextVerts, mat44);
	for (int vertsIndex = 0; vertsIndex < (int)yTextVerts.size(); vertsIndex++)
	{
		g_worldTextVerts.push_back(yTextVerts[vertsIndex]);
	}

	AddVertsForWorldArrow3D(g_basisesVerts, Vec3::WORLD_ORIGIN, 3.f * Vec3::Y_AXIS, 0.7f, Rgba8::GREEN);
	std::vector <Vertex_PCU> zTextVerts;
	s_font->AddVertsForFreeText3D(zTextVerts, Vec2(0.f, 0.f), 0.5f, "z - up", Rgba8::BLUE, 0.8f, Vec2(0.f, 1.f));
	mat44.SetIJK3D(Vec3::Z_AXIS, Vec3::Y_AXIS, -Vec3::X_AXIS);
	mat44.SetTranslation3D(textStart + Vec3(0.f, -1.3f, 0.f));
	TransformVertexArray3D(zTextVerts, mat44);
	for (int vertsIndex = 0; vertsIndex < (int)zTextVerts.size(); vertsIndex++)
	{
		g_worldTextVerts.push_back(zTextVerts[vertsIndex]);
	}

	AddVertsForWorldArrow3D(g_basisesVerts, Vec3::WORLD_ORIGIN, 3.f * Vec3::Z_AXIS, 0.7f, Rgba8::BLUE);

}
void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	g_theDebugConfig.m_renderer = config.m_renderer;
	g_theDebugConfig.m_startHidden = config.m_startHidden;
	g_isVisible = true;
	s_font = g_theDebugConfig.m_renderer->CreateOrGetBitmapFont(s_fontFilePath.c_str());
	AddWorldBasisAndText();
}

void DebugRenderSystemShutDown()
{
	DebugRenderClear();
}

void DebugRenderSetVisible()
{
	if (g_isVisible) 
	{
		g_isVisible = false;
	}
	else
	{
		g_isVisible = true;
	}
}

void DebugRenderSetHidden()
{
	if (g_theDebugConfig.m_startHidden)
	{
		g_theDebugConfig.m_startHidden = false;
	}
	else
	{
		g_theDebugConfig.m_startHidden = true;
	}

}

void DebugRenderClear()
{
	g_theDepthRenderWireframeVerts.clear();
	g_theDepthRenderVerts.clear();
	g_theAlwaysRenderVerts.clear();
	g_theXRayRenderVerts.clear();
	g_theDepthRenderTextVerts.clear();
	g_theDepthRenderScreenTextVerts.clear();
}

void ColorLerpOfAlwaysVertex(std::vector<Vertex_PCU>& verts)
{
	for (int vertsIndex = 0; vertsIndex < (int)g_theAlwaysRenderVerts.size(); vertsIndex++) 
	{
		DebugRenderAlways dRAlways = g_theAlwaysRenderVerts[vertsIndex];
		float colorFactor = dRAlways.m_alwaysStopWatch->GetElapsedFraction();
		Rgba8 color = Interpolate(dRAlways.m_startColor, dRAlways.m_endColor, colorFactor);

		for (int vertsEachIndex = 0; vertsEachIndex < (int)dRAlways.m_vertex.size(); vertsEachIndex++) 
		{
			dRAlways.m_vertex[vertsEachIndex].m_color = color;
			verts.push_back(dRAlways.m_vertex[vertsEachIndex]);
		}
	}
}

// void ColorLerpOfDepthVertex(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderDepth>& debugDepthVector)
// {
// 	for (int vertsIndex = 0; vertsIndex < (int)debugDepthVector.size(); vertsIndex++)
// 	{
// 		DebugRenderDepth dRDepth = debugDepthVector[vertsIndex];
// 		float colorFactor = dRDepth.m_depthStopWatch->GetElapsedFraction();
// 		Rgba8 color = Interpolate(dRDepth.m_startColor, dRDepth.m_endColor, colorFactor);
// 
// 		for (int vertsEachIndex = 0; vertsEachIndex < (int)dRDepth.m_vertex.size(); vertsEachIndex++)
// 		{
// 			dRDepth.m_vertex[vertsEachIndex].m_color = color;
// 			verts.push_back(dRDepth.m_vertex[vertsEachIndex]);
// 		}
// 	}
// }

void RenderAndTimeLerpScreenTextVertexes(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderScreenText> textVecor)
{
	for (int textIndex = 0; textIndex < (int)textVecor.size(); ++textIndex)
	{
		DebugRenderScreenText& dRTextVerts = textVecor[textIndex];
		if (dRTextVerts.m_textStopwatch->m_duration == 0.f)
		{
			for (int textVertexIndex = 0; textVertexIndex < dRTextVerts.m_screenTextVertexes.size(); ++textVertexIndex)
			{
				verts.push_back(dRTextVerts.m_screenTextVertexes[textVertexIndex]);
			}
			continue;
		}
		float interpolatorFactor = dRTextVerts.m_textStopwatch->GetElapsedFraction();
		Rgba8 interpolatedColor = Interpolate(dRTextVerts.m_startColor, dRTextVerts.m_endColor, interpolatorFactor);
		for (int textVertexIndex = 0; textVertexIndex < dRTextVerts.m_screenTextVertexes.size(); ++textVertexIndex)
		{
			dRTextVerts.m_screenTextVertexes[textVertexIndex].m_color = interpolatedColor;
			verts.push_back(dRTextVerts.m_screenTextVertexes[textVertexIndex]);
		}
	}
}
void ColorLerpOfDepthVertex(std::vector<Vertex_PCU>& verts, std::vector<DebugRenderDepth>& debugDepthVector)
{
	for (int vertsIndex = 0; vertsIndex < (int)debugDepthVector.size(); vertsIndex++)
	{
		DebugRenderDepth dRDepth = debugDepthVector[vertsIndex];
		float colorFactor = dRDepth.m_depthStopWatch->GetElapsedFraction();
		Rgba8 color = Interpolate(dRDepth.m_startColor, dRDepth.m_endColor, colorFactor);

		for (int vertsEachIndex = 0; vertsEachIndex < (int)dRDepth.m_vertex.size(); vertsEachIndex++)
		{
			dRDepth.m_vertex[vertsEachIndex].m_color = color;
			verts.push_back(dRDepth.m_vertex[vertsEachIndex]);
		}
	}
}

void RenderWorldBasises() 
{
	g_theDebugConfig.m_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_theDebugConfig.m_renderer->SetModelConstants();
	g_theDebugConfig.m_renderer->BindTexture(nullptr);
	g_theDebugConfig.m_renderer->BindShader(nullptr);

	g_theDebugConfig.m_renderer->DrawVertexArray((int)g_basisesVerts.size(), g_basisesVerts.data());
	g_theDebugConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
	g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	g_theDebugConfig.m_renderer->SetModelConstants();
	g_theDebugConfig.m_renderer->BindTexture(&s_font->GetTexture());
	g_theDebugConfig.m_renderer->DrawVertexArray((int)g_worldTextVerts.size(), g_worldTextVerts.data());
	g_theDebugConfig.m_renderer->BindTexture(nullptr);
}



void DrawAndColorLerpOfDepthTextVertex(Camera const& camera, std::vector<DebugRenderDepthText>& debugTextVector)
{
	for (int textIndex = 0; textIndex < (int)debugTextVector.size(); textIndex++)
	{
		DebugRenderDepthText dRDepth = debugTextVector[textIndex];
		float colorFactor = dRDepth.m_depthStopWatch->GetElapsedFraction();
		Rgba8 color = Interpolate(dRDepth.m_startColor, dRDepth.m_endColor, colorFactor);

		for (int vertsEachIndex = 0; vertsEachIndex < (int)dRDepth.m_vertex.size(); vertsEachIndex++)
		{
			dRDepth.m_vertex[vertsEachIndex].m_color = color;
		}

		Mat44 cameraMat = camera.m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		Mat44 billboardMat = GetBillboardMatrix(BillBoardType::FULL_CAMERA_OPPOSING, cameraMat, dRDepth.m_orign);
		g_theDebugConfig.m_renderer->SetModelConstants(billboardMat);
		g_theDebugConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
		g_theDebugConfig.m_renderer->BindShader(nullptr);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theDebugConfig.m_renderer->BindTexture(&s_font->GetTexture());
		g_theDebugConfig.m_renderer->DrawVertexArray((int)dRDepth.m_vertex.size(), dRDepth.m_vertex.data());
	}
}

void ColorLerpOfXRayAlphaVertex(std::vector<Vertex_PCU>& verts)
{
	for (int vertsIndex = 0; vertsIndex < (int)g_theXRayRenderVerts.size(); vertsIndex++)
	{
		DebugRenderXRay dRXRay = g_theXRayRenderVerts[vertsIndex];
		Rgba8 color = dRXRay.m_startColor;
		color.r += (unsigned char)50;
		color.g += (unsigned char)50;
		color.b += (unsigned char)50;
		color.a = (unsigned char)100;
		
		for (int vertsEachIndex = 0; vertsEachIndex < (int)dRXRay.m_vertex.size(); vertsEachIndex++)
		{
			dRXRay.m_vertex[vertsEachIndex].m_color = color;
			verts.push_back(dRXRay.m_vertex[vertsEachIndex]);
		}
	}
}


void ColorLerpOfXRayDepthVertex(std::vector<Vertex_PCU>& verts)
{
	for (int vertsIndex = 0; vertsIndex < (int)g_theXRayRenderVerts.size(); vertsIndex++)
	{
		DebugRenderXRay dRXRay = g_theXRayRenderVerts[vertsIndex];
		Rgba8 color = dRXRay.m_startColor;
		color.a = 255;
		for (int vertsEachIndex = 0; vertsEachIndex < (int)dRXRay.m_vertex.size(); vertsEachIndex++)
		{
			dRXRay.m_vertex[vertsEachIndex].m_color = color;
			verts.push_back(dRXRay.m_vertex[vertsEachIndex]);
		}
	}
}
void DebugRenderBeginFrame()
{
	for (int depthIndex = 0; depthIndex < (int)g_theDepthRenderVerts.size(); depthIndex++) {
		DebugRenderDepth& currentDepthShape = g_theDepthRenderVerts[depthIndex];
		if (currentDepthShape.m_depthStopWatch->HasDurationElapsed()) 
		{
			DebugRenderDepth bottomShape = g_theDepthRenderVerts[(int)g_theDepthRenderVerts.size() - 1];
			g_theDepthRenderVerts[(int)g_theDepthRenderVerts.size() - 1] = currentDepthShape;
			currentDepthShape = bottomShape;
			g_theDepthRenderVerts.pop_back();
		}

		if (currentDepthShape.m_depthStopWatch->IsStopped())
		{
			currentDepthShape.m_depthStopWatch->Start();
		}

	}
	for (int depthIndex = 0; depthIndex < (int)g_theDepthRenderScreenTextVerts.size(); depthIndex++) {
		DebugRenderScreenText& currentDepthScreenText = g_theDepthRenderScreenTextVerts[depthIndex];
		if (currentDepthScreenText.m_textStopwatch->HasDurationElapsed())
		{
			DebugRenderScreenText bottomText = g_theDepthRenderScreenTextVerts[(int)g_theDepthRenderScreenTextVerts.size() - 1];
			g_theDepthRenderScreenTextVerts[(int)g_theDepthRenderScreenTextVerts.size() - 1] = currentDepthScreenText;
			currentDepthScreenText = bottomText;
			g_theDepthRenderScreenTextVerts.pop_back();
		}

		if (currentDepthScreenText.m_textStopwatch->IsStopped())
		{
			currentDepthScreenText.m_textStopwatch->Start();
		}

	}
	for (int depthIndex = 0; depthIndex < (int)g_theDepthRenderWireframeVerts.size(); depthIndex++) {
		DebugRenderDepth& currentDepthShape = g_theDepthRenderWireframeVerts[depthIndex];

		if (currentDepthShape.m_depthStopWatch->HasDurationElapsed())
		{
			DebugRenderDepth bottomShape = g_theDepthRenderWireframeVerts[(int)g_theDepthRenderWireframeVerts.size() - 1];
			g_theDepthRenderWireframeVerts[(int)g_theDepthRenderWireframeVerts.size() - 1] = currentDepthShape;
			currentDepthShape = bottomShape;
			g_theDepthRenderWireframeVerts.pop_back();
		}

		if (currentDepthShape.m_depthStopWatch->IsStopped())
		{
			currentDepthShape.m_depthStopWatch->Start();
		}

	}
	for (int depthIndex = 0; depthIndex < (int)g_theDepthRenderTextVerts.size(); depthIndex++) {
		DebugRenderDepthText& currentDepthShape = g_theDepthRenderTextVerts[depthIndex];

		if (currentDepthShape.m_depthStopWatch->HasDurationElapsed())
		{
			DebugRenderDepthText bottomShape = g_theDepthRenderTextVerts[(int)g_theDepthRenderTextVerts.size() - 1];
			g_theDepthRenderTextVerts[(int)g_theDepthRenderTextVerts.size() - 1] = currentDepthShape;
			currentDepthShape = bottomShape;
			g_theDepthRenderTextVerts.pop_back();
		}

		if (currentDepthShape.m_depthStopWatch->IsStopped())
		{
			currentDepthShape.m_depthStopWatch->Start();
		}

	}
	

	for (int alwaysIndex = 0; alwaysIndex < (int)g_theAlwaysRenderVerts.size(); alwaysIndex++) {
		DebugRenderAlways& currentAlwaysShape = g_theAlwaysRenderVerts[alwaysIndex];
		if (currentAlwaysShape.m_alwaysStopWatch->HasDurationElapsed())
		{
			DebugRenderAlways bottomShape = g_theAlwaysRenderVerts[(int)g_theAlwaysRenderVerts.size() - 1];
			g_theAlwaysRenderVerts[(int)g_theAlwaysRenderVerts.size() - 1] = currentAlwaysShape;
			currentAlwaysShape = bottomShape;
			g_theAlwaysRenderVerts.pop_back();
		}

		if (currentAlwaysShape.m_alwaysStopWatch->IsStopped())
		{
			currentAlwaysShape.m_alwaysStopWatch->Start();
		}
	}

	for (int xRayIndex = 0; xRayIndex < (int)g_theXRayRenderVerts.size(); xRayIndex++) {
		DebugRenderXRay& currentXRayShape = g_theXRayRenderVerts[xRayIndex];
		if (currentXRayShape.m_xRayStopWatch->HasDurationElapsed())
		{
			DebugRenderXRay bottomShape = g_theXRayRenderVerts[(int)g_theXRayRenderVerts.size() - 1];
			g_theXRayRenderVerts[(int)g_theXRayRenderVerts.size() - 1] = currentXRayShape;
			currentXRayShape = bottomShape;
			g_theXRayRenderVerts.pop_back();
		}

		if (currentXRayShape.m_xRayStopWatch->IsStopped())
		{
			currentXRayShape.m_xRayStopWatch->Start();
		}
	}

}

void DebugRenderWorld(const Camera& camera)
{
	if (g_isVisible) 
	{

		g_theDebugConfig.m_renderer->BeginCamera(camera);


		std::vector<Vertex_PCU> alwaysVerts;
		ColorLerpOfAlwaysVertex(alwaysVerts);
		g_theDebugConfig.m_renderer->BindTexture(nullptr);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::DISABLED);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theDebugConfig.m_renderer->BindShader(nullptr);

		g_theDebugConfig.m_renderer->DrawVertexArray((int)alwaysVerts.size(), alwaysVerts.data());

		std::vector<Vertex_PCU> depthVerts;
		ColorLerpOfDepthVertex(depthVerts, g_theDepthRenderVerts);
		g_theDebugConfig.m_renderer->BindTexture(nullptr);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theDebugConfig.m_renderer->BindShader(nullptr);
		g_theDebugConfig.m_renderer->DrawVertexArray((int)depthVerts.size(), depthVerts.data());

		std::vector<Vertex_PCU> depthWireframeVerts;
		ColorLerpOfDepthVertex(depthWireframeVerts, g_theDepthRenderWireframeVerts);
		g_theDebugConfig.m_renderer->BindTexture(nullptr);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);
		g_theDebugConfig.m_renderer->BindShader(nullptr);
		g_theDebugConfig.m_renderer->DrawVertexArray((int)depthWireframeVerts.size(), depthWireframeVerts.data());

		DrawAndColorLerpOfDepthTextVertex(camera, g_theDepthRenderTextVerts);

		std::vector<Vertex_PCU> xRayAlphaVerts;
		ColorLerpOfXRayAlphaVertex(xRayAlphaVerts);
		g_theDebugConfig.m_renderer->BindTexture(nullptr);
		g_theDebugConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::DISABLED);
		g_theDebugConfig.m_renderer->BindShader(nullptr);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theDebugConfig.m_renderer->DrawVertexArray((int)xRayAlphaVerts.size(), xRayAlphaVerts.data());

		std::vector<Vertex_PCU> xRayDepthVerts;
		ColorLerpOfXRayDepthVertex(xRayDepthVerts);
		g_theDebugConfig.m_renderer->BindTexture(nullptr);
		g_theDebugConfig.m_renderer->SetBlendMode(BlendMode::OPAQUE);
		g_theDebugConfig.m_renderer->BindShader(nullptr);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theDebugConfig.m_renderer->DrawVertexArray((int)xRayDepthVerts.size(), xRayDepthVerts.data());

		//RenderWorldBasises();

		g_theDebugConfig.m_renderer->EndCamera(camera);
	}
}

void DebugRenderScreen(const Camera& camera)
{
	if (g_isVisible)
	{
		g_theDebugConfig.m_renderer->BeginCamera(camera);
		std::vector<Vertex_PCU> screenTextVertexes;
		RenderAndTimeLerpScreenTextVertexes(screenTextVertexes, g_theDepthRenderScreenTextVerts);
		g_theDebugConfig.m_renderer->SetBlendMode(BlendMode::ALPHA);
		g_theDebugConfig.m_renderer->SetDepthMode(DepthMode::ENABLED);
		g_theDebugConfig.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theDebugConfig.m_renderer->BindTexture(&s_font->GetTexture());
		g_theDebugConfig.m_renderer->DrawVertexArray((int)screenTextVertexes.size(), screenTextVertexes.data());
		g_theDebugConfig.m_renderer->BindTexture(nullptr);

		g_theDebugConfig.m_renderer->EndCamera(camera);
	}
}

void DebugRenderEndFrame()
{

}

void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		AddVertsForSphere3D(dRAlways.m_vertex, pos, radius, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		AddVertsForSphere3D(dRDepth.m_vertex, pos, radius, startColor);
		g_theDepthRenderVerts.push_back(dRDepth);															//The DepthRender
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		AddVertsForSphere3D(dRXRay.m_vertex, pos, radius, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}
	}

}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	float thickness = 0.5f * radius;
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		AddVertsForCylinder3D(dRAlways.m_vertex, start, end, thickness, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		AddVertsForCylinder3D(dRDepth.m_vertex, start, end, thickness, startColor);
		g_theDepthRenderVerts.push_back(dRDepth);
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		AddVertsForCylinder3D(dRXRay.m_vertex, start, end, thickness, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}
	}
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{

	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		AddVertsForCylinder3D(dRAlways.m_vertex, top, base, radius, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		AddVertsForCylinder3D(dRDepth.m_vertex, top, base, radius, startColor);
		g_theDepthRenderWireframeVerts.push_back(dRDepth);
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		AddVertsForCylinder3D(dRXRay.m_vertex, top, base, radius, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}
	}
}

void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{

	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		AddVertsForSphere3D(dRAlways.m_vertex, center, radius, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		AddVertsForSphere3D(dRDepth.m_vertex, center, radius, startColor);
		g_theDepthRenderWireframeVerts.push_back(dRDepth);

		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		AddVertsForSphere3D(dRXRay.m_vertex, center, radius, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}

	}
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{

	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		AddVertsForWorldArrow3D(dRAlways.m_vertex, start, end, radius, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		AddVertsForWorldArrow3D(dRDepth.m_vertex, start, end, radius, startColor);
		g_theDepthRenderVerts.push_back(dRDepth);
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		AddVertsForWorldArrow3D(dRXRay.m_vertex, start, end, radius, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}
	}
}

void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	UNUSED(alignment);
	UNUSED(transform);
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		s_font->AddVertsForText3D(dRAlways.m_vertex, Vec2(0.f, 0.f), textHeight, text, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepth dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		s_font->AddVertsForText3D(dRDepth.m_vertex, Vec2(0.f, 0.f), textHeight, text, startColor);
		g_theDepthRenderVerts.push_back(dRDepth);
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		s_font->AddVertsForText3D(dRXRay.m_vertex, Vec2(0.f, 0.f), textHeight, text, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}
	}

}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/, DebugRenderMode mode /*= DebugRenderMode::USE_DEPTH*/)
{
	switch (mode)
	{
	case DebugRenderMode::ALWAYS:
	{
		DebugRenderAlways dRAlways;
		dRAlways.m_startColor = startColor;
		dRAlways.m_endColor = endColor;
		dRAlways.m_alwaysStopWatch = new StopWatch(duration);
		s_font->AddVertsForText3D(dRAlways.m_vertex, Vec2(origin.x, origin.y), textHeight, text, startColor);
		g_theAlwaysRenderVerts.push_back(dRAlways);
		break;
	}
	case DebugRenderMode::USE_DEPTH:
	{
		DebugRenderDepthText dRDepth;
		dRDepth.m_startColor = startColor;
		dRDepth.m_endColor = endColor;
		dRDepth.m_depthStopWatch = new StopWatch(duration);
		dRDepth.m_orign = origin;
		s_font->AddVertsForText3D(dRDepth.m_vertex, Vec2(origin.x, origin.y), textHeight, text, startColor, 1.0f, alignment);
		g_theDepthRenderTextVerts.push_back(dRDepth);
		break;
	}
	case DebugRenderMode::X_RAY:
	{
		DebugRenderXRay dRXRay;
		dRXRay.m_startColor = startColor;
		dRXRay.m_endColor = endColor;
		dRXRay.m_xRayStopWatch = new StopWatch(duration);
		s_font->AddVertsForText3D(dRXRay.m_vertex, Vec2(origin.x, origin.y), textHeight, text, startColor);
		g_theXRayRenderVerts.push_back(dRXRay);
		break;
	}
	}
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/)
{
	DebugRenderScreenText dRScreentext;
	dRScreentext.m_startColor = startColor;
	dRScreentext.m_endColor = endColor;
	dRScreentext.m_textStopwatch = new StopWatch(duration);
	s_font->AddVertsForTextInBox2D(dRScreentext.m_screenTextVertexes, AABB2(position.x, position.y, 1600.f, 800.f), size, text, startColor, 0.8f, alignment, TextDrawMode::OVERRUN, 9999999);
	g_theDepthRenderScreenTextVerts.push_back(dRScreentext);
}

void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor /*= Rgba8::WHITE*/, const Rgba8& endColor /*= Rgba8::WHITE*/)
{
	UNUSED(text);
	UNUSED(duration);
	UNUSED(startColor);
	UNUSED(endColor);
}

bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return true;
}

bool Conmand_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);
	DebugRenderSetVisible();
	return true;
}
