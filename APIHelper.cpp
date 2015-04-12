#include "APIHelper.h"
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

extern "C" JNIEXPORT jobject JNICALL Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_invoke(JNIEnv* env, jobject thiz, jlong ptr, jobject method, jobjectArray args);

Proxy::Proxy(jni::Class& interfaze) : m_Object(NULL)
{
    static jni::Class jniBridge("bitter/jnibridge/JNIBridge");
    static jmethodID newProxyMID = jni::GetStaticMethodID(jniBridge, "newInterfaceProxy", "(JLjava/lang/Class;)Ljava/lang/Object;");
    if (newProxyMID)
        m_Object = jni::Op<jobject>::CallStaticMethod(jniBridge, newProxyMID, (jlong) this, static_cast<jclass>(interfaze));
}

Proxy::~Proxy()
{
    static jni::Class jniBridge("bitter/jnibridge/JNIBridge");
    static jmethodID disableProxyMID = jni::GetStaticMethodID(jniBridge, "disableInterfaceProxy", "(Ljava/lang/Object;)V");
    if (disableProxyMID)
        jni::Op<jvoid>::CallStaticMethod(jniBridge, disableProxyMID, static_cast<jobject>(m_Object));
}

bool Proxy::__Register()
{
	jni::LocalFrame frame;
	jni::Class nativeProxyClass("bitter/jnibridge/JNIBridge");
	char invokeMethodName[] = "invoke";
	char invokeMethodSignature[] = "(JLjava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;";
	JNINativeMethod nativeProxyFunction[] = {{invokeMethodName, invokeMethodSignature, (void*) Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_invoke}};
	if (nativeProxyClass) jni::GetEnv()->RegisterNatives(nativeProxyClass, nativeProxyFunction, 1); // <-- fix this
	return !jni::CheckError();
}

JNIEXPORT jobject JNICALL Java_bitter_jnibridge_JNIBridge_00024InterfaceProxy_invoke(JNIEnv* env, jobject thiz, jlong ptr, jobject method, jobjectArray args)
{
	jmethodID methodID = env->FromReflectedMethod(method);
	Proxy* proxy = (Proxy*)ptr;
	return proxy->__Invoke(methodID, args);
}


}
