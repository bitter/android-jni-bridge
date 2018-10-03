// --------------------------------------------------------
// API-21 moved these functions to BaseBundle but that
// class is not available in earlier versions of android
// --------------------------------------------------------
// Copied from android::os::BaseBundle
// --------------------------------------------------------
::java::lang::Object Get(const ::java::lang::String& arg0) const;
::jboolean GetBoolean(const ::java::lang::String& arg0) const;
::jboolean GetBoolean(const ::java::lang::String& arg0, const ::jboolean& arg1) const;
::jvoid PutBoolean(const ::java::lang::String& arg0, const ::jboolean& arg1) const;
::jint GetInt(const ::java::lang::String& arg0) const;
::jint GetInt(const ::java::lang::String& arg0, const ::jint& arg1) const;
::jvoid PutInt(const ::java::lang::String& arg0, const ::jint& arg1) const;
::jlong GetLong(const ::java::lang::String& arg0, const ::jlong& arg1) const;
::jlong GetLong(const ::java::lang::String& arg0) const;
::jvoid PutLong(const ::java::lang::String& arg0, const ::jlong& arg1) const;
::jdouble GetDouble(const ::java::lang::String& arg0) const;
::jdouble GetDouble(const ::java::lang::String& arg0, const ::jdouble& arg1) const;
::jvoid PutDouble(const ::java::lang::String& arg0, const ::jdouble& arg1) const;
::jboolean IsEmpty() const;
::jint Size() const;
::jvoid PutAll(const ::android::os::PersistableBundle& arg0) const;
::java::util::Set KeySet() const;
::jboolean ContainsKey(const ::java::lang::String& arg0) const;
::java::lang::String GetString(const ::java::lang::String& arg0, const ::java::lang::String& arg1) const;
::java::lang::String GetString(const ::java::lang::String& arg0) const;
jni::Array< ::jlong > GetLongArray(const ::java::lang::String& arg0) const;
::jvoid PutString(const ::java::lang::String& arg0, const ::java::lang::String& arg1) const;
::jvoid PutLongArray(const ::java::lang::String& arg0, const jni::Array< ::jlong >& arg1) const;
::jvoid PutStringArray(const ::java::lang::String& arg0, const jni::Array< ::java::lang::String >& arg1) const;
jni::Array< ::jint > GetIntArray(const ::java::lang::String& arg0) const;
::jvoid PutIntArray(const ::java::lang::String& arg0, const jni::Array< ::jint >& arg1) const;
::jvoid PutBooleanArray(const ::java::lang::String& arg0, const jni::Array< ::jboolean >& arg1) const;
::jvoid PutDoubleArray(const ::java::lang::String& arg0, const jni::Array< ::jdouble >& arg1) const;
jni::Array< ::jboolean > GetBooleanArray(const ::java::lang::String& arg0) const;
jni::Array< ::jdouble > GetDoubleArray(const ::java::lang::String& arg0) const;
jni::Array< ::java::lang::String > GetStringArray(const ::java::lang::String& arg0) const;