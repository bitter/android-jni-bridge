package bitter.jnibridge;

import java.lang.reflect.*;
import java.lang.invoke.*;

public class JNIBridge
{
	static native Object invoke(long ptr, Class clazz, Method method, Object[] args);
	static native void   delete(long ptr);

	static Object newInterfaceProxy(final long ptr, final Class[] interfaces)
	{
		return Proxy.newProxyInstance(JNIBridge.class.getClassLoader(), interfaces, new InterfaceProxy(ptr));
	}

	static void disableInterfaceProxy(final Object proxy)
	{
		if (proxy != null)
			((InterfaceProxy) Proxy.getInvocationHandler(proxy)).disable();
	}

	private static class InterfaceProxy implements InvocationHandler
	{
		private Object m_InvocationLock = new Object[0];
		private long m_Ptr;
		private Constructor<MethodHandles.Lookup> m_constructor;

		@SuppressWarnings("unused")
		public InterfaceProxy(final long ptr)
		{
			m_Ptr = ptr;

			try
			{
				m_constructor = MethodHandles.Lookup.class.getDeclaredConstructor(Class.class, Integer.TYPE);
				m_constructor.setAccessible(true);
			}
			// MethodHandles.Lookup was added in Android Oreo, we get NoClassDefFoundError on devices with older OS versions
			catch (NoClassDefFoundError e)
			{
				m_constructor = null;
			}
			catch (NoSuchMethodException e)
			{
				m_constructor = null;
			}
		}

		private Object invokeDefault(Object proxy, Throwable t, Method m, Object[] args) throws Throwable
		{
			if (args == null)
			{
				args = new Object[0];
			}
			Class<?> k = m.getDeclaringClass();
			MethodHandles.Lookup lookup = m_constructor.newInstance(k, MethodHandles.Lookup.PRIVATE);

			return lookup.in(k).unreflectSpecial(m, k).bindTo(proxy).invokeWithArguments(args);
		}

		public Object invoke(Object proxy, Method method, Object[] args) throws Throwable
		{
			synchronized (m_InvocationLock)
			{
				if (m_Ptr == 0)
					return null;

				try
				{
					return JNIBridge.invoke(m_Ptr, method.getDeclaringClass(), method, args);
				}
				catch (NoSuchMethodError e)
				{
					if (m_constructor == null)
					{
						System.err.println("JNIBridge error: Java interface default methods are only supported since Android Oreo");
						throw e;
					}
					// isDefault() is only available since API 24, instead use getModifiers to check if a method has default implementation
					if ((method.getModifiers() & Modifier.ABSTRACT) == 0)
						return invokeDefault(proxy, e, method, args);
					else
						throw e;
				}
			}
		}

		public void finalize()
		{
			synchronized (m_InvocationLock)
			{
				if (m_Ptr == 0)
					return;
				JNIBridge.delete(m_Ptr);
			}
		}

		public void disable()
		{
			synchronized (m_InvocationLock)
			{
				m_Ptr = 0;
			}
		}
	}
}