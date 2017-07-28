void Bundle::__Initialize() { }

// --------------------------------------------------------
// API-21 moved these functions to BaseBundle but that
// class is not available in earlier versions of android
// --------------------------------------------------------
// Copied from android::os::BaseBundle
// --------------------------------------------------------
::java::lang::Object Bundle::Get(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "get", "(Ljava/lang/String;)Ljava/lang/Object;");
	return ::java::lang::Object(jni::Op<jobject>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jboolean Bundle::GetBoolean(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getBoolean", "(Ljava/lang/String;)Z");
	return ::jboolean(jni::Op<jboolean>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jboolean Bundle::GetBoolean(const ::java::lang::String& arg0, const ::jboolean& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getBoolean", "(Ljava/lang/String;Z)Z");
	return ::jboolean(jni::Op<jboolean>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jvoid Bundle::PutBoolean(const ::java::lang::String& arg0, const ::jboolean& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putBoolean", "(Ljava/lang/String;Z)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jint Bundle::GetInt(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getInt", "(Ljava/lang/String;)I");
	return ::jint(jni::Op<jint>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jint Bundle::GetInt(const ::java::lang::String& arg0, const ::jint& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getInt", "(Ljava/lang/String;I)I");
	return ::jint(jni::Op<jint>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jvoid Bundle::PutInt(const ::java::lang::String& arg0, const ::jint& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putInt", "(Ljava/lang/String;I)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jlong Bundle::GetLong(const ::java::lang::String& arg0, const ::jlong& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getLong", "(Ljava/lang/String;J)J");
	return ::jlong(jni::Op<jlong>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jlong Bundle::GetLong(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getLong", "(Ljava/lang/String;)J");
	return ::jlong(jni::Op<jlong>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jvoid Bundle::PutLong(const ::java::lang::String& arg0, const ::jlong& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putLong", "(Ljava/lang/String;J)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jdouble Bundle::GetDouble(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getDouble", "(Ljava/lang/String;)D");
	return ::jdouble(jni::Op<jdouble>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jdouble Bundle::GetDouble(const ::java::lang::String& arg0, const ::jdouble& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getDouble", "(Ljava/lang/String;D)D");
	return ::jdouble(jni::Op<jdouble>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jvoid Bundle::PutDouble(const ::java::lang::String& arg0, const ::jdouble& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putDouble", "(Ljava/lang/String;D)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
::jboolean Bundle::IsEmpty() const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "isEmpty", "()Z");
	return ::jboolean(jni::Op<jboolean>::CallMethod(m_Object, methodID));
}
::jint Bundle::Size() const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "size", "()I");
	return ::jint(jni::Op<jint>::CallMethod(m_Object, methodID));
}
::jvoid Bundle::PutAll(const ::android::os::PersistableBundle& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putAll", "(Landroid/os/PersistableBundle;)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::java::util::Set Bundle::KeySet() const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "keySet", "()Ljava/util/Set;");
	return ::java::util::Set(jni::Op<jobject>::CallMethod(m_Object, methodID));
}
::jboolean Bundle::ContainsKey(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "containsKey", "(Ljava/lang/String;)Z");
	return ::jboolean(jni::Op<jboolean>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::java::lang::String Bundle::GetString(const ::java::lang::String& arg0, const ::java::lang::String& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getString", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	return ::java::lang::String(jni::Op<jobject>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
::java::lang::String Bundle::GetString(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
	return ::java::lang::String(jni::Op<jobject>::CallMethod(m_Object, methodID, (jobject)arg0));
}
jni::Array< ::jlong > Bundle::GetLongArray(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getLongArray", "(Ljava/lang/String;)[J");
	return jni::Array< ::jlong >(jni::Op<jlongArray>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jvoid Bundle::PutString(const ::java::lang::String& arg0, const ::java::lang::String& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putString", "(Ljava/lang/String;Ljava/lang/String;)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
::jvoid Bundle::PutLongArray(const ::java::lang::String& arg0, const jni::Array< ::jlong >& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putLongArray", "(Ljava/lang/String;[J)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
::jvoid Bundle::PutStringArray(const ::java::lang::String& arg0, const jni::Array< ::java::lang::String >& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putStringArray", "(Ljava/lang/String;[Ljava/lang/String;)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
jni::Array< ::jint > Bundle::GetIntArray(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getIntArray", "(Ljava/lang/String;)[I");
	return jni::Array< ::jint >(jni::Op<jintArray>::CallMethod(m_Object, methodID, (jobject)arg0));
}
::jvoid Bundle::PutIntArray(const ::java::lang::String& arg0, const jni::Array< ::jint >& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putIntArray", "(Ljava/lang/String;[I)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
::jvoid Bundle::PutBooleanArray(const ::java::lang::String& arg0, const jni::Array< ::jboolean >& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putBooleanArray", "(Ljava/lang/String;[Z)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
::jvoid Bundle::PutDoubleArray(const ::java::lang::String& arg0, const jni::Array< ::jdouble >& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "putDoubleArray", "(Ljava/lang/String;[D)V");
	return ::jvoid(jni::Op<jvoid>::CallMethod(m_Object, methodID, (jobject)arg0, (jobject)arg1));
}
jni::Array< ::jboolean > Bundle::GetBooleanArray(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getBooleanArray", "(Ljava/lang/String;)[Z");
	return jni::Array< ::jboolean >(jni::Op<jbooleanArray>::CallMethod(m_Object, methodID, (jobject)arg0));
}
jni::Array< ::jdouble > Bundle::GetDoubleArray(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getDoubleArray", "(Ljava/lang/String;)[D");
	return jni::Array< ::jdouble >(jni::Op<jdoubleArray>::CallMethod(m_Object, methodID, (jobject)arg0));
}
jni::Array< ::java::lang::String > Bundle::GetStringArray(const ::java::lang::String& arg0) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "getStringArray", "(Ljava/lang/String;)[Ljava/lang/String;");
	return jni::Array< ::java::lang::String >(jni::Op<jobjectArray>::CallMethod(m_Object, methodID, (jobject)arg0));
}
