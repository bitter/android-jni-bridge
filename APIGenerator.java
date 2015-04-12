
import java.lang.reflect.*;
import java.io.*;
import java.net.*;
import java.util.*;
import java.util.jar.*;
import java.util.regex.*;

public class APIGenerator
{	
	final static Set<String> KEYWORDS = new HashSet<String>(Arrays.asList(new String[] {
		"Assert", "asm", "namespace"
	}));

	static final Comparator<Class> CLASSNAME_COMPARATOR = new Comparator<Class>() {
		public int compare(Class lhs, Class rhs) { return lhs.getName().compareTo(rhs.getName()); }
	};

	final Set<Class> m_AllClasses = new TreeSet<Class>(CLASSNAME_COMPARATOR);
	final Set<Class> m_VisitedClasses = new TreeSet<Class>(CLASSNAME_COMPARATOR);
	final Set<Class> m_DependencyChain = new LinkedHashSet<Class>();

	public static void main(String[] argsArray) throws Exception
	{
		LinkedList<String> args = new LinkedList<String>(Arrays.asList(argsArray));
		if (args.size() < 2)
		{
			System.err.format("Usage: APIGenerator <dst> <jarfile> <regex...>\n");
			System.exit(1);
		}
		String dst = args.pollFirst();
		if (!new File(dst).isDirectory())
		{
			System.err.format("Usage: APIGenerator <dst> <jarfile> <regex...>\n");
			System.err.format("%s: is not a directory.\n", dst);
			System.exit(1);			
		}

		String jar = args.pollFirst();
		if (!new File(jar).isFile())
		{
			System.err.format("Usage: APIGenerator <dst> <jarfile> <regex...>\n");
			System.err.format("%s: is not a jar file.\n", jar);
			System.exit(1);			
		}

		// We need this to box proxy values
		args.add("::java::lang::Byte");
		args.add("::java::lang::Short");
		args.add("::java::lang::Integer");
		args.add("::java::lang::Long");
		args.add("::java::lang::Float");
		args.add("::java::lang::Double");
		args.add("::java::lang::Character");
		args.add("::java::lang::Boolean");

		APIGenerator generator = new APIGenerator();
		generator.collectDependencies(new JarFile(jar), args);
		generator.print(dst);
	}

	public void collectDependencies(JarFile file, LinkedList<String> args) throws Exception
	{
		System.err.format("Loading classes from '%s'\n", file.getName());
		URLClassLoader customClasses = new URLClassLoader(new URL[]{new File(file.getName()).toURI().toURL()}, null);
		Enumeration<JarEntry> entries = file.entries();
		while (entries.hasMoreElements())
		{
			String name = entries.nextElement().getName();
			if (name.endsWith(".class"))
			{
				try
				{
					m_AllClasses.add(customClasses.loadClass(name.substring(0, name.length() - 6).replace("/", ".")));
				} catch (Throwable ignore) {}
			}
		}
		System.err.format("Searching for candidates\n");
		for (String arg : args)
		{
			Pattern pattern = Pattern.compile(arg);
			for (Class clazz : m_AllClasses)
			{
				String cppClassName = getClassName(clazz);
				if (!pattern.matcher(cppClassName).matches())
					continue;
				int nClasses = m_DependencyChain.size();
				collectDependencies(clazz);
				System.err.format("[%d][%d]\t%s\n", m_DependencyChain.size(), m_DependencyChain.size() - nClasses, cppClassName);
				break;
			}
		}
	}

	public void collectDependencies(Class clazz) throws Exception
	{		
		clazz = collectDirectDependencies(clazz);
		if (clazz == null)
			return;

		if (m_VisitedClasses.contains(clazz))
			return;

		m_VisitedClasses.add(clazz);

		Class superClass = clazz.getSuperclass();
		if (superClass != null)
			collectDependencies(superClass);

		for (Class interfaceClass : clazz.getInterfaces())
			collectDependencies(interfaceClass);

		for (Field field : clazz.getDeclaredFields())
		{
			if (!isValid(field))
				continue;
			collectDependencies(field.getType());
		}

		for (Method method : clazz.getDeclaredMethods())
		{
			if (!isValid(method))
				continue;
			for (Class paramType : method.getParameterTypes())
				collectDependencies(paramType);
			collectDependencies(method.getReturnType());
		}

		for (Constructor constructor : clazz.getDeclaredConstructors())			
		{
			if (!isValid(constructor))
				continue;
			for (Class paramType : constructor.getParameterTypes())
				collectDependencies(paramType);
		}
	}

