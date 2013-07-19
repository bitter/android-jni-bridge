#pragma once
#include <stdint.h>
#include <jni.h>
#if 0 // ANDROID
#include <android/log.h>
#define JNI_TRACE(...) __android_log_print(ANDROID_LOG_VERBOSE, "JNIBridge", __VA_ARGS__)
#else
#define JNI_TRACE(...) do {} while (false)
#endif

typedef void* jvoid; // make it possible to return void

namespace jni
{

enum Errno
{
	kJNI_NO_ERROR = 0,
	kJNI_ATTACH_FAILED,
	kJNI_INVALID_PARAMETERS,
	kJNI_EXCEPTION_THROWN
};

// --------------------------------------------------------------------------------------
// Initialization and error functions
// --------------------------------------------------------------------------------------
void        Initialize(JavaVM& vm);

Errno       CheckError();
Errno       PeekError();
const char* GetErrorMessage();

jthrowable  ExceptionThrown(jclass clazz = 0);

// Internalish
bool        CheckForParameterError(bool valid);
bool        CheckForExceptionError(JNIEnv* env);

// --------------------------------------------------------------------------------------
// Oracle JNI functions (a selection of)
// http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/functions.html#wp9502
// --------------------------------------------------------------------------------------

JavaVM*     GetJavaVM();
JNIEnv*     AttachCurrentThread();
void        DetachCurrentThread();

jclass      FindClass(const char* name);

jint        Throw(jthrowable object);
jint        ThrowNew(jclass exception, const char* message);
void        FatalError(const char* str);

jobject     NewGlobalRef(jobject obj);
void        DeleteGlobalRef(jobject obj);

jclass      GetObjectClass(jobject object);
jboolean    IsInstanceOf(jobject object, jclass clazz);

jmethodID   GetMethodID(jclass clazz, const char* name, const char* signature);
jfieldID    GetFieldID(jclass clazz, const char* name, const char* signature);
jmethodID   GetStaticMethodID(jclass clazz, const char* name, const char* signature);
jfieldID    GetStaticFieldID(jclass clazz, const char* name, const char* signature);

jobject		NewObject(jclass clazz, jmethodID methodID, ...);

jstring     NewStringUTF(const char* str);
jsize       GetStringUTFLength(jstring string);
const char* GetStringUTFChars(jstring str, jboolean* isCopy = 0);
void        ReleaseStringUTFChars(jstring str, const char* utfchars);


class LocalFrame
{
public:
	LocalFrame(jint capacity = 64);
	~LocalFrame();

private:
	LocalFrame(const LocalFrame& frame);
	LocalFrame& operator=(const LocalFrame& rhs);
	bool m_FramePushed;
};

// Some logic explanation
// Only invoke function if thread can be attached
// Only invoke function if 'parameters' == true
// If function is 'exception safe' only check for exceptions after function has been invoked
// Only adjust return value on exception if function is not exception safe
#define JNI_CALL_NO_RET(parameters, check_exception, function)                                          \
	JNI_TRACE("%d:%d:%s", static_cast<bool>(parameters), check_exception, #function);                   \
	JNIEnv* env(AttachCurrentThread());                                                                 \
	if (env && !CheckForParameterError(parameters) && !(check_exception && CheckForExceptionError(env)))\
	{                                                                                                   \
		function;                                                                                       \
	    CheckForExceptionError(env);                                                                    \
	}

#define JNI_CALL(rt, parameters, check_exception, function)                                             \
	JNI_TRACE("%d:%d:%s %s", static_cast<bool>(parameters), check_exception, #rt, #function);           \
	JNIEnv* env(AttachCurrentThread());                                                                 \
	if (env && !CheckForParameterError(parameters) && !(check_exception && CheckForExceptionError(env)))\
	{                                                                                                   \
		rt JNI_CALL_result = function;                                                                  \
		if (!(CheckForExceptionError(env) && check_exception))                                          \
			return JNI_CALL_result;                                                                     \
	}                                                                                                   \
	return 0

//----------------------------------------------------------------------------
// JNI Operations
// Heavily inspired by https://github.com/kohsuke/jnitl
//----------------------------------------------------------------------------
template <class JT, class RT,
	RT   (JNIEnv::* CallMethodOP)(jobject, jmethodID, va_list),
	RT   (JNIEnv::* CallStaticMethodOP)(jclass, jmethodID, va_list),
	RT   (JNIEnv::* GetFieldOP)(jobject, jfieldID),
	void (JNIEnv::* SetFieldOP)(jobject, jfieldID, RT),
	RT   (JNIEnv::* GetStaticFieldOP)(jclass, jfieldID),
	void (JNIEnv::* SetStaticFieldOP)(jclass, jfieldID, RT)
>
class Basic_Op
{
public:
	static JT CallStaticMethod(jclass clazz, jmethodID id, ...)
	{
		va_list args;
		va_start(args, id);
		JNI_CALL(JT, clazz && id, true, static_cast<JT>((env->*CallStaticMethodOP)(clazz, id, args)));
		va_end(args);
	}
	static JT CallMethod(jobject object, jmethodID id, ...)
	{
		va_list args;
		va_start(args, id);
		JNI_CALL(JT, object && id, true, static_cast<JT>((env->*CallMethodOP)(object, id, args)));
		va_end(args);
	}
	static JT GetStaticField(jclass clazz, jfieldID id)
	{
		JNI_CALL(JT, clazz && id, true, static_cast<JT>((env->*GetStaticFieldOP)(clazz, id)));
	}
	static JT GetField(jobject object, jfieldID id)
	{
		JNI_CALL(JT, object && id, true, static_cast<JT>((env->*GetFieldOP)(object, id)));
	}
	static void SetField(jobject object, jfieldID id, const RT& value)
	{
		JNI_CALL_NO_RET(object && id, true, (env->*SetFieldOP)(object, id, value));
	}
	static void SetStaticField(jclass clazz, jfieldID id, const RT& value)
	{
		JNI_CALL_NO_RET(clazz && id, true, (env->*SetStaticFieldOP)(clazz, id, value));
	}
};

#define JNITL_DEF_OP_LIST(mt,ft) \
	&JNIEnv::Call##mt##V, \
	&JNIEnv::CallStatic##mt##V, \
	&JNIEnv::Get##ft, \
	&JNIEnv::Set##ft, \
	&JNIEnv::GetStatic##ft, \
	&JNIEnv::SetStatic##ft

#define JNITL_DEF_OP(t,mt,ft) \
	template <> \
	class Op<t> : public Basic_Op<t,t, \
		JNITL_DEF_OP_LIST(mt,ft) \
	> {};


	// it defaults to jobject
template<typename T>
class Op : public Basic_Op<T, jobject, JNITL_DEF_OP_LIST(ObjectMethod, ObjectField)> {};

// specialization for primitives
JNITL_DEF_OP(jboolean,BooleanMethod,BooleanField)
JNITL_DEF_OP(jint,IntMethod,IntField)
JNITL_DEF_OP(jshort,ShortMethod,ShortField)
JNITL_DEF_OP(jbyte,ByteMethod,ByteField)
JNITL_DEF_OP(jlong,LongMethod,LongField)
JNITL_DEF_OP(jfloat,FloatMethod,FloatField)
JNITL_DEF_OP(jdouble,DoubleMethod,DoubleField)
JNITL_DEF_OP(jchar,CharMethod,CharField)

// void requires a specialization.
template <>
class Op<jvoid>
{
public:
	static jvoid CallMethod(jobject object, jmethodID id, ...)
	{
		va_list args;
		va_start(args, id);
		JNI_CALL_NO_RET(object && id, true, env->CallVoidMethodV(object, id, args));
		va_end(args);
		return 0;
	}
	static jvoid CallStaticMethod(jclass clazz, jmethodID id, ...)
	{
		va_list args;
		va_start(args, id);
		JNI_CALL_NO_RET(clazz && id, true, env->CallStaticVoidMethodV(clazz, id, args));
		va_end(args);
		return 0;
	}
};

}
