#pragma once
struct Vec2; 
struct Vec3; 
struct Vec4;
#include <math.h>
//-----------------------------------------------------------------------------------------
// A 4x4 Homogeneous 2D/3D transformation matrix, stored basis-major in memory (Ix,Iy,Iz,Iw,Jx,Jy...).
// Note: we specifically DO NOT provide an operator* overload, since doing so would require a
// decision to commit the Mat44 to only work correctly with EITHER column-major or row- major style
//They demand two different ways of writing operator*, and in order to implement
//an operator*, we are forced to make a notational committment.This is certainly ambiguous to the
// reader and, at the very least, potentially confusing .Instead, we prefer to use method names ,
// such append", which are more neutral (e.g. multiply a new matrix "on the right in column
// notation / on the left in row- notation"
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------
//	Mat44()
//		 Ix,  Jx,  Kx,  Tx, 
//		 Iy,  Jy,  Ky,  Ty, 
//		 Iz,  Jz,  Kz,	Tz, 
//		 Iw,  Jw,  Kw,  Tw 
//	
//////////////////////////////////////////////////////////////////////////

struct Mat44
{
	enum { Ix, Iy, Iz, Iw,	Jx, Jy, Jz, Jw,  Kx, Ky, Kz, Kw,   Tx, Ty, Tz, Tw }; // index nicknames, [0] through [15]
	float m_values[16]; // stored in "basis major" order (Ix,Iy,Iz,Iw,Jx...) - translation in [12,13,14]
	Mat44();
	explicit Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D); 
	explicit Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D);
	explicit Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	explicit Mat44(float const* sixteenValuesBasisMajor);

	static Mat44 const CreateTranslation2D(Vec2 const& translationXY);
	static Mat44 const CreateTranslation3D(Vec3 const& translationXYZ);
	static Mat44 const CreateUniformScale2D(float uniformScaleXY);
	static Mat44 const CreateUniformScale3D(float uniformScaleXYZ);
	static Mat44 const CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY);
	static Mat44 const CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ);
	static Mat44 const CreateZRotationDegrees(float rotationDegreesAboutZ);
	static Mat44 const CreateYRotationDegrees(float rotationDegreesAboutY);
	static Mat44 const CreateXRotationDegrees(float rotationDegreesAboutX);
	static Mat44 const CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	static Mat44 const CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar); 


	//Vector Quantity: any vector is not position
	Vec2 const TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const; // assumes z=0, w=0/no position transform only rotation
	Vec3 const TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const; // assumes w=0
	Vec2 const TransformPosition2D(Vec2 const& positionXY) const;// assumes z=0, w=1
	Vec3 const TransformPosition3D(Vec3 const& position3D) const;// assumes w=1
	Vec4 const TransformHomogeneous3D(Vec4 const& homogeneousPoint3D)const; // w is provided 


	float*			GetAsFloatArray();			// non-const (mutable) version
	float const*	GetAsFloatArray() const;	// const version, used only when Mat44 is const
	Vec2 const		GetIBasis2D() const;
	Vec2 const		GetJBasis2D() const;
	Vec2 const		GetTranslation2D() const;
	Vec3 const		GetIBasis3D() const;
	Vec3 const		GetJBasis3D() const;
	Vec3 const		GetKBasis3D() const;
	Vec3 const		GetTranslation3D() const;
	Vec4 const		GetIBasis4D()const;
	Vec4 const		GetJBasis4D() const;
	Vec4 const		GetKBasis4D() const;
	Vec4 const		GetTranslation4D() const;
	Mat44 const		GetOrthonormalInverse()const;
	
	void SetTranslation2D(Vec2 const& translationXY);// Sets translationZ = ,translationW = 1
	void SetTranslation3D( Vec3 const& translationXYZ );// Sets translationW = 1
	void SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D); // sets z=0, w= for i & j; does not modify k or t
	void SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY); // Sets Z=0, w-0 for i,j,t; does not modify k
	void SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D);  // Sets w = 0 for i, j, k
	void SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ); // sets w=0 for i,j,k,t
	void SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D);
	void Transpose();// Swap columns for rows
	void Orthonormalize_XFwd_YLeft_ZUp();//Forward is Canonical, Up is secondary and Left tertiary
	 

	void Append(Mat44 const& appendThis);					// multiply on right in column notation / on left in row notation
	void AppendZRotation(float degreesRotationAboutZ);		//same as appending(*= in column notation) a z-rotation matrix
	void AppendYRotation(float degreesRotationAboutY);		//same as appending(*= in column notation) a y-rotation matrix
	void AppendXRotation(float degreesRotationAboutX);		//same as appending(*= in column notation) a x-rotation matrix
	void AppendTranslation2D(Vec2 const& translationXY);    //same as appending (*= in column notation) a translation matrix
	void AppendTranslation3D(Vec3 const& translationXYZ);    //same as appending (*= in column notation) a translation matrix
	void AppendScaleUniform2D(float uniformScaleXY);		// K and T bases should remain unaffected
	void AppendScaleUniform3D(float uniformScaleXYZ);		// translation should remain unaffected
	void AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY); // K and T bases should remain unaffected
	void AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ); // translation should remain unaffected

		// Function to calculate the determinant of a 3x3 matrix
	float Determinant3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i) const {
		return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
	}
	// Function to calculate the determinant of the matrix
	float Determinant() const {
		// Using Laplace expansion for a 4x4 matrix
		return m_values[0] * Determinant3x3(m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]) -
			m_values[1] * Determinant3x3(m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]) +
			m_values[2] * Determinant3x3(m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]) -
			m_values[3] * Determinant3x3(m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]);
	}
	// Function to calculate the inverse matrix
	Mat44 Inverse() const {
		Mat44 result;

		float det = this->Determinant();

		if (fabsf(det) < 1e-6f) {
			// Matrix is singular, cannot compute the inverse
			// You might want to handle this case differently based on your application's requirements
			// For example, returning an identity matrix or throwing an exception
			// For now, just return the original matrix
			return *this;
		}

		float invDet = 1.0f / det;

		result.m_values[0] = Determinant3x3(m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]) * invDet;
		result.m_values[1] = -Determinant3x3(m_values[1], m_values[2], m_values[3], m_values[9], m_values[10], m_values[11], m_values[13], m_values[14], m_values[15]) * invDet;
		result.m_values[2] = Determinant3x3(m_values[1], m_values[2], m_values[3], m_values[5], m_values[6], m_values[7], m_values[13], m_values[14], m_values[15]) * invDet;
		result.m_values[3] = -Determinant3x3(m_values[1], m_values[2], m_values[3], m_values[5], m_values[6], m_values[7], m_values[9], m_values[10], m_values[11]) * invDet;

		result.m_values[4] = -Determinant3x3(m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]) * invDet;
		result.m_values[5] = Determinant3x3(m_values[0], m_values[2], m_values[3], m_values[8], m_values[10], m_values[11], m_values[12], m_values[14], m_values[15]) * invDet;
		result.m_values[6] = -Determinant3x3(m_values[0], m_values[2], m_values[3], m_values[4], m_values[6], m_values[7], m_values[12], m_values[14], m_values[15]) * invDet;
		result.m_values[7] = Determinant3x3(m_values[0], m_values[2], m_values[3], m_values[4], m_values[6], m_values[7], m_values[8], m_values[10], m_values[11]) * invDet;

		result.m_values[8] = Determinant3x3(m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]) * invDet;
		result.m_values[9] = -Determinant3x3(m_values[0], m_values[1], m_values[3], m_values[8], m_values[9], m_values[11], m_values[12], m_values[13], m_values[15]) * invDet;
		result.m_values[10] = Determinant3x3(m_values[0], m_values[1], m_values[3], m_values[4], m_values[5], m_values[7], m_values[12], m_values[13], m_values[15]) * invDet;
		result.m_values[11] = -Determinant3x3(m_values[0], m_values[1], m_values[3], m_values[4], m_values[5], m_values[7], m_values[8], m_values[9], m_values[11]) * invDet;

		result.m_values[12] = -Determinant3x3(m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]) * invDet;
		result.m_values[13] = Determinant3x3(m_values[0], m_values[1], m_values[2], m_values[8], m_values[9], m_values[10], m_values[12], m_values[13], m_values[14]) * invDet;
		result.m_values[14] = -Determinant3x3(m_values[0], m_values[1], m_values[2], m_values[4], m_values[5], m_values[6], m_values[12], m_values[13], m_values[14]) * invDet;
		result.m_values[15] = Determinant3x3(m_values[0], m_values[1], m_values[2], m_values[4], m_values[5], m_values[6], m_values[8], m_values[9], m_values[10]) * invDet;

		return result;

	}
};
