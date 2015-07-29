void Object::__Initialize() { }

Object::operator const char* ()
{
	return static_cast<const char*>(ToString());
}

