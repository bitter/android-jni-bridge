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

	ExpandingArray<GlobalClass*> GlobalClass::g_AllClasses;

	GlobalClass::GlobalClass(const char* name, jclass clazz)
	{
		g_AllClasses.Add(this);
		m_Class = new Class(name, clazz);
	}

	GlobalClass::~GlobalClass()
	{
		g_AllClasses.Remove(this);
		if (m_Class)
			delete m_Class;
	}

	void GlobalClass::CleanupAllClasses()
	{
		for (int i = 0; i < g_AllClasses.GetCount(); i++)
		{
			auto instance = g_AllClasses[i];
			delete instance->m_Class;
			instance->m_Class = nullptr;
		}
		g_AllClasses.Clear();
	}

}
