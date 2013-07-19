
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

	final Set<Class> m_VisitedClasses = new LinkedHashSet<Class>();
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

		APIGenerator generator = new APIGenerator();
		generator.collectDependencies(new JarFile(jar), args);
		generator.print(dst);
	}

	public void collectDependencies(JarFile file, LinkedList<String> args) throws Exception
	{
		URLClassLoader customClasses = new URLClassLoader(new URL[]{new File(file.getName()).toURI().toURL()}, null);
		Enumeration<JarEntry> entries = file.entries();
		while (entries.hasMoreElements())
		{
			String name = entries.nextElement().getName();
			if (name.endsWith(".class"))
			{
				String className = name.substring(0, name.length() - 6).replace("/", ".");
				String cppClassName = className.replace(".", "::").replace("$", "_");
				for (String arg : args)
				{
					if (Pattern.matches(arg, cppClassName))
					{
						int nClasses = m_DependencyChain.size();
						collectDependencies(customClasses.loadClass(className));
						System.err.format("[%d][%d]\t%s\n", m_DependencyChain.size(), m_DependencyChain.size() - nClasses, cppClassName);
						break;						
					}
				}
			}
		}
		args.clear();
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
			return "jarray";
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
	private boolean isStaticFinal(Member member)	{ return isStatic(member) && (Modifier.FINAL & member.getModifiers()) != 0; }
	private boolean isPublic(Member member)			{ return (Modifier.PUBLIC & member.getModifiers()) != 0; }
	private boolean isValid(Member member)			{ return isPublic(member) && !member.isSynthetic(); }
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

	private void print(String dst) throws Exception
	{
		// Implement classes
		for (Class clazz : m_VisitedClasses)
		{
			PrintStream source = new PrintStream(new FileOutputStream(new File(dst, clazz.getCanonicalName() + ".cpp")));
			source.format("#include \"API.h\"\n");
			implementClass(source, clazz);
			source.close();
		}

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
		header.format("};\n\n");
	}

	private void declareClassMembers(PrintStream out, Class clazz) throws Exception
	{
		File templateFile = new File("templates", clazz.getName() + ".h");
		boolean hasTemplate  = templateFile.exists();

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
		}
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

		closeNameSpace(out, namespace);
	}

	private void implementClassMembers(PrintStream out, Class clazz) throws Exception
	{
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
		}

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