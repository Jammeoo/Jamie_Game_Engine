#pragma once
#include "Engine/Core/XmlUtils.hpp"
class SpriteSheet;
class StopWatch;
class SpriteDefinition;
enum class SpriteAnimPlaybackType
{
	ONCE,		// for 5-frame anim, plays 0,1,2,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4...
	LOOP,		// for 5-frame anim, plays 0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0,1,2,3,4,0...
	PINGPONG,	// for 5-frame anim, plays 0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1,2,3,4,3,2,1,0,1...
};
//------------------------------------------------------------------------------------------------
class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float secondsPerFrame = 0.25f, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);
	SpriteDefinition const& GetSpriteDefAtTime(float seconds) const;
	bool LoadFromXMLElement(XmlElement& directionDefsElement);
	SpriteDefinition const& GetFirstSpriteDefInAnimAtTime() const;
	int GetSpriteIndexAtTime(float seconds) const;
	float GetTimeLength();
private:

	SpriteSheet const& m_spriteSheet;
	
	int				m_startSpriteIndex = -1;
	int				m_endSpriteIndex = -1;
	float				m_secondsPerFrame = 0.05f; // Client specifies FPS in constructor, but we store period = 1/FPS
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;
	StopWatch* m_animStopWatch = nullptr;
};
