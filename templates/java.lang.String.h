String(const char* str);
~String();

String& operator = (const String& other);
bool EmptyOrNull();

const char* c_str();

operator jstring () const;

private:
	const char* m_Str;