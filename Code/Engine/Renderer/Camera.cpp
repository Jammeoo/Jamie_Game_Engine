#include <cmath>
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near, float far)
{
	m_mode = eMode_Orthographic;
	m_orthographicBottomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = near;
	m_orthographicFar = far;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_orthographicBottomLeft;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_orthographicTopRight;
}

void Camera::TranslateOrtho2D(const Vec2& translation2D) {
	m_orthographicBottomLeft += translation2D;
	m_orthographicTopRight += translation2D;
}

void Camera::TranslatePosiZY2D(const Vec2& translation2D)
{
	m_position.y += translation2D.x;
	m_position.z += translation2D.y;
}

void Camera::SetPosiZY(const Vec2& newPosi)
{
	m_position.y = newPosi.x;
	m_position.z = newPosi.y;
}

void Camera::SetOrthographicView(Vec2 const& bottomLeft, Vec2 const& topRight, float near /*= 0.0f*/, float far /*= 1.0f*/)
{
	m_mode = eMode_Orthographic;
	m_orthographicBottomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = near;
	m_orthographicFar = far;
}

void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	m_mode = eMode_Perspective;
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = near;
	m_perspectiveFar = far;
}

Vec2 Camera::GetOrthographicBottomLeft() const
{
	return m_orthographicBottomLeft;
}

Vec2 Camera::GetOrthographicTopRight() const
{
	return m_orthographicTopRight;
}

void Camera::TranslateOrthographic2D(Vec2 const& translation)
{
	m_orthographicBottomLeft += translation;
	m_orthographicTopRight += translation;
}

Mat44 Camera::GetOrthographicMatrix() const
{
	Mat44 mat44 = Mat44::CreateOrthoProjection(m_orthographicBottomLeft.x, m_orthographicTopRight.x, m_orthographicBottomLeft.y, m_orthographicTopRight.y, m_orthographicNear, m_orthographicFar);
	return mat44;
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	Mat44 mat44 = Mat44::CreatePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
	return mat44;
}

Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 mat44;
	if (m_mode == eMode_Orthographic) 
	{
		mat44 = GetOrthographicMatrix();
	}
	else
	{
		mat44 = GetPerspectiveMatrix();
	}
	mat44.Append(GetRenderMatrix());
	return mat44;
}

void Camera::SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis; 
	m_renderKBasis = kBasis;
}

Mat44 Camera::GetRenderMatrix() const
{
	Mat44 mat44 = Mat44();
	mat44.SetIJK3D(m_renderIBasis, m_renderJBasis, m_renderKBasis);
	return mat44;
}

void Camera::SetTransform(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

Mat44 Camera::GetViewMatrix() const
{
	Mat44 mat44 = Mat44();
	mat44 = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	mat44.SetTranslation3D(m_position);
	Mat44 inversedMat44 = Mat44();
	inversedMat44 = mat44.GetOrthonormalInverse();
	return inversedMat44;
}

Vec3 Camera::GetWorldMouseRayNormalized(Vec2 const& cursorPosInScreenSpace, Vec2 const& screenDims) const
{

	Vec3 cameraForward = Vec3::WORLD_ORIGIN;
	Vec3 cameraLeft = Vec3::WORLD_ORIGIN;
	Vec3 cameraUp = Vec3::WORLD_ORIGIN;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(cameraForward, cameraLeft, cameraUp);
	Vec2 cursorPosi = cursorPosInScreenSpace;
	
	//From -1 to 1
	float deltaX = RangeMap(cursorPosi.x, 0.f, screenDims.x, -1.f, 1.f);
	float deltaY = RangeMap(cursorPosi.y, 0.f, screenDims.y, -1.f, 1.f);
	
	Vec3 aDisp = m_perspectiveNear * cameraForward;
	Vec3 bDisp = (-deltaX) * m_perspectiveAspect * m_perspectiveNear * TanDegrees(0.5f * m_perspectiveFOV) * cameraLeft;
	Vec3 cDisp = (-deltaY) * m_perspectiveNear * TanDegrees(0.5f * m_perspectiveFOV) * cameraUp;

	Vec3 result = (aDisp + bDisp + cDisp).GetNormalized();
	return result;
		// ScreenSpace
// 	Vec2  screenSpaceCenter = Vec2(screenDims.x * 0.5f, screenDims.y * 0.5f);
// 	float cursorScreenSpaceDispAlongX = cursorPosInScreenSpace.x - screenSpaceCenter.x;
// 	float cursorScreenSpaceDispAlongY = cursorPosInScreenSpace.y - screenSpaceCenter.y;
// 	float cursorDispAlongXRatio = cursorScreenSpaceDispAlongX / screenDims.x;
// 	float cursorDispAlongYRatio = cursorScreenSpaceDispAlongY / screenDims.y;
// 
// 	// ViewSpace
// 	float screenAspectRatio = screenDims.x / screenDims.y;
// 	float halfFOVDegrees = m_perspectiveFOV * 0.5f;
// 	float viewSpaceHalfHeight = m_perspectiveNear * TanDegrees(halfFOVDegrees);
// 	float viewSpaceHeight = viewSpaceHalfHeight * 2.f;
// 	float viewSpaceWidth = viewSpaceHeight * screenAspectRatio;
// 	Vec3  viewSpaceCenter = Vec3(m_perspectiveNear, 0.f, 0.f);
// 	float cursorViewDispAlongX = cursorDispAlongXRatio * viewSpaceWidth;
// 	float cursorViewDispAlongY = cursorDispAlongYRatio * viewSpaceHeight;
// 
// 	//Mat44 camModelMatrix = GetModelMatrix();
// 	Vec3 camForward;
// 	Vec3 camLeft;
// 	Vec3 camUp;
// 	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(camForward, camLeft, camUp);
// 
// 	Vec3 worldSpaceVecFromCamToViewCenter = viewSpaceCenter.x * camForward;
// 	// Negate the y and z, as directX is +x going right and +y going down, and in viewSpace +y is going left and +z is going up
// 	Vec3 worldSpaceVecFromViewCenterToCursorDispX = -1.f * (cursorViewDispAlongX * camLeft);
// 	Vec3 worldSpaceVecFromCursorDispXToCursorDispY = -1.f * (cursorViewDispAlongY * camUp);
// 
// 	Vec3 worldSpaceCursorPos = m_position + worldSpaceVecFromCamToViewCenter + worldSpaceVecFromViewCenterToCursorDispX + worldSpaceVecFromCursorDispXToCursorDispY;
// 	Vec3 dispFromCamToWorldCursorPos = worldSpaceCursorPos - m_position;
// 	Vec3 dirFromCamToWorldCursorPos = dispFromCamToWorldCursorPos.GetNormalized();
// 
// 	return dirFromCamToWorldCursorPos;
}

Vec3 Camera::GetWorldScreenCenterNormalized() const
{
	Vec3 cameraForward = Vec3::WORLD_ORIGIN;
	Vec3 cameraLeft = Vec3::WORLD_ORIGIN;
	Vec3 cameraUp = Vec3::WORLD_ORIGIN;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(cameraForward, cameraLeft, cameraUp);

	Vec3 aDisp = m_perspectiveNear * cameraForward;
	Vec3 result = aDisp.GetNormalized();
	return result;
}

