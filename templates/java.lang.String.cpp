String::String(const char* str) : ::java::lang::Object(str ? jni::NewStringUTF(str) : NULL) { __Initialize(); }
String::~String()
{
	if (m_Str)
		jni::ReleaseStringUTFChars(*this, m_Str);
	m_Str = 0;
}

void String::__Initialize()
{
	m_Str = 0;
}

String::operator jstring () const
{
	return (jstring)(jobject)m_Object;
}

String& String::operator = (const String& other)
{
	if (m_Object == other.m_Object)
		return *this;

	if (m_Str)
		jni::ReleaseStringUTFChars(*this, m_Str);
	m_Str = 0;

	m_Object = other.m_Object;
	return *this;
}

String::operator const char* ()
{
	if (m_Object && !m_Str)
		m_Str = jni::GetStringUTFChars(*this);
	return m_Str;
}

bool String::EmptyOrNull()
{
	if (!m_Object)
		return true;
	const char* str = *this;
	return !str || !str[0];
}
