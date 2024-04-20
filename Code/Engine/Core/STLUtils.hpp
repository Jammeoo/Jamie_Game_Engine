#pragma once
// #include "Engine/Core/EngineCommon.hpp"
// #include <string>
 #include<vector>

template<typename T_TypeOfThingPointedTo>


void ClearAndDeleteEverything(std::vector<T_TypeOfThingPointedTo* >& myVector) //Pointer to whatever
{
	for (int index = 0; index < (int)myVector.size(), ++index) 
	{
		delete myVector[index];//delete a nullptr in C++ is okay
	}
	myVector.clear();
}


