#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#define UNUSED(x) (void)(x);
Mat44::Mat44()
{
	m_values[0] = 1.f;
	m_values[1] = 0.f;
	m_values[2] = 0.f;
	m_values[3] = 0.f;

	m_values[4] = 0.f;
	m_values[5] = 1.f;
	m_values[6] = 0.f;
	m_values[7] = 0.f;

	m_values[8] = 0.f;
	m_values[9] = 0.f;
	m_values[10] = 1.f;
	m_values[11] = 0.f;

	m_values[12] = 0.f;
	m_values[13] = 0.f;
	m_values[14] = 0.f;
	m_values[15] = 1.f;

}

Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	m_values[0] = iBasis2D.x;
	m_values[1] = iBasis2D.y;
	m_values[2] = 0.f;
	m_values[3] = 0.f;

	m_values[4] = jBasis2D.x;
	m_values[5] = jBasis2D.y;
	m_values[6] = 0.f;
	m_values[7] = 0.f;

	m_values[8] = 0.f;
	m_values[9] = 0.f;
	m_values[10] = 1.f;
	m_values[11] = 0.f;

	m_values[12] = translation2D.x;
	m_values[13] = translation2D.y;
	m_values[14] = 0.f;
	m_values[15] = 1.f;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	m_values[0] = iBasis3D.x;
	m_values[1] = iBasis3D.y;
	m_values[2] = iBasis3D.z;
	m_values[3] = 0.f;

	m_values[4] = jBasis3D.x;
	m_values[5] = jBasis3D.y;
	m_values[6] = jBasis3D.z;
	m_values[7] = 0.f;

	m_values[8] = kBasis3D.x;
	m_values[9] = kBasis3D.y;
	m_values[10] = kBasis3D.z;
	m_values[11] = 0.f;

	m_values[12] = translation3D.x;
	m_values[13] = translation3D.y;
	m_values[14] = translation3D.z;
	m_values[15] = 1.f;
}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[0] = iBasis4D.x;
	m_values[1] = iBasis4D.y;
	m_values[2] = iBasis4D.z;
	m_values[3] = iBasis4D.w;

	m_values[4] = jBasis4D.x;
	m_values[5] = jBasis4D.y;
	m_values[6] = jBasis4D.z;
	m_values[7] = jBasis4D.w;

	m_values[8] = kBasis4D.x;
	m_values[9] = kBasis4D.y;
	m_values[10] = kBasis4D.z;
	m_values[11] = kBasis4D.w;

	m_values[12] = translation4D.x;
	m_values[13] = translation4D.y;
	m_values[14] = translation4D.z;
	m_values[15] = translation4D.w;
}

Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	for (int i = 0; i < 16; i++) 
	{
		m_values[i] = sixteenValuesBasisMajor[i];
	}
}

Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	Mat44 mat44;
	mat44.m_values[12] = translationXY.x;
	mat44.m_values[13] = translationXY.y;
	return mat44;
}

Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[12] = translationXYZ.x;
	mat44.m_values[13] = translationXYZ.y;
	mat44.m_values[14] = translationXYZ.z;
	return mat44;
}

Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[0] = uniformScaleXY;
	mat44.m_values[5] = uniformScaleXY;
	return mat44;
}

Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[0] = uniformScaleXYZ;
	mat44.m_values[5] = uniformScaleXYZ;
	mat44.m_values[10] = uniformScaleXYZ;
	return mat44;
}

Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[0] = nonUniformScaleXY.x;
	mat44.m_values[5] = nonUniformScaleXY.y;
	return mat44;
}

Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[0] = nonUniformScaleXYZ.x;
	mat44.m_values[5] = nonUniformScaleXYZ.y;
	mat44.m_values[10] = nonUniformScaleXYZ.z;
	return mat44;
}

Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[Ix] = CosDegrees(rotationDegreesAboutZ);
	mat44.m_values[Iy] = SinDegrees(rotationDegreesAboutZ);
	mat44.m_values[Jx] = -SinDegrees(rotationDegreesAboutZ);
	mat44.m_values[Jy] = CosDegrees(rotationDegreesAboutZ);
	return mat44;
}

Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[0] = CosDegrees(rotationDegreesAboutY);
	mat44.m_values[2] = -SinDegrees(rotationDegreesAboutY);
	mat44.m_values[8] = SinDegrees(rotationDegreesAboutY);
	mat44.m_values[10] = CosDegrees(rotationDegreesAboutY);
	return mat44;
}

Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	Mat44 mat44 = Mat44();
	mat44.m_values[5] = CosDegrees(rotationDegreesAboutX);
	mat44.m_values[6] = SinDegrees(rotationDegreesAboutX);
	mat44.m_values[9] = -SinDegrees(rotationDegreesAboutX);
	mat44.m_values[10] = CosDegrees(rotationDegreesAboutX);
	return mat44;
}

Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Mat44 mat44;
	mat44.m_values[Ix] = 2.f / (right - left);
	mat44.m_values[Tx] = (right + left) / (left - right);
	mat44.m_values[Jy] = 2.f / (top - bottom);
	mat44.m_values[Ty] = (top + bottom) / (bottom - top);
	mat44.m_values[Kz] = 1.f / (zFar - zNear);
	mat44.m_values[Tz] = zNear / (zNear - zFar);
	return mat44;
}

Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	Mat44 mat44;
	float c = CosDegrees(fovYDegrees * 0.5f);
	float s = SinDegrees(fovYDegrees * 0.5f);
	float scaleY = c / s;
	float scaleX = scaleY / aspect;
	float scaleZ = zFar / (zFar - zNear);
	float translateZ = (zNear * zFar) / (zNear - zFar);

	mat44.m_values[Ix] = scaleX;
	mat44.m_values[Jy] = scaleY;
	mat44.m_values[Kz] = scaleZ;
	mat44.m_values[Kw] = 1.f;				//Put z into the w component(in preparation for the hardware w-divide)
	mat44.m_values[Tz] = translateZ;
	mat44.m_values[Tw] = 0.f;				

	return mat44;
}

Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
 	Vec4 MatrixX = Vec4(m_values[0], m_values[4], m_values[8], m_values[12]);
 	Vec4 MatrixY = Vec4(m_values[1], m_values[5], m_values[9], m_values[13]);

	float resultX = MatrixX.x * vectorQuantityXY.x + MatrixX.y * vectorQuantityXY.y;
	float resultY = MatrixY.x * vectorQuantityXY.x + MatrixY.y * vectorQuantityXY.y;
	return Vec2(resultX, resultY);
}

Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	Vec4 MatrixX = Vec4(m_values[0], m_values[4], m_values[8], m_values[12]);
	Vec4 MatrixY = Vec4(m_values[1], m_values[5], m_values[9], m_values[13]);
	Vec4 MatrixZ = Vec4(m_values[2], m_values[6], m_values[10], m_values[14]);
	float resultX = MatrixX.x * vectorQuantityXYZ.x + MatrixX.y * vectorQuantityXYZ.y + MatrixX.z * vectorQuantityXYZ.z;
	float resultY = MatrixY.x * vectorQuantityXYZ.x + MatrixY.y * vectorQuantityXYZ.y + MatrixY.z * vectorQuantityXYZ.z;
	float resultZ = MatrixZ.x * vectorQuantityXYZ.x + MatrixZ.y * vectorQuantityXYZ.y + MatrixZ.z * vectorQuantityXYZ.z;
	return Vec3(resultX, resultY, resultZ);
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	Vec4 positionXYZW = Vec4(positionXY.x, positionXY.y, 0.f, 1.f);
	float resultX = positionXYZW.x * m_values[Ix] + positionXYZW.y * m_values[Jx] + positionXYZW.z * m_values[Kx] + positionXYZW.w * m_values[Tx];
	float resultY = positionXYZW.x * m_values[Iy] + positionXYZW.y * m_values[Jy] + positionXYZW.z * m_values[Ky] + positionXYZW.w * m_values[Ty];
	return Vec2(resultX, resultY);
}

Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
	Vec4 positionXYZW = Vec4(position3D.x, position3D.y, position3D.z, 1.f);
	float resultX = positionXYZW.x * m_values[Ix] + positionXYZW.y * m_values[Jx] + positionXYZW.z * m_values[Kx] + positionXYZW.w * m_values[Tx];
	float resultY = positionXYZW.x * m_values[Iy] + positionXYZW.y * m_values[Jy] + positionXYZW.z * m_values[Ky] + positionXYZW.w * m_values[Ty];
	float resultZ = positionXYZW.x * m_values[Iz] + positionXYZW.y * m_values[Jz] + positionXYZW.z * m_values[Kz] + positionXYZW.w * m_values[Tz];
	return Vec3(resultX, resultY, resultZ);
}

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	float resultX = homogeneousPoint3D.x * m_values[Ix] + homogeneousPoint3D.y * m_values[Jx] + homogeneousPoint3D.z * m_values[Kx] + homogeneousPoint3D.w * m_values[Tx];
	float resultY = homogeneousPoint3D.x * m_values[Iy] + homogeneousPoint3D.y * m_values[Jy] + homogeneousPoint3D.z * m_values[Ky] + homogeneousPoint3D.w * m_values[Ty];
	float resultZ = homogeneousPoint3D.x * m_values[Iz] + homogeneousPoint3D.y * m_values[Jz] + homogeneousPoint3D.z * m_values[Kz] + homogeneousPoint3D.w * m_values[Tz];
	float resultW = homogeneousPoint3D.x * m_values[Iw] + homogeneousPoint3D.y * m_values[Jw] + homogeneousPoint3D.z * m_values[Kw] + homogeneousPoint3D.w * m_values[Tw];

	return Vec4(resultX, resultY, resultZ, resultW);
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{

	return Vec2(m_values[Ix], m_values[Iy]);
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	//1. Isolate i,j,k Rotation Mat44
	Mat44 mat44PureTrans;
	Mat44 mat44PureRotation;
	for (int index = 0; index < 12; index++) 
	{
		mat44PureTrans.m_values[index] = m_values[index];
	}
	mat44PureTrans.m_values[Iw] = 0;
	mat44PureTrans.m_values[Jw] = 0;
	mat44PureTrans.m_values[Kw] = 0;

	mat44PureTrans.m_values[Tx] = 0;
	mat44PureTrans.m_values[Ty] = 0;
	mat44PureTrans.m_values[Tz] = 0;
	mat44PureTrans.m_values[Tw] = 1;
	//2.Transpose Trans Mat44
	mat44PureTrans.Transpose();
	//3. Pure Rotation Matrix
	
	mat44PureRotation.m_values[Tx] = -m_values[Tx];
	mat44PureRotation.m_values[Ty] = -m_values[Ty];
	mat44PureRotation.m_values[Tz] = -m_values[Tz];

	mat44PureTrans.Append(mat44PureRotation);

	return mat44PureTrans;
}
//Right Up forward i,j,k
void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;

	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;
}

