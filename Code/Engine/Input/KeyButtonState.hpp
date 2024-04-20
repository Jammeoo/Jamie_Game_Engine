#pragma once
struct KeyButtonState 
{

public:
	void Update();
public:
	bool m_IsPressed = false;
	bool m_wasPressedLastFrame = false;
};