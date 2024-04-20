#pragma once
#include"Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
struct Mat44;

class Camera {
public:
	enum Mode
	{
		eMode_Orthographic,
		eMode_Perspective,
		eMode_Count
	};
	//Old
	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f);
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	void TranslateOrtho2D(const Vec2& translation2D);
	void TranslatePosiZY2D(const Vec2& translation2D);
	void SetPosiZY(const Vec2& newPosi);
	//New
	void SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f);
	void SetPerspectiveView(float aspect, float fov, float near, float far);

	Vec2 GetOrthographicBottomLeft()const;
	Vec2 GetOrthographicTopRight()const;
	void TranslateOrthographic2D(Vec2 const& translation);

	Mat44 GetOrthographicMatrix()const;
	Mat44 GetPerspectiveMatrix()const;
	Mat44 GetProjectionMatrix()const;

	void SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	Mat44 GetRenderMatrix() const;

	void SetTransform(const Vec3& position, const EulerAngles& orientation);
	Mat44 GetViewMatrix()const;

	//Mouse cursor
	Vec3 GetWorldMouseRayNormalized(Vec2 const& cursorPosInScreenSpace, Vec2 const& screenDims) const;
	Vec3 GetWorldScreenCenterNormalized() const;
public:
	//new
	Mode m_mode = eMode_Orthographic;

	Vec2 m_orthographicBottomLeft;
	Vec2 m_orthographicTopRight;
	float m_orthographicNear;
	float m_orthographicFar;

	float m_perspectiveAspect;
	float m_perspectiveFOV;
	float m_perspectiveNear;
	float m_perspectiveFar;

	Vec3 m_renderIBasis = Vec3(1.f, 0.f, 0.f);
	Vec3 m_renderJBasis = Vec3(0.f, 1.f, 0.f);
	Vec3 m_renderKBasis = Vec3(0.f, 0.f, 1.f);

	Vec3 m_position;
	EulerAngles m_orientation;
};