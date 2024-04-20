#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees)
	:m_yawDegrees(yawDegrees),
	m_pitchDegrees(pitchDegrees),
	m_rollDegrees(rollDegrees)
{
	
}

void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	Mat44 mat44;
	mat44.AppendZRotation(m_yawDegrees);
	mat44.AppendYRotation(m_pitchDegrees);
	mat44.AppendXRotation(m_rollDegrees);

	out_forwardIBasis = mat44.GetIBasis3D();
	out_leftJBasis = mat44.GetJBasis3D();
	out_upKBasis = mat44.GetKBasis3D();
}

Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	Mat44 mat44;
	mat44.AppendZRotation(m_yawDegrees);
	mat44.AppendYRotation(m_pitchDegrees);
	mat44.AppendXRotation(m_rollDegrees);
	return mat44;
}

Mat44 EulerAngles::GetAsMatrix_XFwdOnly() const
{
	Mat44 mat44;
	mat44.AppendZRotation(m_yawDegrees);
	return mat44;
}

Vec3 EulerAngles::GetForwardDir_XFwd_YLeft_ZUp()
{
	float cy = CosDegrees(m_yawDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	Vec3 forward;
	forward.x = cy * cp;
	forward.y = sy * cp;
	forward.z = -sp;
	return forward;
}

void EulerAngles::SetFromText(char const* text)
{
	Strings originalText = SplitStringOnDelimiter(text, ',');
	m_yawDegrees = (float)atof(originalText[0].c_str());
	m_pitchDegrees = (float)atof(originalText[1].c_str());
	m_rollDegrees = (float)atof(originalText[2].c_str());
}

Vec3 EulerAngles::GetForwardVector() const
{
	return this->GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D().GetNormalized();
}
