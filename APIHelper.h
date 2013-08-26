#pragma once
#include "JNIBridge.h"

namespace jni
{

template <typename T>
class GlobalRef
{
public:
	GlobalRef(T object) { m_Ref = new Reference(object); }
	GlobalRef(const GlobalRef<T>& o) { Aquire(o.m_Ref); }
	~GlobalRef() { Release(); }

	inline operator T() const	{ return *m_Ref; }
	GlobalRef<T>& operator = (const GlobalRef<T>& o)
	{
		if (m_Ref == o.m_Ref)
			return *this;

		Release();
		Aquire(o.m_Ref);

		return *this;
	}

private:
	class Reference
	{
	public:
		Reference(T object)
		{
			m_Object = static_cast<T>(object ? jni::NewGlobalRef(object) : 0);
			m_Counter = 1;			
		}
		~Reference()
		{
			if (m_Object)
				jni::DeleteGlobalRef(m_Object);
			m_Object = 0;			
		}

		inline operator T() const { return m_Object; }
		void Aquire() { __sync_add_and_fetch(&m_Counter, 1); }
		bool Release() { return __sync_sub_and_fetch(&m_Counter, 1); }

	private:
		T      m_Object;
		volatile int m_Counter;
	};

	void Aquire(Reference* ref)
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
	class Reference* m_Ref;
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
	char*             m_ClassName;
	GlobalRef<jclass> m_Class;
};

class Object
{
public:
	explicit inline Object(jobject obj) : m_Object(obj) {}

	inline operator bool() const	{ return m_Object != 0; }
	inline operator jobject() const	{ return m_Object; }

protected:
	GlobalRef<jobject> m_Object;
};


template <typename T>
class ArrayBase
{
public:
	explicit ArrayBase(T obj) : m_Array(obj) {}

	inline size_t Length() const { return m_Array != 0 ? jni::GetArrayLength(m_Array) : 0; }

	inline operator bool() const { return m_Array != 0; }
	inline operator T() const { return m_Array; }

protected:
	GlobalRef<T> m_Array;
};

template <typename T, typename AT>
class PrimitiveArrayBase : public ArrayBase<AT>
{
public:
	explicit PrimitiveArrayBase(AT        obj) : ArrayBase<AT>(obj) {};
	explicit PrimitiveArrayBase(size_t length) : ArrayBase<AT>(jni::Op<T>::NewArray(length)) {};

	inline T operator[] (const int i)
	{
		T value = 0;
		if (*this)
			jni::Op<T>::GetArrayRegion(*this, i, 1, &value);
		return value;
	}

	inline T* Lock()
	{
		return *this ? jni::Op<T>::GetArrayElements(*this) : 0;
	}
	inline void Release(T* elements)
	{
		if (*this)
			jni::Op<T>::ReleaseArrayElements(*this, elements, 0);
	}
};


template <typename T>
class Array : public ArrayBase<jobjectArray>
{
public:
	explicit inline Array(jobjectArray obj) : ArrayBase<jobjectArray>(obj) {};
	explicit inline Array(size_t length, T initialElement = 0) : ArrayBase<jobjectArray>(jni::NewObjectArray(length, T::__CLASS, initialElement)) {};

	inline T operator[] (const int i) { return T(*this ? jni::GetObjectArrayElement(*this, i) : 0); }

	inline T* Lock()
	{
		T* elements = malloc(Length() * sizeof(T));
		for (int i = 0; i < Length(); ++i)
			elements[i] = T(jni::GetObjectArrayElement(*this, i));
		return elements;
	}
	inline void Release(T* elements)
	{
		for (int i = 0; i < Length(); ++i)
			jni::SetObjectArrayElement(*this, i, elements[i]);
		free(elements);
	}
};

#define DEF_PRIMITIVE_ARRAY_TYPE(t) \
template <> \
class Array<t> : public PrimitiveArrayBase<t, t##Array> \
{ \
public: \
	explicit inline Array(t##Array obj) : PrimitiveArrayBase<t, t##Array>(obj) {}; \
	explicit inline Array(size_t length) : PrimitiveArrayBase<t, t##Array>(jni::Op<t>::NewArray(length)) {}; \
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

template <typename T>
inline T Cast(const jni::Object& o)
{
	return T(jni::IsInstanceOf(o, T::__CLASS) ? (jobject) o : 0);
}

template <typename T>
inline T ExceptionOccurred()
{
	return T(ExceptionOccurred(T::CLASS));
}

}