void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.f;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.f;
	m_values[Jw] = 0.f;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.f;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

}

void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.f;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.f;
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat44::Transpose()
{
	Mat44 mat44Copy = Mat44();

	mat44Copy.m_values[Ix] = m_values[Ix];		//*this must be a Pure Rotation Mat44
	mat44Copy.m_values[Jx] = m_values[Iy];
	mat44Copy.m_values[Kx] = m_values[Iz];
	mat44Copy.m_values[Tx] = m_values[Iw];

	mat44Copy.m_values[Iy] = m_values[Jx];
	mat44Copy.m_values[Jy] = m_values[Jy];
	mat44Copy.m_values[Ky] = m_values[Jz];
	mat44Copy.m_values[Ty] = m_values[Jw];

	mat44Copy.m_values[Iz] = m_values[Kx];
	mat44Copy.m_values[Jz] = m_values[Ky];
	mat44Copy.m_values[Kz] = m_values[Kz];
	mat44Copy.m_values[Tz] = m_values[Kw];

	mat44Copy.m_values[Iw] = m_values[Tx];
	mat44Copy.m_values[Jw] = m_values[Ty];
	mat44Copy.m_values[Kw] = m_values[Tz];
	mat44Copy.m_values[Tw] = m_values[Tw];

	m_values[Ix] = mat44Copy.m_values[Ix];
	m_values[Iy] = mat44Copy.m_values[Iy];
	m_values[Iz] = mat44Copy.m_values[Iz];
	m_values[Iw] = mat44Copy.m_values[Iw];

	m_values[Jx] = mat44Copy.m_values[Jx];
	m_values[Jy] = mat44Copy.m_values[Jy];
	m_values[Jz] = mat44Copy.m_values[Jz];
	m_values[Jw] = mat44Copy.m_values[Jw];

	m_values[Kx] = mat44Copy.m_values[Kx];
	m_values[Ky] = mat44Copy.m_values[Ky];
	m_values[Kz] = mat44Copy.m_values[Kz];
	m_values[Kw] = mat44Copy.m_values[Kw];

	m_values[Tx] = mat44Copy.m_values[Tx];
	m_values[Ty] = mat44Copy.m_values[Ty];
	m_values[Tz] = mat44Copy.m_values[Tz];
	m_values[Tw] = mat44Copy.m_values[Tw];
}

