#include "JNIBridge.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

namespace jni
{

// __thread is not supported on android (dynamic linker) :(
template <typename T>
class TLS
{
public:
	TLS()  { pthread_key_create(&m_Key, free); }
	~TLS() { pthread_key_delete(m_Key); }
	inline operator T () const { return static_cast<T>(pthread_getspecific(m_Key)); }
	inline T operator = (const T value) { pthread_setspecific(m_Key, value); return value; }
private:
	TLS(const TLS& tls);
	TLS<T> operator = (const TLS<T>&);
private:
	pthread_key_t m_Key;
};

static JavaVM*     g_JavaVM;

// --------------------------------------------------------------------------------------
// Oracle JNI functions (hidden)
// http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/functions.html#wp9502
// --------------------------------------------------------------------------------------
static jint PushLocalFrame(jint capacity)
{
	JNI_CALL_RETURN(jint, true, true, env->PushLocalFrame(capacity));
}

static jobject PopLocalFrame(jobject result)
{
	JNI_CALL_RETURN(jobject, true, false, env->PopLocalFrame(result));
}

// --------------------------------------------------------------------------------------
// Initialization and error functions (hidden)
// --------------------------------------------------------------------------------------
struct Error
{
	Errno errno;
	char  errstr[256];
};
static TLS<Error*> g_Error;

static inline Error& GetErrorInternal()
{
	Error* error = g_Error;
	if (!error)
	{
		error = static_cast<Error*>(malloc(sizeof(*error)));
		memset(error, 0, sizeof(*error));
		g_Error = error;
	}
	return *error;
}

static inline void SetError(Errno errno, const char* errmsg)
{
	Error& error = GetErrorInternal();
	if (error.errno)
		return;

	error.errno = errno;
	strcpy(error.errstr, errmsg);
}

static void ClearErrors()
{
	JNIEnv* env = AttachCurrentThread();
	if (env)
	{
		GetErrorInternal().errno = kJNI_NO_ERROR;
		env->ExceptionClear();
	}
}

// --------------------------------------------------------------------------------------
// Initialization and error functions (public)
// --------------------------------------------------------------------------------------
void Initialize(JavaVM& vm)
{
	g_JavaVM = &vm;
}

Errno PeekError()
{
	return GetErrorInternal().errno;
}

const char* GetErrorMessage()
{
	return &GetErrorInternal().errstr[0];
}

Errno CheckError()
{
	Errno errno = PeekError();
	if (errno)
		ClearErrors();
	return errno;
}

jthrowable ExceptionThrown(jclass clazz)
{
	JNIEnv* env = AttachCurrentThread();
	if (!env)
		return 0;

	jthrowable t = env->ExceptionOccurred();
	if (!t)
		return 0;

	if (clazz)
	{
		env->ExceptionClear();
		if (!env->IsInstanceOf(t, clazz))
		{
			env->Throw(t); // re-throw
			return 0;
		}
	}

	ClearErrors();
	return t;
}

bool CheckForParameterError(bool valid)
{
	if (!valid)
		SetError(kJNI_INVALID_PARAMETERS, "java.lang.IllegalArgumentException: Null parameter detected");
	return !valid;
}

bool CheckForExceptionError(JNIEnv* env) // Do we need to make this safer?
{
	if (env->ExceptionCheck())
	{
		Error& error = GetErrorInternal();
		if (!error.errno)
		{
			SetError(kJNI_EXCEPTION_THROWN, "java.lang.IllegalThreadStateException: Unable to determine exception message");

			LocalFrame frame;
			jthrowable t = env->ExceptionOccurred();
			env->ExceptionClear();
			{
				jclass jobject_class = env->FindClass("java/lang/Object");
				jstring jmessage = Op<jstring>::CallMethod(t, env->GetMethodID(jobject_class, "toString", "()Ljava/lang/String;"));

				const char* message = env->GetStringUTFChars(jmessage, NULL);
				strncpy(error.errstr, message, sizeof(error.errstr));
				error.errstr[sizeof(error.errstr) - 1] = 0;

				env->ReleaseStringUTFChars(jmessage, message);
			}
			env->Throw(t); // re-throw exception
			if (!env->ExceptionOccurred())
			{
				int* p = 0; *p = 4711;
			}
		}
		return true;
	}
	return false;
}

// --------------------------------------------------------------------------------------
// Oracle JNI functions (public)
// http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/functions.html#wp9502
// --------------------------------------------------------------------------------------

JavaVM* GetJavaVM()
{
	return g_JavaVM;
}

JNIEnv* GetEnv()
{
	JavaVM* vm = g_JavaVM;
	if (!vm)
		return 0;

	JNIEnv* env(0);
	vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
	return env;
}

JNIEnv* AttachCurrentThread()
{
	JavaVM* vm = g_JavaVM;
	if (!vm)
		return 0;

	JNIEnv* env = GetEnv();
	if (!env)
	{
		JavaVMAttachArgs args;
		args.version = JNI_VERSION_1_6;
		args.name = NULL;
		args.group = NULL;
	#if !ANDROID
		vm->AttachCurrentThread(reinterpret_cast<void**>(&env), &args);
	#else
		vm->AttachCurrentThread(&env, &args);
	#endif
	}

	if (!env)
		SetError(kJNI_ATTACH_FAILED, "java.lang.IllegalThreadStateException: Unable to attach to VM");

	return env;
}

void DetachCurrentThread()
{
	JavaVM* vm = g_JavaVM;
	if (!vm)
		return;

	vm->DetachCurrentThread();
}

jclass FindClass(const char* name)
{
	JNI_CALL_RETURN(jclass, name, true, env->FindClass(name));
}

jint Throw(jthrowable object)
{
	JNI_CALL_RETURN(jint, object, true, env->Throw(object));
}

jint ThrowNew(jclass clazz, const char* message)
{
	JNI_CALL_RETURN(jint, clazz && message, true, env->ThrowNew(clazz, message));
}

void FatalError(const char* str)
{
	JNI_CALL(str, false, env->FatalError(str));
}

jobject NewLocalRef(jobject object)
{
	JNI_CALL_RETURN(jobject, object, true, env->NewLocalRef(object));
}

void DeleteLocalRef(jobject object)
{
	JNI_CALL(object, false, env->DeleteLocalRef(object));
}

jobject NewGlobalRef(jobject object)
{
	JNI_CALL_RETURN(jobject, object, true, env->NewGlobalRef(object));
}

void DeleteGlobalRef(jobject object)
{
	JNI_CALL(object, false, env->DeleteGlobalRef(object));
}

jobject NewWeakGlobalRef(jobject object)
{
	JNI_CALL_RETURN(jobject, object, true, env->NewWeakGlobalRef(object));
}

void DeleteWeakGlobalRef(jobject object)
{
	JNI_CALL(object, false, env->DeleteWeakGlobalRef(object));
}

jclass GetObjectClass(jobject object)
{
	JNI_CALL_RETURN(jclass, object, true, env->GetObjectClass(object));
}

jboolean IsInstanceOf(jobject object, jclass clazz)
{
	JNI_CALL_RETURN(jboolean, object && clazz, true, env->IsInstanceOf(object, clazz));
}

jmethodID GetMethodID(jclass clazz, const char* name, const char* signature)
{
	JNI_CALL_RETURN(jmethodID, clazz && name && signature, true, env->GetMethodID(clazz, name, signature));
}

jfieldID GetFieldID(jclass clazz, const char* name, const char* signature)
{
	JNI_CALL_RETURN(jfieldID, clazz && name && signature, true, env->GetFieldID(clazz, name, signature));
}

jmethodID GetStaticMethodID(jclass clazz, const char* name, const char* signature)
{
	JNI_CALL_RETURN(jmethodID, clazz && name && signature, true, env->GetStaticMethodID(clazz, name, signature));
}

jfieldID GetStaticFieldID(jclass clazz, const char* name, const char* signature)
{
	JNI_CALL_RETURN(jfieldID, clazz && name && signature, true, env->GetStaticFieldID(clazz, name, signature));
}

jobject NewObject(jclass clazz, jmethodID methodID, ...)
{
	va_list args;
	va_start(args, methodID);
	JNI_CALL_DECLARE(jobject, result, clazz && methodID, true, env->NewObjectV(clazz, methodID, args));
	va_end(args);
	return result;
}

jstring NewStringUTF(const char* str)
{
	JNI_CALL_RETURN(jstring, str, true, env->NewStringUTF(str));
}

jsize GetStringUTFLength(jstring string)
{
	JNI_CALL_RETURN(jsize, string, true, env->GetStringUTFLength(string));
}

const char* GetStringUTFChars(jstring str, jboolean* isCopy)
{
	JNI_CALL_RETURN(const char*, str, true, env->GetStringUTFChars(str, isCopy));
}

void ReleaseStringUTFChars(jstring str, const char* utfchars)
{
	JNI_CALL(str && utfchars, false, env->ReleaseStringUTFChars(str, utfchars));
}

size_t GetArrayLength(jarray obj)
{
	JNI_CALL_RETURN(size_t, obj, true, env->GetArrayLength(obj));
}

jobjectArray NewObjectArray(jsize length, jclass elementClass, jobject initialElement)
{
	JNI_CALL_RETURN(jobjectArray, elementClass, true, env->NewObjectArray(length, elementClass, initialElement));
}

jobject GetObjectArrayElement(jobjectArray obj, size_t index)
{
	JNI_CALL_RETURN(jobject, obj, true, env->GetObjectArrayElement(obj, index));
}

void SetObjectArrayElement(jobjectArray obj, size_t index, jobject val)
{
	JNI_CALL(obj, true, env->SetObjectArrayElement(obj, index, val));
}

// --------------------------------------------------------------------------------------
// ThreadScope
// --------------------------------------------------------------------------------------
ThreadScope::ThreadScope()
{
	m_NeedDetach = !jni::GetEnv();
}

ThreadScope::~ThreadScope()
{
	if (m_NeedDetach)
		jni::DetachCurrentThread();
}

// --------------------------------------------------------------------------------------
// LocalFrame
// --------------------------------------------------------------------------------------
LocalFrame::LocalFrame(jint capacity)
{
	if (PushLocalFrame(capacity) < 0)
		FatalError("Out of memory: Unable to allocate local frame(64)");
	m_FramePushed = (PeekError() == kJNI_NO_ERROR);
}
LocalFrame::~LocalFrame()
{
	if (m_FramePushed)
		PopLocalFrame(NULL);
}

}
