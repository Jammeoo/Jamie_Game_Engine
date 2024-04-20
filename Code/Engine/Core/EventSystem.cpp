#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/EngineCommon.hpp"

EventSystem* g_eventSystem = nullptr;

EventSystem::EventSystem()
{

}

EventSystem::~EventSystem()
{

}

void EventSystem::Startup()
{

}

void EventSystem::ShutDown()
{

}

void EventSystem::BeginFrame()
{

}

void EventSystem::EndFrame()
{

}

void EventSystem::SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	SubscriptionList& subscribers = m_subscriptionListsByEventName[eventName];//Create an entry if it didn't exist
	EventSubcription_Function* newSubscripter = new EventSubcription_Function(functionPtr);
	subscribers.push_back(newSubscripter);
		 
	//m_eventSysMutex.lock();
	//SubscriptionList& subscriberOfThisEvent = m_subscriptionListsByEventName[eventName];
	//subscriberOfThisEvent.push_back(functionPtr);
	//m_eventSysMutex.unlock();
}

void EventSystem::UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{

	std::map<std::string, SubscriptionList>::iterator found = m_subscriptionListsByEventName.find(eventName);
	if (found == m_subscriptionListsByEventName.end())
	{
		return;//No subscribers for this event name
	}
	SubscriptionList& subscriberForThisEvent = found->second;
	for (int subscriberIndex = 0; subscriberIndex < (int)subscriberForThisEvent.size(); subscriberIndex++)
	{
		EventSubcriptionBase* subscriber = subscriberForThisEvent[subscriberIndex];

		EventSubcription_Function* asFuncSubscriber = dynamic_cast<EventSubcription_Function*>(subscriber);
		if (asFuncSubscriber && asFuncSubscriber->m_funcPtr == functionPtr) 
		{
			delete subscriber;
			subscriber = nullptr;
		}
	}

	//m_eventSysMutex.lock();
	//SubscriptionList& subscriberOfThisEvent = m_subscriptionListsByEventName[eventName];
	//for (int index = 0; index < (int)subscriberOfThisEvent.size(); index++) 
	//{
	//	if (subscriberOfThisEvent[index] == functionPtr) 
	//	{
	//		subscriberOfThisEvent[index] = nullptr;
	//	}
	//}
	//m_eventSysMutex.unlock();
}

void EventSystem::UnsubscribeFromAllEvents()
{
	//m_eventSysMutex.lock();
	//std::map<std::string, std::vector<EventCallBackFunction>>::iterator eventItr;
	//for (eventItr = m_subscriptionListsByEventName.begin(); eventItr != m_subscriptionListsByEventName.end(); eventItr--) 
	//{
	//	std::string const& eventName = eventItr->first;
	//	m_eventSysMutex.unlock();
	//
	//	std::vector<EventCallBackFunction> tempFuncList = eventItr->second;
	//	for (int functionIndex = 0; functionIndex < tempFuncList.size(); functionIndex++) 
	//	{
	//		UnsubscribeEventCallbackFunction(eventName, tempFuncList[functionIndex]);
	//	}
	//
	//	m_eventSysMutex.lock();
	//}
	//m_eventSysMutex.unlock();
}

void EventSystem::FireEvent(std::string const& eventName, EventArgs& arg)
{
	std::map<std::string, SubscriptionList>::iterator found = m_subscriptionListsByEventName.find(eventName);
	if (found == m_subscriptionListsByEventName.end()) 
	{
		return;//No subscribers for this event name
	}

	SubscriptionList& subscriberForThisEvent = found->second;
	for (int subscriberIndex = 0; subscriberIndex < (int)subscriberForThisEvent.size(); subscriberIndex++) 
	{
		EventSubcriptionBase* subscriber = subscriberForThisEvent[subscriberIndex];
		if (subscriber) 
		{
			bool consumedEvent = subscriber->Execute(arg);
			if (consumedEvent) {
				break;//Stop calling subscriber callbacks, this subscriber consumed this event
			}
		}
	}

// 	m_eventSysMutex.lock();
// 	SubscriptionList& subscriberOfThisEvent = m_subscriptionListsByEventName[eventName];
// 	for (int index = 0; index < (int)subscriberOfThisEvent.size(); index++)
// 	{
// 		EventCallBackFunction callbackFuncPtr = subscriberOfThisEvent[index];
// 		if (callbackFuncPtr)
// 		{
// 			m_eventSysMutex.unlock();
// 			bool wasConsumed = callbackFuncPtr(arg);
// 			m_eventSysMutex.lock();
// 			if(wasConsumed) break;
// 		}
// 	}
// 	m_eventSysMutex.unlock();
}

void EventSystem::FireEvent(std::string const& eventName)
{
	EventArgs emptyArg;
	FireEvent(eventName, emptyArg);
}

void EventSystem::GetNamesOfAllEvents(std::vector<std::string>& nameVector)
{
	UNUSED(nameVector);
	//m_eventSysMutex.lock();
	//std::map<std::string, std::vector<EventCallBackFunction>>::iterator eventItr;
	//for (eventItr = m_subscriptionListsByEventName.begin(); eventItr != m_subscriptionListsByEventName.end(); eventItr++)
	//{
	//	std::string const& eventName = eventItr->first;
	//	nameVector.push_back(eventName);
	//}
	//m_eventSysMutex.unlock();

}

bool EventSystem::IsEventSubscribed(std::string const& targetEventName)
{
	UNUSED(targetEventName);
	//std::map<std::string, std::vector<EventCallBackFunction>>::iterator eventItr;
	//for (eventItr = m_subscriptionListsByEventName.begin(); eventItr != m_subscriptionListsByEventName.end(); eventItr--)
	//{
	//	std::string const& eventName = eventItr->first;
	//	if (targetEventName == eventName) 
	//	{
	//		return true;
	//	}
	//}
	return false;
}

// const char* EventSystem::CheckEventSubscribed(std::string const& targetEventName)
// {
// 	std::string resultString;
// 	if (IsEventSubscribed(targetEventName)) 
// 	{
// 		resultString = targetEventName;
// 	}
// 	else 
// 	{
// 		resultString = "Unknown command: " + targetEventName;
// 	}
// 
// 	return resultString.c_str();
// }



void SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	g_eventSystem->SubscribeEventCallbackFunction(eventName, functionPtr);
}

void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr)
{
	g_eventSystem->UnsubscribeEventCallbackFunction(eventName, functionPtr);

}

void UnsubscribeFromAllEvents()
{
	g_eventSystem->UnsubscribeFromAllEvents();
}

void FireEvent(std::string const& eventName, EventArgs& arg)
{
	g_eventSystem->FireEvent(eventName, arg);
}

void FireEvent(std::string const& eventName)
{
	g_eventSystem->FireEvent(eventName);
}