void Mat44::Orthonormalize_XFwd_YLeft_ZUp()
{
	Mat44 mat44 = Mat44(m_values);
	
	//Normalize i
	Vec3 iBase = this->GetIBasis3D();
	Vec3 iBaseNormalized = iBase.GetNormalized();


	//Normalize k

	Vec3 kBase = this->GetKBasis3D();
	float kLengthOnI = DotProduct3D(kBase, iBaseNormalized);
	Vec3 kBaseOnIDIr = kLengthOnI * iBaseNormalized;
	Vec3 kBaseNormalized = (kBase - kBaseOnIDIr).GetNormalized();
	
	//Normalize j step 1, j perpendicular to i 
	Vec3 jBase = this->GetJBasis3D();
	float jLengthOnI = DotProduct3D(jBase, iBaseNormalized);
	Vec3 jBaseOnIDir = jLengthOnI * iBaseNormalized;
	Vec3 jBasePerpenToI = (jBase - jBaseOnIDir).GetNormalized();

	//Normalize j step 2,j perpendicular to k
	Vec3 jBasePerpenToIandOnK = DotProduct3D(jBasePerpenToI, kBaseNormalized) * kBaseNormalized;
	Vec3 jBasePerpenToIKNormalized = (jBasePerpenToI - jBasePerpenToIandOnK).GetNormalized();

	//Return I,J,K
	m_values[Ix] = iBaseNormalized.x;
	m_values[Iy] = iBaseNormalized.y;
	m_values[Iz] = iBaseNormalized.z;

	m_values[Jx] = jBasePerpenToIKNormalized.x;
	m_values[Jy] = jBasePerpenToIKNormalized.y;
	m_values[Jz] = jBasePerpenToIKNormalized.z;

	m_values[Kx] = kBaseNormalized.x;
	m_values[Ky] = kBaseNormalized.y;
	m_values[Kz] = kBaseNormalized.z;

}

