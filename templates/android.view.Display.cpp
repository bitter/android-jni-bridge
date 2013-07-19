void Display::__Initialize() { }

jint Display::__GetRawWidth() const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getRawWidth", "()I");
	return methodID != 0 ? jni::Op<jint>::CallMethod(m_Object, methodID) : 0;
}

jint Display::__GetRawHeight() const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getRawHeight", "()I");
	return methodID != 0 ? jni::Op<jint>::CallMethod(m_Object, methodID) : 0;
}
