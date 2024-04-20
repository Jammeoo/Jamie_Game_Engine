#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex,
	float secondsPerFrame /*= 20.f*/, SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::LOOP*/)
	:m_spriteSheet(sheet),m_startSpriteIndex(startSpriteIndex),m_endSpriteIndex(endSpriteIndex),
	m_secondsPerFrame(secondsPerFrame),m_playbackType(playbackType)
{

}


SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{

	int currentFrame = RoundDownToInt(seconds / m_secondsPerFrame);		// 21=2.05f/0.05f
	int keyFrameNum = m_endSpriteIndex - m_startSpriteIndex + 1;			//10=44-34 +1

	if (m_playbackType == SpriteAnimPlaybackType::ONCE) 
	{
// 		int numOfFrames = abs(m_endSpriteIndex - m_startSpriteIndex) + 1;
// 		float animationDuration = m_secondsPerFrame * numOfFrames;
// 
// 		int spriteAnimIndex = m_startSpriteIndex;
// 		if (seconds > animationDuration)
// 		{
// 			spriteAnimIndex = m_endSpriteIndex;
// 		}
// 		else
// 		{
// 			spriteAnimIndex += currentFrame;
// 		}
// 		return  m_spriteSheet.GetSpriteDef(spriteAnimIndex);
		int indexNumCurrentCycle = currentFrame % keyFrameNum;
		if (indexNumCurrentCycle <= keyFrameNum)								//12<=11     1<11
		{
			return m_spriteSheet.GetSpriteDef(indexNumCurrentCycle + m_startSpriteIndex);	//34+1
		}
		else
		{
			return m_spriteSheet.GetSpriteDef(m_endSpriteIndex);					//44
		}
	}
	else if (m_playbackType == SpriteAnimPlaybackType::LOOP) 
	{
		int frameNumInOneCycle = currentFrame % keyFrameNum;
		return m_spriteSheet.GetSpriteDef(frameNumInOneCycle + m_startSpriteIndex);	//21%11=10  10+34
	}
	else /*if (m_playbackType == SpriteAnimPlaybackType::PINGPONG) */
	{
		int frameNumInOneCycle = 2 * keyFrameNum - 2;								//18=10*2-2
		int currentFrameNumInOneCycle = currentFrame % frameNumInOneCycle;			//3=21%18
		if (currentFrameNumInOneCycle <= frameNumInOneCycle)						//21<=18
		{
			int goalFrameNum = currentFrameNumInOneCycle % frameNumInOneCycle;
			return m_spriteSheet.GetSpriteDef(goalFrameNum + m_startSpriteIndex);   //3=21%18 3+34
		}																			//if not 21 but 28
		else
		{
			int goalFrameNum = keyFrameNum - (frameNumInOneCycle % keyFrameNum) ;//28%10=8 10-8=2
			return m_spriteSheet.GetSpriteDef(goalFrameNum + m_startSpriteIndex);
		}
	}
}

bool SpriteAnimDefinition::LoadFromXMLElement(XmlElement& directionDefsElement)
{
	bool result = false;
	XmlElement* animaDefElement = directionDefsElement.FirstChildElement();;
	m_startSpriteIndex = ParseXmlAttribute(*animaDefElement, "startFrame", m_startSpriteIndex);
	m_endSpriteIndex = ParseXmlAttribute(*animaDefElement, "endFrame", m_startSpriteIndex);
	result = true;
	return result;
}

SpriteDefinition const& SpriteAnimDefinition::GetFirstSpriteDefInAnimAtTime() const
{
	return m_spriteSheet.GetSpriteDef(m_startSpriteIndex);
}

int SpriteAnimDefinition::GetSpriteIndexAtTime(float seconds) const
{
	int currentFrame = RoundDownToInt(seconds / m_secondsPerFrame);		// 21=2.05f/0.05f
	int keyFrameNum = m_endSpriteIndex - m_startSpriteIndex + 1;			//10=44-34 +1

	if (m_playbackType == SpriteAnimPlaybackType::ONCE)
	{
		int numOfFrames = abs(m_endSpriteIndex - m_startSpriteIndex) + 1;
		float animationDuration = m_secondsPerFrame * numOfFrames;

		int spriteAnimIndex = m_startSpriteIndex;
		if (seconds > animationDuration)
		{
			spriteAnimIndex = m_endSpriteIndex;
		}
		else
		{
			spriteAnimIndex += currentFrame;
		}
		return  spriteAnimIndex;
		// 		if (currentFrame <= keyFrameNum)								//12<=11     1<11
		// 		{
		// 			return m_spriteSheet.GetSpriteDef(currentFrame + m_startSpriteIndex);	//34+1
		// 		}
		// 		else
		// 		{
		// 			return m_spriteSheet.GetSpriteDef(m_endSpriteIndex);					//44
		// 		}
	}
	else if (m_playbackType == SpriteAnimPlaybackType::LOOP)
	{
		int frameNumInOneCycle = currentFrame % keyFrameNum;
		return (frameNumInOneCycle + m_startSpriteIndex);	//21%11=10  10+34
	}
	else /*if (m_playbackType == SpriteAnimPlaybackType::PINGPONG) */
	{
		int frameNumInOneCycle = 2 * keyFrameNum - 2;								//18=10*2-2
		int currentFrameNumInOneCycle = currentFrame % frameNumInOneCycle;			//3=21%18
		if (currentFrameNumInOneCycle <= frameNumInOneCycle)						//21<=18
		{
			int goalFrameNum = currentFrameNumInOneCycle % frameNumInOneCycle;
			return (goalFrameNum + m_startSpriteIndex);   //3=21%18 3+34
		}																			//if not 21 but 28
		else
		{
			int goalFrameNum = keyFrameNum - (frameNumInOneCycle % keyFrameNum);//28%10=8 10-8=2
			return (goalFrameNum + m_startSpriteIndex);
		}
	}
}



float SpriteAnimDefinition::GetTimeLength()
{
	float result = (float)(m_endSpriteIndex - m_startSpriteIndex + 1) * m_secondsPerFrame;
	return result;
}