	private Class collectDirectDependencies(Class clazz) throws Exception
	{
		while (clazz.isArray())
			clazz = clazz.getComponentType();

		if (!isValid(clazz))
			return null;

		if (m_DependencyChain.contains(clazz))
			return clazz;

		Class superClass = clazz.getSuperclass();
		if (superClass != null)
			collectDirectDependencies(superClass);

		m_DependencyChain.add(clazz);

		return clazz;
	}

	private String safe(String name, Class clazz)
	{
		if (clazz != null && name.equals(getSimpleName(clazz)))
			name = "x" + name;
		while (KEYWORDS.contains(name))
			name = "x" + name;
		return name;
	}

	private String getMethodName(Method method)
	{
		String name = capitalize(method.getName().replace('$', '_'));
		return safe(name, method.getDeclaringClass());
	}

	private String getFieldName(Field field)
	{
		String name = "f" + capitalize(field.getName().replace('$', '_'));
		return safe(name, field.getDeclaringClass());
	}

	private String capitalize(String str)
	{
		char[] chars = str.toCharArray();
		chars[0] = Character.toUpperCase(chars[0]);
		return new String(chars);
	}

	private String getSimpleName(Class clazz)
	{
		if (clazz.isArray())
			return "Array< " + getClassName(clazz.getComponentType()) + " >";
		if (clazz.isPrimitive())
			return "j" + clazz.getSimpleName();
		String fullClassName = clazz.getName();
		return safe(fullClassName.substring(fullClassName.lastIndexOf('.') + 1), null).replace('$', '_');
	}

	private String getPrimitiveType(Class clazz)
	{
		if (clazz.isArray())
		{
			Class elementClazz = clazz.getComponentType();
			if (elementClazz.isPrimitive())
				return "j" + elementClazz.getSimpleName() + "Array";
			return "jobjectArray";
		}
		if (clazz.isPrimitive())
			return "j" + clazz.getSimpleName();
		return "jobject";
	}

	private String getNameSpace(Class clazz)
	{
		if (clazz.isPrimitive())
			return "";
		if (clazz.isArray())
			return "jni";
		String fullName = clazz.getName();
		String simpleName = getSimpleName(clazz);
		String packageName = fullName.substring(0, fullName.length() - simpleName.length() - 1);
		StringBuilder namespace = new StringBuilder("");
		String[] namespaceComponents = packageName.split("\\.");
		for (int i = 0; i < namespaceComponents.length; ++i)
		{
			namespace.append("::");
			namespace.append(safe(namespaceComponents[i], null));
		}
		return namespace.toString();
	}

	private String getSignature(Class clazz) throws Exception
	{
		if (clazz == null)
			return "V";
		if (clazz.isArray())
			return "[" + getSignature(clazz.getComponentType());
		if (clazz.isPrimitive())
		{
			if (clazz.equals(byte.class))	return "B";
			if (clazz.equals(short.class))	return "S";
			if (clazz.equals(int.class))	return "I";
			if (clazz.equals(long.class))	return "J";
			if (clazz.equals(float.class))	return "F";
			if (clazz.equals(double.class))	return "D";
			if (clazz.equals(char.class))	return "C";
			if (clazz.equals(boolean.class))return "Z";
			if (clazz.equals(void.class))	return "V";
			throw new Exception("Unknown primitive: " + clazz);
		}
		return "L" + clazz.getName().replace('.', '/') + ";";
	}

