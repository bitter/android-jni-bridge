#pragma once

#include "JNIBridge.h"

namespace jni
{

class GlobalRefAllocator
{
public:
	static jobject Alloc(jobject o) { return jni::NewGlobalRef(o); }
	static void    Free(jobject o)  { return jni::DeleteGlobalRef(o); }
};

class WeakGlobalRefAllocator
{
public:
	static jobject Alloc(jobject o) { return jni::NewWeakGlobalRef(o); }
	static void    Free(jobject o)  { return jni::DeleteWeakGlobalRef(o); }
};

template <typename RefType, typename ObjType>
class Ref
{
public:
	Ref(ObjType object) { m_Ref = new RefCounter(object); }
	Ref(const Ref<RefType,ObjType>& o) { Aquire(o.m_Ref); }
	~Ref() { Release(); }

	inline operator ObjType() const	{ return *m_Ref; }
	Ref<RefType,ObjType>& operator = (const Ref<RefType,ObjType>& o)
	{
		if (m_Ref == o.m_Ref)
			return *this;

		Release();
		Aquire(o.m_Ref);

		return *this;
	}

private:
	class RefCounter
	{
	public:
		RefCounter(ObjType object)
		{
			m_Object = static_cast<ObjType>(object ? RefType::Alloc(object) : 0);
			m_Counter = 1;			
		}
		~RefCounter()
		{
			if (m_Object)
				RefType::Free(m_Object);
			m_Object = 0;			
		}

		inline operator ObjType() const { return m_Object; }
		void Aquire() { __sync_add_and_fetch(&m_Counter, 1); }
		bool Release() { return __sync_sub_and_fetch(&m_Counter, 1); }

	private:
		ObjType      m_Object;
		volatile int m_Counter;
	};

	void Aquire(RefCounter* ref)
	{
		m_Ref = ref;
		m_Ref->Aquire();
	}

	void Release()
	{
		if (!m_Ref->Release())
		{
			delete m_Ref;
			m_Ref = NULL;
		}
	}

private:
	class RefCounter* m_Ref;
};


class Class
{
public:
	Class(const char* name, jclass clazz = 0);
	~Class();

	inline operator jclass()
	{
		jclass result = m_Class;
		if (result)
			return result;
		return m_Class = jni::FindClass(m_ClassName);
	}

private:
	Class(const Class& clazz);
	Class& operator = (const Class& o);

private:
	char*       m_ClassName;
	Ref<GlobalRefAllocator, jclass> m_Class;
};

class Object
{
public:
	explicit inline Object(jobject obj) : m_Object(obj) { }

	inline operator bool() const	{ return m_Object != 0; }
	inline operator jobject() const	{ return m_Object; }

protected:
	Ref<GlobalRefAllocator, jobject> m_Object;
};


// ------------------------------------------------
// Utillities
// ------------------------------------------------
template <typename T> inline bool InstanceOf(jobject o) { return jni::IsInstanceOf(o, T::__CLASS); }
template <typename T> inline T Cast(jobject o) { return T(InstanceOf<T>(o) ? o : 0); }
template <typename T> inline bool Catch() { return jni::ExceptionThrown(T::__CLASS); }
template <typename T> inline bool ThrowNew(const char* message) { return jni::ThrowNew(T::__CLASS, message) == 0; }

// ------------------------------------------------	
// Array Support
// ------------------------------------------------
template <typename T>
class ArrayBase
{
protected:
	explicit ArrayBase(T obj)       : m_Array(obj) {}
	explicit ArrayBase(jobject obj) : m_Array(static_cast<T>(obj)) {}

public:
	inline size_t Length() const { return m_Array != 0 ? jni::GetArrayLength(m_Array) : 0; }

	inline operator bool() const { return m_Array != 0; }
	inline operator T() const { return m_Array; }

protected:
	Ref<GlobalRefAllocator, T> m_Array;
};

template <typename T, typename AT>
class PrimitiveArrayBase : public ArrayBase<AT>
{
protected:
	explicit PrimitiveArrayBase(AT obj)                      : ArrayBase<AT>(obj) {};
	explicit PrimitiveArrayBase(jobject obj)                 : ArrayBase<AT>(obj) {};
	explicit PrimitiveArrayBase(size_t length)               : ArrayBase<AT>(jni::Op<T>::NewArray(length)) {};
	template<typename T2>
	explicit PrimitiveArrayBase(size_t length, T2* elements) : ArrayBase<AT>(jni::Op<T>::NewArray(length))
	{
		T* array = jni::Op<T>::GetArrayElements(*this);
		for (int i = 0; i < length; ++i)
			array[i] = static_cast<T>(elements[i]);
		jni::Op<T>::ReleaseArrayElements(*this, array, 0);
	};

public:
	inline T operator[] (const int i) const
	{
		T value = 0;
		if (*this)
			jni::Op<T>::GetArrayRegion(*this, i, 1, &value);
		return value;
	}

