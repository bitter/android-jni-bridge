void Number::__Initialize() { }

Number::operator ::jbyte() const { return ByteValue(); }
Number::operator ::jshort() const { return ShortValue(); }
Number::operator ::jint() const { return IntValue(); }
Number::operator ::jlong() const { return LongValue(); }
Number::operator ::jfloat() const { return FloatValue(); }
Number::operator ::jdouble() const { return DoubleValue(); }