	private Class box(Class clazz)
	{
		if (clazz.isPrimitive())
		{
			if (clazz.equals(byte.class))	return Byte.class;
			if (clazz.equals(short.class))	return Short.class;
			if (clazz.equals(int.class))	return Integer.class;
			if (clazz.equals(long.class))	return Long.class;
			if (clazz.equals(float.class))	return Float.class;
			if (clazz.equals(double.class))	return Double.class;
			if (clazz.equals(char.class))	return Character.class;
			if (clazz.equals(boolean.class))return Boolean.class;
		}
		return clazz;
	}

	private String runtimeUnbox(Class clazz)
	{
		if (clazz.isPrimitive())
		{
			if (clazz.equals(byte.class))	return ".ByteValue()";
			if (clazz.equals(short.class))	return ".ShortValue()";
			if (clazz.equals(int.class))	return ".IntValue()";
			if (clazz.equals(long.class))	return ".LongValue()";
			if (clazz.equals(float.class))	return ".FloatValue()";
			if (clazz.equals(double.class))	return ".DoubleValue()";
			if (clazz.equals(char.class))	return ".CharValue()";
			if (clazz.equals(boolean.class))return ".BooleanValue()";
		}
		return "";
	}

	private String getSignature(Class... clazzes) throws Exception
	{
		StringBuilder signature = new StringBuilder();
		for (Class clazz : clazzes)
			signature.append(getSignature(clazz));
		return signature.toString();
	}

	private String getMethodSignature(Method method) throws Exception
	{
		return String.format("(%s)%s", getSignature(method.getParameterTypes()), getSignature(method.getReturnType()));
	}

	private String getConstructorSignature(Constructor constructor) throws Exception
	{
		return String.format("(%s)V", getSignature(constructor.getParameterTypes()));
	}

	private String getFieldSignature(Field field) throws Exception
	{
		return getSignature(field.getType());
	}

	private String getClassName(Class clazz)
	{
		StringBuilder buffer = new StringBuilder();
		buffer.append(getNameSpace(clazz));
		buffer.append("::");
		buffer.append(getSimpleName(clazz));
		return buffer.toString();
	}

	private String getSuperClassName(Class clazz)
	{
		Class superClass = clazz.getSuperclass();
		if (superClass == null)
			return "jni::Object";
		else
			return getClassName(superClass);
	}

	private boolean isValid(Class clazz)
	{
		return !(
			clazz == null ||
			clazz.isPrimitive() ||
			isAnonymous(clazz) ||
			"package-info".equals(clazz.getSimpleName())
		);
	}

	private boolean isInner(Class clazz)			{ return clazz.getEnclosingClass() != null; }
	private boolean isAnonymous(Class clazz)		{ return isInner(clazz) && clazz.getDeclaringClass() == null; }
	private boolean isStatic(Member member)			{ return (Modifier.STATIC & member.getModifiers()) != 0; }
	private boolean isFinal(Member member)			{ return (Modifier.FINAL & member.getModifiers()) != 0; }
	private boolean isStaticFinal(Member member)	{ return isStatic(member) && isFinal(member); }
	private boolean isPublic(Member member)			{ return (Modifier.PUBLIC & member.getModifiers()) != 0; }
	private boolean isProtected(Member member)		{ return (Modifier.PROTECTED & member.getModifiers()) != 0; }
	private boolean isValid(Member member)			{ return (isPublic(member) || isProtected(member)) && !member.isSynthetic(); }
	private boolean isValid(Method method)			{ return isValid((Member) method) && !method.isBridge(); }
	private boolean isValid(Constructor ctor, Class clazz)
	{
		return !(ctor.getParameterTypes().length == 1 && ctor.getParameterTypes()[0] == clazz)
			&& isValid((Member)ctor);
	}

	private String enterNameSpace(PrintStream out, String currentNameSpace, Class clazz)
	{
		String nameSpace = getNameSpace(clazz);
		if (nameSpace.equals(currentNameSpace))
			return currentNameSpace;

		closeNameSpace(out, currentNameSpace);

		for (String part : nameSpace.split("::"))
		{
			if (!part.isEmpty()) // ignore anonymous namespaces
			{
				out.print("namespace ");
				out.print(part);
				out.print(" { ");
			}
		}
		out.print("\n");
		return nameSpace;
	}