	inline T* Lock() const
	{
		return *this ? jni::Op<T>::GetArrayElements(*this) : 0;
	}
	inline void Release(T* elements, bool writeBackData = true) const
	{
		if (*this)
			jni::Op<T>::ReleaseArrayElements(*this, elements, writeBackData ? 0 : JNI_ABORT);
	}

	inline T* LockCritical() const
	{
		return *this ? jni::GetPrimitiveArrayCritical(*this, NULL) : 0;
	}
	inline void ReleaseCritical(T* elements, bool writeBackData = true) const
	{
		if (*this)
			jni::ReleasePrimitiveArrayCritical(*this, elements, writeBackData ? 0 : JNI_ABORT);
	}
};

template <typename T>
class ObjectArray : public ArrayBase<jobjectArray>
{
protected:
	explicit inline ObjectArray(jobject obj)                                  : ArrayBase<jobjectArray>(obj) {};
	explicit inline ObjectArray(jobjectArray obj)                             : ArrayBase<jobjectArray>(obj) {};
	explicit inline ObjectArray(jclass type, size_t length, T initialElement) : ArrayBase<jobjectArray>(jni::NewObjectArray(length, type, initialElement)) {};
	template<typename T2>
	explicit inline ObjectArray(jclass type, size_t length, T2* elements)     : ArrayBase<jobjectArray>(jni::NewObjectArray(length, type, NULL))
	{
		for (int i = 0; i < length; ++i)
			jni::SetObjectArrayElement(*this, i, static_cast<T>(elements[i]));
	}

public:
	inline T operator[] (const int i) { return T(*this ? jni::GetObjectArrayElement(*this, i) : 0); }

	inline T* Lock()
	{
		T* elements = reinterpret_cast<T*>(malloc(Length() * sizeof(T)));
		for (int i = 0; i < Length(); ++i)
			new (&(elements[i])) T(jni::GetObjectArrayElement(*this, i));

		return elements;
	}
	inline void Release(T* elements)
	{
		for (int i = 0; i < Length(); ++i)
		{
			jni::SetObjectArrayElement(*this, i, elements[i]);
			elements[i].~T();
		}

		free(elements);
	}
};

template <typename T>
class Array : public ObjectArray<T>
{
public:
	explicit inline Array(jobject obj)                         : ObjectArray<T>(obj) {};
	explicit inline Array(jobjectArray obj)                    : ObjectArray<T>(obj) {};
	explicit inline Array(size_t length, T initialElement = 0) : ObjectArray<T>(T::__CLASS, length, initialElement) {};
	template<typename T2>
	explicit inline Array(size_t length, T2* elements)         : ObjectArray<T>(T::__CLASS, length, elements) {};
};

template <>
class Array<jobject> : public ObjectArray<jobject>
{
public:
	explicit inline Array(jobject obj)                                      : ObjectArray<jobject>(obj) {};
	explicit inline Array(jobjectArray obj)                                 : ObjectArray<jobject>(obj) {};
	template<typename T>
	explicit inline Array(jclass type, size_t length, T initialElement = 0) : ObjectArray<jobject>(type, length, initialElement) {};
	template<typename T2>
	explicit inline Array(jclass type, size_t length, T2* elements)         : ObjectArray<jobject>(type, length, elements) {};
};

#define DEF_PRIMITIVE_ARRAY_TYPE(t) \
template <> \
class Array<t> : public PrimitiveArrayBase<t, t##Array> \
{ \
public: \
	explicit inline Array(jobject   obj)               : PrimitiveArrayBase<t, t##Array>(obj) {}; \
	explicit inline Array(t##Array  obj)               : PrimitiveArrayBase<t, t##Array>(obj) {}; \
	explicit inline Array(size_t length)               : PrimitiveArrayBase<t, t##Array>(length) {}; \
	template<typename T2> \
	explicit inline Array(size_t length, T2* elements) : PrimitiveArrayBase<t, t##Array>(length, elements) {}; \
};

DEF_PRIMITIVE_ARRAY_TYPE(jboolean)
DEF_PRIMITIVE_ARRAY_TYPE(jint)
DEF_PRIMITIVE_ARRAY_TYPE(jshort)
DEF_PRIMITIVE_ARRAY_TYPE(jbyte)
DEF_PRIMITIVE_ARRAY_TYPE(jlong)
DEF_PRIMITIVE_ARRAY_TYPE(jfloat)
DEF_PRIMITIVE_ARRAY_TYPE(jdouble)
DEF_PRIMITIVE_ARRAY_TYPE(jchar)

#undef DEF_PRIMITIVE_ARRAY_TYPE

// ------------------------------------------------
// Proxy Support
// ------------------------------------------------
class ProxyInvoker
{
public:
	ProxyInvoker() {}
	virtual ~ProxyInvoker() {};
	virtual jobject __Invoke(jclass, jmethodID, jobjectArray) = 0;

public:
	static bool __Register();

protected:
	virtual ::jobject __ProxyObject() const = 0;

private:
	ProxyInvoker(const ProxyInvoker& proxy);
	ProxyInvoker& operator = (const ProxyInvoker& o);
};

}