void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 mat44 = *this;
	float* theLeft = mat44.m_values;
	float const* theRight = appendThis.m_values;

	m_values[Ix] = theLeft[Ix] * theRight[Ix] + theLeft[Jx] * theRight[Iy] + theLeft[Kx] * theRight[Iz] + theLeft[Tx] * theRight[Iw];
	m_values[Iy] = theLeft[Iy] * theRight[Ix] + theLeft[Jy] * theRight[Iy] + theLeft[Ky] * theRight[Iz] + theLeft[Ty] * theRight[Iw];
	m_values[Iz] = theLeft[Iz] * theRight[Ix] + theLeft[Jz] * theRight[Iy] + theLeft[Kz] * theRight[Iz] + theLeft[Tz] * theRight[Iw];
	m_values[Iw] = theLeft[Iw] * theRight[Ix] + theLeft[Jw] * theRight[Iy] + theLeft[Kw] * theRight[Iz] + theLeft[Tw] * theRight[Iw];

	m_values[Jx] = theLeft[Ix] * theRight[Jx] + theLeft[Jx] * theRight[Jy] + theLeft[Kx] * theRight[Jz] + theLeft[Tx] * theRight[Jw];
	m_values[Jy] = theLeft[Iy] * theRight[Jx] + theLeft[Jy] * theRight[Jy] + theLeft[Ky] * theRight[Jz] + theLeft[Ty] * theRight[Jw];
	m_values[Jz] = theLeft[Iz] * theRight[Jx] + theLeft[Jz] * theRight[Jy] + theLeft[Kz] * theRight[Jz] + theLeft[Tz] * theRight[Jw];
	m_values[Jw] = theLeft[Iw] * theRight[Jx] + theLeft[Jw] * theRight[Jy] + theLeft[Kw] * theRight[Jz] + theLeft[Tw] * theRight[Jw];

	m_values[Kx] = theLeft[Ix] * theRight[Kx] + theLeft[Jx] * theRight[Ky] + theLeft[Kx] * theRight[Kz] + theLeft[Tx] * theRight[Kw];
	m_values[Ky] = theLeft[Iy] * theRight[Kx] + theLeft[Jy] * theRight[Ky] + theLeft[Ky] * theRight[Kz] + theLeft[Ty] * theRight[Kw];
	m_values[Kz] = theLeft[Iz] * theRight[Kx] + theLeft[Jz] * theRight[Ky] + theLeft[Kz] * theRight[Kz] + theLeft[Tz] * theRight[Kw];
	m_values[Kw] = theLeft[Iw] * theRight[Kx] + theLeft[Jw] * theRight[Ky] + theLeft[Kw] * theRight[Kz] + theLeft[Tw] * theRight[Kw];

	m_values[Tx] = theLeft[Ix] * theRight[Tx] + theLeft[Jx] * theRight[Ty] + theLeft[Kx] * theRight[Tz] + theLeft[Tx] * theRight[Tw];
	m_values[Ty] = theLeft[Iy] * theRight[Tx] + theLeft[Jy] * theRight[Ty] + theLeft[Ky] * theRight[Tz] + theLeft[Ty] * theRight[Tw];
	m_values[Tz] = theLeft[Iz] * theRight[Tx] + theLeft[Jz] * theRight[Ty] + theLeft[Kz] * theRight[Tz] + theLeft[Tz] * theRight[Tw];
	m_values[Tw] = theLeft[Iw] * theRight[Tx] + theLeft[Jw] * theRight[Ty] + theLeft[Kw] * theRight[Tz] + theLeft[Tw] * theRight[Tw];
}