	private String closeNameSpace(PrintStream out, String currentNameSpace)
	{
		if (currentNameSpace != null)
		{
			for (String part : currentNameSpace.split("::"))
				if (!part.isEmpty()) // ignore anonymous namespaces
					out.print('}');
			out.print("\n\n");
		}
		return null;
	}

	private String getParameterSignature(Class<?>[] parameterTypes)
	{
		StringBuilder buffer = new StringBuilder();
		for (int i = 0; i < parameterTypes.length; ++i)
		{
			if (i > 0) buffer.append(", ");
			buffer.append("const ");
			buffer.append(getClassName(parameterTypes[i]));
			buffer.append("& arg");
			buffer.append(i);
		}
		return buffer.toString();
	}

	private String getParameterNames(int nParameters)
	{
		StringBuilder buffer = new StringBuilder();
		for (int i = 0; i < nParameters; ++i)
		{
			if (i > 0) buffer.append(", ");
			buffer.append("arg");
			buffer.append(i);
		}
		return buffer.toString();
	}

	private String getParameterJNINames(Class<?>[] parameterTypes)
	{
		StringBuilder buffer = new StringBuilder();
		for (int i = 0; i < parameterTypes.length; ++i)
		{
			buffer.append(", ");
			if (parameterTypes[i] != null && !parameterTypes[i].isPrimitive())
				buffer.append("(jobject)");
			buffer.append("arg");
			buffer.append(i);
		}
		return buffer.toString();
	}

	private String getParametersFromJNIObjectArray(Class<?>[] parameterTypes)
	{
		StringBuilder buffer = new StringBuilder();
		for (int i = 0; i < parameterTypes.length; ++i)
		{
			if (i > 0)
				buffer.append(", ");
			buffer.append(getClassName(box(parameterTypes[i])));
			buffer.append("(jni::GetObjectArrayElement(args, ");
			buffer.append(i);
			buffer.append("))");
			buffer.append(runtimeUnbox(parameterTypes[i]));
		}
		return buffer.toString();
	}

	private void print(String dst) throws Exception
	{
		System.err.println("Generating cpp code");
		// Implement classes
		for (Class clazz : m_VisitedClasses)
		{
			PrintStream source = new PrintStream(new FileOutputStream(new File(dst, clazz.getCanonicalName() + ".cpp")));
			source.format("#include \"API.h\"\n");
			implementClass(source, clazz);
			source.close();
		}

		System.err.println("Creating header file");
		PrintStream header = new PrintStream(new FileOutputStream(new File(dst, "API.h")));
		header.format("#pragma once\n");
		header.format("#include \"APIHelper.h\"\n");

		// Declare classes
		String currentNameSpace = null;
		for (Class clazz : m_VisitedClasses)
		{
			currentNameSpace = enterNameSpace(header, currentNameSpace, clazz);
			header.format("struct %s;\n", getSimpleName(clazz));
		}

		// Define classes
		for (Class clazz : m_DependencyChain)
		{
			currentNameSpace = enterNameSpace(header, currentNameSpace, clazz);
			declareClass(header, clazz);			
		}
		currentNameSpace = closeNameSpace(header, currentNameSpace);
		header.close();
	}

	private void declareClass(PrintStream header, Class clazz) throws Exception
	{
		header.format("struct ");
		header.format("%s : %s", getSimpleName(clazz), getSuperClassName(clazz));
		header.format("\n{\n");
		header.format("\tstatic jni::Class __CLASS;\n\n");

		// Use cast operators for interfaces to avoid deadly diamond of death
		for (Class interfaze : clazz.getInterfaces())
			header.format("\toperator %s();\n", getClassName(interfaze));

		declareClassMembers(header, clazz);

		if (clazz.isInterface())
			declareProxy(header, clazz);

		header.format("};\n\n");
	}

