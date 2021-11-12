Number(Number&& o) = default;
Number& operator=(const Number& o) = default;
Number& operator=(Number&& o) = default;
operator ::jbyte() const;
operator ::jshort() const;
operator ::jint() const;
operator ::jlong() const;
operator ::jfloat() const;
operator ::jdouble() const;