void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Mat44 mat44 = Mat44::CreateZRotationDegrees(degreesRotationAboutZ);
	Append(mat44);
}

void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Mat44 mat44 = Mat44::CreateYRotationDegrees(degreesRotationAboutY);
	Append(mat44);
}

void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Mat44 mat44 = Mat44::CreateXRotationDegrees(degreesRotationAboutX);
	Append(mat44);
}

void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Mat44 mat44 = Mat44::CreateTranslation2D(translationXY);
	Append(mat44);
}

void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 mat44 = Mat44::CreateTranslation3D(translationXYZ);
	Append(mat44);
}

void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Mat44 mat44 = Mat44::CreateUniformScale2D(uniformScaleXY);
	Append(mat44);
}

void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Mat44 mat44 = Mat44::CreateUniformScale3D(uniformScaleXYZ);
	Append(mat44);
}

void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 mat44 = Mat44::CreateNonUniformScale2D(nonUniformScaleXY);
	Append(mat44);
}

void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 mat44 = Mat44::CreateNonUniformScale3D(nonUniformScaleXYZ);
	Append(mat44);
}



// Mat44 Mat44::InverseMatix()const
// {
// 	// Function to calculate and store inverse, returns false if
// // matrix is singular
// 
// 
// 			// Find determinant of A[][]
// 	Mat44 
// 	// Find adjoint
// 	int adj[N][N];
// 	adjoint(A, adj);
// 
// 	// Find Inverse using formula "inverse(A) =
// 	// adj(A)/det(A)"
// 	for (int i = 0; i < N; i++)
// 		for (int j = 0; j < N; j++)
// 			inverse[i][j] = adj[i][j] / float(det);
// 
// 	return true;
// 	bool inverse(int A[N][N], float inverse[N][N])
// 	{
// 
// 	}
// }