	private void declareProxy(PrintStream header, Class clazz) throws Exception
	{
		header.format("\tstruct Proxy : private jni::Proxy\n");
		header.format("\t{\n");
		header.format("\t\toperator %s();\n", getClassName(clazz));
		// Use cast operators for interfaces to avoid deadly diamond of death
		for (Class interfaze : clazz.getInterfaces())
			header.format("\t\toperator %s();\n", getClassName(interfaze));
		declareProxyMembers(header, clazz);
		header.format("\t};\n");
	}

	private void declareProxyMembers(PrintStream out, Class clazz) throws Exception
	{
		out.format("\tprotected:\n");
		out.format("\t\tvirtual jobject __Invoke(jmethodID, jobjectArray);\n");
		for (Method method : clazz.getDeclaredMethods())
		{
			if (!isValid(method) || isStatic(method))
				continue;
			out.format("\t\tvirtual %s %s(%s) = 0;\n",
				method.getReturnType() == void.class ? "void" : getClassName(method.getReturnType()),
				getMethodName(method),
				getParameterSignature(method.getParameterTypes()));
		}
		out.format("\t\tProxy();\n");
	}

	private void declareClassMembers(PrintStream out, Class clazz) throws Exception
	{
		File templateFile = new File("templates", clazz.getName() + ".h");
		boolean hasTemplate  = templateFile.exists();

/* example ------------------
	static ::java::util::Comparator& fCASE_INSENSITIVE_ORDER();
*/
		for (Field field : clazz.getDeclaredFields())
		{
			if (!isValid(field))
				continue;
			out.format("\t%s%s%s %s()%s;\n",
				isStatic(field) ? "static " : "",
				getClassName(field.getType()),
				isStaticFinal(field) ? "&" : "",
				getFieldName(field),
				isStatic(field) ? "" : " const");

			if (isFinal(field))
				continue;
			out.format("\t%svoid %s(%s)%s;\n",
				isStatic(field) ? "static " : "",
				getFieldName(field),
				getParameterSignature(new Class[] {field.getType()}),
				isStatic(field) ? "" : " const");
		}
/* example ------------------
	jni::Array< ::java::lang::String > Split(const ::java::lang::String& arg0, const ::jint& arg1) const;
*/
		for (Method method : clazz.getDeclaredMethods())
		{
			if (!isValid(method))
				continue;
			out.format("\t%s%s %s(%s)%s;\n",
				isStatic(method) ? "static " : "",
				getClassName(method.getReturnType()),
				getMethodName(method),
				getParameterSignature(method.getParameterTypes()),
				isStatic(method) ? "" : " const");
		}
/* example ------------------
	static jobject __Constructor(const jni::Array< ::jchar >& arg0, const ::jint& arg1, const ::jint& arg2);
	String(const jni::Array< ::jchar >& arg0, const ::jint& arg1, const ::jint& arg2) : ::java::lang::Object(__Constructor(arg0, arg1, arg2)) { __Initialize(); }
*/
		for (Constructor constructor : clazz.getDeclaredConstructors())
		{
			if (!isValid(constructor, clazz))
				continue;
			Class[] params = constructor.getParameterTypes();
			out.format("\tstatic jobject __Constructor(%s);\n", getParameterSignature(params));
			out.format("\t%s(%s) : %s(__Constructor(%s)) {%s}\n",
				getSimpleName(clazz),
				getParameterSignature(params),
				getSuperClassName(clazz),
				getParameterNames(params.length),
				hasTemplate ? " __Initialize(); " : "");
		}
		// Standard constructors
		out.format("\texplicit %s(jobject o) : %s(o) {%s}\n",
			getSimpleName(clazz),
			getSuperClassName(clazz),
			hasTemplate ? " __Initialize(); " : "");
		out.format("\t%s(const %s& o)  : %s(o) {%s}\n",
			getSimpleName(clazz),
			getSimpleName(clazz),
			getSuperClassName(clazz),
			hasTemplate ? " __Initialize(); " : "");
		if (hasTemplate)
		{
			out.format("%s\n",	new Scanner(templateFile).useDelimiter("\\Z").next());
			out.format("private:\n");
			out.format("\tvoid __Initialize();\n");
		}
		out.format("\n");
	}

