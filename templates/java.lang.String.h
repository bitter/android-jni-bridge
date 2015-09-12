String(const char* str);
~String();

String& operator = (const String& other);
bool EmptyOrNull();

const char* c_str() { return static_cast<const char*>(*this); }

operator jstring () const;
operator const char* ();

private:
	const char* m_Str;