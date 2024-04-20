#pragma once
#include <vector>
#include <string>
#include <map>
#include <mutex>
class NamedStrings;
typedef NamedStrings EventArgs;
typedef bool (*EventCallBackFunction)(EventArgs& args);
//typedef std::vector<EventCallBackFunction> SubscriptionList;


class EventSystem;
extern EventSystem* g_eventSystem;
//----------------------------------------------------------------------------------------------------------------------//

class EventSubcriptionBase
{
	friend class EventSystem;
protected:
	EventSubcriptionBase() = default;
	virtual ~EventSubcriptionBase() = default;
	virtual bool Execute(EventArgs& args) = 0;
protected:

};
//----------------------------------------------------------------------------------------------------------------------//

typedef std::vector<EventSubcriptionBase*>SubscriptionList;

class EventSubcription_Function:public EventSubcriptionBase
{
	friend class EventSystem;
protected:
	EventSubcription_Function(EventCallBackFunction functPtr) :m_funcPtr(functPtr)
	{

	}
	virtual ~EventSubcription_Function() = default;
	virtual bool Execute(EventArgs& args) override 
	{
		return m_funcPtr(args);
	}
protected:
	EventCallBackFunction m_funcPtr = nullptr;
};

//----------------------------------------------------------------------------------------------------------------------//
template<typename T>
class EventSubcription_ObjectMethod :public EventSubcriptionBase
{
	friend class EventSystem;
	typedef bool (T::* EventCallBackObjectMethod)(EventArgs& args);
protected:
	EventSubcription_ObjectMethod(T& objectInstance, EventCallBackObjectMethod method)
		:m_object(objectInstance),m_method(method)
	{

	}
	virtual ~EventSubcription_ObjectMethod() = default;
	virtual bool Execute(EventArgs& args) override
	{
		bool consumedEvent = (m_object.*m_method)(args);
		return consumedEvent;
	}
protected:
	T& m_object;
	EventCallBackObjectMethod m_method = nullptr;
	//EventCallBackFunction m_funcPtr = nullptr;

};

class EventSystem
{
public:
	EventSystem();
	~EventSystem();
	void Startup();
	void ShutDown();
	void BeginFrame();
	void EndFrame();
	void SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
	template<typename T>
	void SubscribeEventCallbackObjectMethod(std::string const& eventName, T& object, bool (T::* method)(EventArgs& args));

	void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
	void UnsubscribeFromAllEvents();
	void FireEvent(std::string const& eventName, EventArgs& arg);
	void FireEvent(std::string const& eventName);
	void GetNamesOfAllEvents(std::vector<std::string>& nameVector);
	bool IsEventSubscribed(std::string const& targetEventName);
	//const char* CheckEventSubscribed(std::string const& targetEventName);



protected:
	std::map<std::string, SubscriptionList> m_subscriptionListsByEventName;



private:
	std::mutex m_eventSysMutex;

};


void SubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
void UnsubscribeEventCallbackFunction(std::string const& eventName, EventCallBackFunction functionPtr);
void UnsubscribeFromAllEvents();
void FireEvent(std::string const& eventName, EventArgs& arg);
void FireEvent(std::string const& eventName);

template<typename T>
void EventSystem::SubscribeEventCallbackObjectMethod(std::string const& eventName, T& object, bool (T::* method)(EventArgs& args))
{
	SubscriptionList& subscribers = m_subscriptionListsByEventName[eventName];//Create an entry if it didn't exist
	EventSubcription_ObjectMethod<T>* newSubscripter = new EventSubcription_ObjectMethod<T>(object, method);
	subscribers.push_back(newSubscripter);
}