	private void implementClass(PrintStream out, Class clazz) throws Exception
	{
		String namespace = enterNameSpace(out, null, clazz);
		out.format("jni::Class %s::__CLASS(\"%s\");\n\n", getSimpleName(clazz), clazz.getName().replace('.', '/'));

		for (Class interfaze : clazz.getInterfaces())
			out.format("%s::operator %s() { return %s((jobject)*this); }\n", getSimpleName(clazz), getClassName(interfaze), getClassName(interfaze));

		implementClassMembers(out, clazz);

		// Apply template
		File tempalteFile = new File("templates", clazz.getName() + ".cpp");
		if (tempalteFile.exists())
			out.format("%s\n",	new Scanner(tempalteFile).useDelimiter("\\Z").next());

		if (clazz.isInterface())
			implementProxy(out, clazz);

		closeNameSpace(out, namespace);
	}

	private void implementProxy(PrintStream out, Class clazz) throws Exception
	{
		out.format("%s::Proxy::Proxy() : jni::Proxy(%s::__CLASS) {}\n", getSimpleName(clazz), getSimpleName(clazz));
		out.format("%s::Proxy::operator %s() { return %s(static_cast<jobject>(m_Object)); }\n", getSimpleName(clazz), getSimpleName(clazz), getSimpleName(clazz));
		for (Class interfaze : clazz.getInterfaces())
			out.format("%s::Proxy::operator %s() { return %s(static_cast<jobject>(m_Object)); }\n", getSimpleName(clazz), getClassName(interfaze), getClassName(interfaze));
		out.format("jobject %s::Proxy::__Invoke(jmethodID methodID, jobjectArray args) {\n", getSimpleName(clazz));

		int nMethods = 0;
		Method[] methods = clazz.getDeclaredMethods();
		for (Method method : methods)
		{
			if (!isValid(method) || isStatic(method))
				continue;
			++nMethods;
		}

		if (nMethods == 0) { out.format("\treturn 0;\n}"); return; }

		out.format("\tstatic jmethodID methodIDs[] = {\n");
		for (Method method : methods)
		{
			if (!isValid(method) || isStatic(method))
				continue;
			out.format("\t\tjni::GetMethodID(__CLASS, \"%s\", \"%s\"),\n", method.getName(), getMethodSignature(method));
		}
		out.format("\t};\n");
		int i = 0;
		for (Method method : methods)
		{
			if (!isValid(method) || isStatic(method))
				continue;
			Class returnType = method.getReturnType();
			Class[] params = method.getParameterTypes();
			if (returnType != void.class)
				out.format("\tif (methodIDs[%d] == methodID) { return static_cast< %s >(%s(%s)); }\n",
					i++,
					getClassName(box(returnType)),
					getMethodName(method),
					getParametersFromJNIObjectArray(params));
			else
				out.format("\tif (methodIDs[%d] == methodID) { %s(%s); return 0; }\n",
					i++,
					getMethodName(method),
					getParametersFromJNIObjectArray(params));
		}
		// put warning here?
		out.format("\treturn 0;\n}");
	}

