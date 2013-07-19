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
	inline Object(const Object& obj) : m_Object(obj.m_Object) {}

	inline operator bool() const	{ return m_Object != 0; }
	inline operator jobject() const	{ return m_Object; }

protected:
	GlobalRef<jobject> m_Object;
};


template <typename T>
class Array
{
public:
	explicit inline Array(jarray obj) : m_Array(obj) {}
	inline Array(const Array& obj) : m_Array(obj.m_Array) {}

	inline operator jobject() const { return m_Array; }
	inline operator jarray() const { return m_Array; }

protected:
	GlobalRef<jarray> m_Array;
};

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
