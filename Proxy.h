#pragma once

#include "API.h"
#include <pthread.h>

namespace jni
{

class ProxyTracker;

class ProxyObject : public virtual ProxyInvoker
{
// Dispatch invoke calls
public:
	virtual jobject __Invoke(jclass clazz, jmethodID mid, jobjectArray args);
// Cleanup all proxy objects
	static void DeleteAllProxies();

// These functions are special and always forwarded
protected:
	virtual ::jint HashCode() const;
	virtual ::jboolean Equals(const ::jobject arg0) const;
	virtual java::lang::String ToString() const;

	bool __TryInvoke(jclass clazz, jmethodID methodID, jobjectArray args, bool* success, jobject* result);
	virtual bool __InvokeInternal(jclass clazz, jmethodID mid, jobjectArray args, jobject* result) = 0;

// Factory stuff
protected:
	static jobject NewInstance(void* nativePtr, const jobject* interfaces, size_t interfaces_len);
	static void DisableInstance(jobject proxy);

	static ProxyTracker proxyTracker;
};


class ProxyTracker 
{
public:
	ProxyTracker();
	~ProxyTracker();
	void StartTracking(ProxyObject* obj);
	void StopTracking(ProxyObject* obj);
	void DeleteAllProxies();

private:
	class LinkedProxy
	{
	public:
		LinkedProxy(ProxyObject* target, LinkedProxy* link) : obj(target), next(link) {}

		ProxyObject* obj;
		LinkedProxy* next;
	};

	LinkedProxy* head;
	pthread_mutex_t lock;
};

template <class RefAllocator, class ...TX>
class ProxyGenerator : public ProxyObject, public TX::__Proxy...
{	
protected:
	ProxyGenerator() : m_ProxyObject(NewInstance(this, (jobject[]){TX::__CLASS...}, sizeof...(TX)))	
	{
		proxyTracker.StartTracking(this);
	}

	virtual ~ProxyGenerator()
	{
		proxyTracker.StopTracking(this);
		DisableInstance(__ProxyObject());
	}

	virtual ::jobject __ProxyObject() const { return m_ProxyObject; }

private:
	template<typename... Args> inline void DummyInvoke(Args&&...) {}
	virtual bool __InvokeInternal(jclass clazz, jmethodID mid, jobjectArray args, jobject* result)
	{
		bool success = false;
		DummyInvoke(ProxyObject::__TryInvoke(clazz, mid, args, &success, result), TX::__Proxy::__TryInvoke(clazz, mid, args, &success, result)...);
		return success;
	}

	Ref<RefAllocator, jobject> m_ProxyObject;
};

template <class ...TX> class Proxy     : public ProxyGenerator<GlobalRefAllocator, TX...> {};
template <class ...TX> class WeakProxy : public ProxyGenerator<WeakGlobalRefAllocator, TX...> {};

}