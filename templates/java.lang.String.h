String(const char* str);
~String();

String& operator = (const String& other);
bool EmptyOrNull();
operator jstring () const;
operator const char* ();

private:
	const char* m_Str;