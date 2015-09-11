#include "API.h"

#include <stdlib.h>
#include <string.h>

namespace jni
{

Class::Class(const char* name, jclass clazz) : m_Class(clazz)
{
	m_ClassName = static_cast<char *>(malloc(strlen(name) + 1));
	strcpy(m_ClassName, name);
}

Class::~Class()
{
	free(m_ClassName);
}

// ------------------------------------------------
// Proxy Support
// ------------------------------------------------
jni::Class s_JNIBridgeClass("bitter/jnibridge/JNIBridge");

JNIEXPORT jobject JNICALL Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_invoke(JNIEnv* env, jobject thiz, jlong ptr, jobject method, jobjectArray args)
{
	jmethodID methodID = env->FromReflectedMethod(method);
	ProxyInvoker* proxy = (ProxyInvoker*)ptr;
	return proxy->__Invoke(methodID, args);
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
	char invokeMethodSignature[] = "(JLjava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;";
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

::jstring ProxyObject::ToString() const
{
	static java::lang::String str("<native proxy object>");
	return str;
}

jobject ProxyObject::__Invoke(jmethodID mid, jobjectArray args)
{
	jobject result;
	if (!__Invoke(mid, args, &result))
	{
		jni::ThrowNew<java::lang::NoSuchMethodError>("<no such native function>");
	}

	return result;
}

bool ProxyObject::__TryInvoke(jmethodID methodID, jobjectArray args, bool* success, jobject* result)
{
	if (*success)
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


}