	private void implementClassMembers(PrintStream out, Class clazz) throws Exception
	{
/* example ------------------
::java::util::Comparator& String::fCASE_INSENSITIVE_ORDER()
{
	static jfieldID fieldID = jni::GetStaticFieldID(__CLASS, "CASE_INSENSITIVE_ORDER", "Ljava/util/Comparator;");
	static ::java::util::Comparator val = ::java::util::Comparator(jni::Op<jobject>::GetStaticField(__CLASS, fieldID));
	return val;
}
*/
		for (Field field : clazz.getDeclaredFields())
		{
			if (!isValid(field))
				continue;
			out.format("%s%s %s::%s()%s\n",
				getClassName(field.getType()),
				isStaticFinal(field) ? "&" : "",
				getSimpleName(clazz),
				getFieldName(field),
				isStatic(field) ? "" : " const");
			out.format("{\n");
			out.format("\tstatic jfieldID fieldID = jni::Get%sFieldID(__CLASS, \"%s\", \"%s\");\n",
				isStatic(field) ? "Static" : "",
				field.getName(),
				getFieldSignature(field));
			out.format("\t%s%s val = %s(jni::Op<%s>::Get%sField(%s, fieldID));\n",
				isStaticFinal(field) ? "static " : "",
				getClassName(field.getType()),
				getClassName(field.getType()),
				getPrimitiveType(field.getType()),
				isStatic(field) ? "Static" : "",
				isStatic(field) ? "__CLASS" : "m_Object");
			out.format("\treturn val;\n");
			out.format("}\n");

			if (isFinal(field))
				continue;
			out.format("void %s::%s(%s)%s\n",
				getSimpleName(clazz),
				getFieldName(field),
				getParameterSignature(new Class[] {field.getType()}),
				isStatic(field) ? "" : " const");
			out.format("{\n");
			out.format("\tstatic jfieldID fieldID = jni::Get%sFieldID(__CLASS, \"%s\", \"%s\");\n",
				isStatic(field) ? "Static" : "",
				field.getName(),
				getFieldSignature(field));
			out.format("\tjni::Op<%s>::Set%sField(%s, fieldID%s);\n",
				getPrimitiveType(field.getType()),
				isStatic(field) ? "Static" : "",
				isStatic(field) ? "__CLASS" : "m_Object",
				getParameterJNINames(new Class[] {field.getType()}));
			out.format("}\n");

		}

/* example ------------------
jni::Array< ::java::lang::String > String::Split(const ::java::lang::String& arg0, const ::jint& arg1) const
{
	static jmethodID methodID = jni::GetMethodID(__CLASS, "split", "(Ljava/lang/String;I)[Ljava/lang/String;");
	return jni::Array< ::java::lang::String >(jni::Op<jobjectArray>::CallMethod(m_Object, methodID, (jobject)arg0, arg1));
}
*/
		for (Method method : clazz.getDeclaredMethods())
		{
			if (!isValid(method))
				continue;
			Class[] params = method.getParameterTypes();
			out.format("%s %s::%s(%s)%s\n",
				getClassName(method.getReturnType()),
				getSimpleName(clazz),
				getMethodName(method),
				getParameterSignature(params),
				isStatic(method) ? "" : " const");
			out.format("{\n");
			out.format("\tstatic jmethodID methodID = jni::Get%sMethodID(__CLASS, \"%s\", \"%s\");\n",
				isStatic(method) ? "Static" : "",
				method.getName(),
				getMethodSignature(method));
			out.format("\treturn %s(jni::Op<%s>::Call%sMethod(%s, methodID%s));\n",
				getClassName(method.getReturnType()),
				getPrimitiveType(method.getReturnType()),
				isStatic(method) ? "Static" : "",
				isStatic(method) ? "__CLASS" : "m_Object",
				getParameterJNINames(params));
			out.format("}\n");
		}

/* example ------------------
jobject String::__Constructor(const jni::Array< ::jbyte >& arg0, const ::jint& arg1, const ::jint& arg2)
{
	static jmethodID constructorID = jni::GetMethodID(__CLASS, "<init>", "([BII)V");
	return jni::NewObject(__CLASS, constructorID, (jobject)arg0, arg1, arg2);
}
*/
		for (Constructor constructor : clazz.getDeclaredConstructors())
		{
			if (!isValid(constructor, clazz))
				continue;
			Class[] params = constructor.getParameterTypes();
			out.format("jobject %s::__Constructor(%s)\n", getSimpleName(clazz), getParameterSignature(params));
			out.format("{\n");
			out.format("\tstatic jmethodID constructorID = jni::GetMethodID(__CLASS, \"<init>\", \"%s\");\n",
				getConstructorSignature(constructor));
			out.format("\treturn jni::NewObject(__CLASS, constructorID%s);\n",
				getParameterJNINames(params));
			out.format("}\n");
		}
	}
}