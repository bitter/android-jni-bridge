#include "Proxy.h"

namespace jni
{

jni::Class s_JNIBridgeClass("bitter/jnibridge/JNIBridge");
ProxyTracker ProxyObject::proxyTracker;

JNIEXPORT jobject JNICALL Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_invoke(JNIEnv* env, jobject thiz, jlong ptr, jclass clazz, jobject method, jobjectArray args)
{
	jmethodID methodID = env->FromReflectedMethod(method);
	ProxyInvoker* proxy = (ProxyInvoker*)ptr;
	return proxy->__Invoke(clazz, methodID, args);
}

JNIEXPORT void JNICALL Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_delete(JNIEnv* env, jobject thiz, jlong ptr)
{
	delete (ProxyInvoker*)ptr;
}

bool ProxyInvoker::__Register()
{
	jni::LocalFrame frame;
	jni::Class nativeProxyClass("bitter/jnibridge/JNIBridge");
	char invokeMethodName[] = "invoke";
	char invokeMethodSignature[] = "(JLjava/lang/Class;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;";
	char deleteMethodName[] = "delete";
	char deleteMethodSignature[] = "(J)V";

	JNINativeMethod nativeProxyFunction[] = {
		{invokeMethodName, invokeMethodSignature, (void*) Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_invoke},
		{deleteMethodName, deleteMethodSignature, (void*) Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_delete}
	};

	if (nativeProxyClass) jni::GetEnv()->RegisterNatives(nativeProxyClass, nativeProxyFunction, 2); // <-- fix this
	return !jni::CheckError();
}

::jint ProxyObject::HashCode() const
{
	return java::lang::System::IdentityHashCode(java::lang::Object(__ProxyObject()));
}

::jboolean ProxyObject::Equals(const ::jobject arg0) const
{
	return jni::IsSameObject(__ProxyObject(), arg0);
}

java::lang::String ProxyObject::ToString() const
{
	return java::lang::String("<native proxy object>");
}

jobject ProxyObject::__Invoke(jclass clazz, jmethodID mid, jobjectArray args)
{
	jobject result;
	if (!__InvokeInternal(clazz, mid, args, &result))
	{
		java::lang::reflect::Method method(jni::ToReflectedMethod(clazz, mid, false));
		jni::ThrowNew<java::lang::NoSuchMethodError>(method.ToString().c_str());
	}

	return result;
}

void ProxyObject::DeleteAllProxies()
{
	proxyTracker.DeleteAllProxies();
}

bool ProxyObject::__TryInvoke(jclass clazz, jmethodID methodID, jobjectArray args, bool* success, jobject* result)
{
	if (*success)
		return false;

	// in case jmethodIDs are not unique across classes - couldn't find any spec regarding this
	if (!jni::IsSameObject(clazz, java::lang::Object::__CLASS))
		return false;

	static jmethodID methodIDs[] = {
		jni::GetMethodID(java::lang::Object::__CLASS, "hashCode", "()I"),
		jni::GetMethodID(java::lang::Object::__CLASS, "equals", "(Ljava/lang/Object;)Z"),
		jni::GetMethodID(java::lang::Object::__CLASS, "toString", "()Ljava/lang/String;")
	};
	if (methodIDs[0] == methodID) { *result = jni::NewLocalRef(static_cast<java::lang::Integer>(HashCode())); *success = true; return true; }
	if (methodIDs[1] == methodID) { *result = jni::NewLocalRef(static_cast<java::lang::Boolean>(Equals(::java::lang::Object(jni::GetObjectArrayElement(args, 0))))); *success = true; return true; }
	if (methodIDs[2] == methodID) {	*result = jni::NewLocalRef(static_cast<java::lang::String>(ToString())); *success = true;	return true; }

	return false;
}

jobject ProxyObject::NewInstance(void* nativePtr, const jobject* interfaces, size_t interfaces_len)
{
	Array<jobject> interfaceArray(java::lang::Class::__CLASS, interfaces_len, interfaces);

	static jmethodID newProxyMID = jni::GetStaticMethodID(s_JNIBridgeClass, "newInterfaceProxy", "(J[Ljava/lang/Class;)Ljava/lang/Object;");
	return  jni::Op<jobject>::CallStaticMethod(s_JNIBridgeClass, newProxyMID, (jlong) nativePtr, static_cast<jobjectArray>(interfaceArray));
}

void ProxyObject::DisableInstance(jobject proxy)
{
	static jmethodID disableProxyMID = jni::GetStaticMethodID(s_JNIBridgeClass, "disableInterfaceProxy", "(Ljava/lang/Object;)V");
	jni::Op<jvoid>::CallStaticMethod(s_JNIBridgeClass, disableProxyMID, proxy);
}

ProxyTracker::ProxyTracker()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lock, &attr);
	pthread_mutexattr_destroy(&attr);
	head = NULL;
}

ProxyTracker::~ProxyTracker()
{
	pthread_mutex_destroy(&lock);
}

void ProxyTracker::StartTracking(ProxyObject* obj)
{
	pthread_mutex_lock(&lock);
	head = new LinkedProxy(obj, head);
	pthread_mutex_unlock(&lock);
}

void ProxyTracker::StopTracking(ProxyObject* obj)
{
	pthread_mutex_lock(&lock);
	LinkedProxy* current = head;
	LinkedProxy* previous = NULL;
	while (current != NULL && current->obj != obj)
	{
		previous = current;
		current = current->next;
	}

	if (current != NULL)
	{
		if (previous == NULL)
			head = current->next;
		else
			previous->next = current->next;
		delete current;
	}
	pthread_mutex_unlock(&lock);
}

void ProxyTracker::DeleteAllProxies()
{
	pthread_mutex_lock(&lock);
	LinkedProxy* current = head;
	head = NULL; // Destructor will call StopTracking, this will prevent it from looping through the whole list
	while (current != NULL)
	{
		LinkedProxy* previous = current;
		current = current->next;
		delete previous->obj;
		delete previous;
	}
	pthread_mutex_unlock(&lock);
}

}