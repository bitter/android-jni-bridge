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

}
