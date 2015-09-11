#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "API.h"

using namespace java::lang;
using namespace java::io;
using namespace java::util;

int main(int,char**)
{
	JavaVM* vm;
	void* envPtr;
	JavaVMInitArgs vm_args;
	memset(&vm_args, 0, sizeof(vm_args));

	char classPath[] = {"-Djava.class.path=../build"};

	JavaVMOption options[1];
	options[0].optionString = classPath;

	vm_args.options = options;
	vm_args.nOptions = 1;
	vm_args.version = JNI_VERSION_1_6;
	JNI_CreateJavaVM(&vm, &envPtr, &vm_args);

	JNIEnv* env = (JNIEnv*)envPtr;

	jni::Initialize(*vm);

	// ------------------------------------------------------------------------
	// System.out.println("Hello world");
	// ------------------------------------------------------------------------

	// pure JNI
	{
		jstring   helloWorldString             = env->NewStringUTF("RAW");
		jclass    javaLangSystem               = env->FindClass("java/lang/System");
		jclass    javaIoPrintStream            = env->FindClass("java/io/PrintStream");
		jfieldID  javaLangSystem_outFID        = env->GetStaticFieldID(javaLangSystem, "out", "Ljava/io/PrintStream;");
		jobject   javaLangSystem_out           = env->GetStaticObjectField(javaLangSystem, javaLangSystem_outFID);
		jmethodID javaIoPrintStream_printlnMID = env->GetMethodID(javaIoPrintStream, "println", "(Ljava/lang/String;)V");
		env->CallVoidMethod(javaLangSystem_out, javaIoPrintStream_printlnMID, helloWorldString);
	}

	// JNI.h
	jni::Errno error;
	{
		jni::Initialize(*vm);
		if ((env = jni::AttachCurrentThread()))
		{
			jni::LocalFrame frame;
			jstring   helloWorldString             = env->NewStringUTF("JNI");
			jclass    javaLangSystem               = env->FindClass("java/lang/System");
			jclass    javaIoPrintStream            = env->FindClass("java/io/PrintStream");
			jfieldID  javaLangSystem_outFID        = env->GetStaticFieldID(javaLangSystem, "out", "Ljava/io/PrintStream;");
			jobject   javaLangSystem_out           = env->GetStaticObjectField(javaLangSystem, javaLangSystem_outFID);
			jmethodID javaIoPrintStream_printlnMID = env->GetMethodID(javaIoPrintStream, "println", "(Ljava/lang/String;)V");
			env->CallVoidMethod(javaLangSystem_out, javaIoPrintStream_printlnMID, helloWorldString);
			if ((error = jni::CheckError()))
				printf("JNI %s\n", jni::GetErrorMessage());
		}
	}

	// Ops.h
	{
		jni::LocalFrame frame;
		jstring   helloWorldString             = env->NewStringUTF("Ops");
		jclass    javaLangSystem               = env->FindClass("java/lang/System");
		jclass    javaIoPrintStream            = env->FindClass("java/io/PrintStream");
		jfieldID  javaLangSystem_outFID        = env->GetStaticFieldID(javaLangSystem, "out", "Ljava/io/PrintStream;");
		jobject   javaLangSystem_out           = jni::Op<jobject>::GetStaticField(javaLangSystem, javaLangSystem_outFID);
		jmethodID javaIoPrintStream_printlnMID = env->GetMethodID(javaIoPrintStream, "println", "(Ljava/lang/String;)V");
		jni::Op<jvoid>::CallMethod(javaLangSystem_out, javaIoPrintStream_printlnMID, helloWorldString);
		if ((error = jni::CheckError()))
			printf("Ops %d:%s\n", error, jni::GetErrorMessage());
	}

	{
		System::fOut().Println("Api");
	}

	// ------------------------------------------------------------------------
	// import java.io.PrintStream;
	// import java.util.Properties;
	// import java.util.Enumerator;
	//
	// PrintStream out       = System.out;
	// Properties properties = System.getPropertes();
	// Enumerator keys       = properties.keys();
	// while (keys.hasMoreElements())
	//     out.println(properties.getProperty((String)keys.next()));
	// ------------------------------------------------------------------------
	timeval start, stop;

	// Optimized version
	gettimeofday(&start, NULL);
	{
		jni::LocalFrame frame;
		PrintStream out       = System::fOut();
		Properties properties = System::GetProperties();
		Enumeration keys       = properties.Keys();
		while (keys.HasMoreElements())
			out.Println(properties.GetProperty(jni::Cast<String>(keys.NextElement())));
	}
	gettimeofday(&stop, NULL);
	printf("%f ms.\n", (stop.tv_sec - start.tv_sec) * 1000.0 + (stop.tv_usec - start.tv_usec) / 1000.0);

	// CharSequence test
	java::lang::CharSequence string = "hello world";
	printf("%s\n", static_cast<const char*>(string.ToString()));

	// -------------------------------------------------------------
	// Array Test
	// -------------------------------------------------------------
	{
		jni::LocalFrame frame;
		jni::Array<int> test01(4, (int[]){1, 2, 3, 4});
		for (int i = 0; i < test01.Length(); ++i)
			printf("ArrayTest01[%d],", test01[i]);
		printf("\n");

		jni::Array<java::lang::Integer> test02(4, (java::lang::Integer[]){1, 2, 3, 4});
		for (int i = 0; i < test02.Length(); ++i)
			printf("ArrayTest02[%d],", test02[i].IntValue());
		printf("\n");

		jni::Array<jobject> test03(java::lang::Integer::__CLASS, 4, (jobject[]){java::lang::Integer(1), java::lang::Integer(2), java::lang::Integer(3), java::lang::Integer(4)});
		for (int i = 0; i < test03.Length(); ++i)
			printf("ArrayTest03[%d],", java::lang::Integer(test03[i]).IntValue());
		printf("\n");

		jni::Array<jobject> test04(java::lang::Integer::__CLASS, 4, (java::lang::Integer[]){1, 2, 3, 4});
		for (int i = 0; i < test04.Length(); ++i)
			printf("ArrayTest04[%d],", java::lang::Integer(test04[i]).IntValue());
		printf("\n");

		jni::Array<int> test05(4, (java::lang::Integer[]){1, 2, 3, 4});
		for (int i = 0; i < test05.Length(); ++i)
			printf("ArrayTest05[%d],", test05[i]);
		printf("\n");

		jni::Array<java::lang::Integer> test10(4, 4733);
		for (int i = 0; i < test10.Length(); ++i)
			printf("ArrayTest10[%d],", test10[i].IntValue());
		printf("\n");

		jni::Array<jobject> test11(java::lang::Integer::__CLASS, 4, java::lang::Integer(4733));
		for (int i = 0; i < test11.Length(); ++i)
			printf("ArrayTest11[%d],", java::lang::Integer(test11[i]).IntValue());
		printf("\n");
	}

	// -------------------------------------------------------------
	// Proxy test
	// -------------------------------------------------------------
	if (!jni::ProxyInvoker::__Register())
		printf("%s\n", jni::GetErrorMessage());

	struct PretendRunnable : jni::Proxy<Runnable>
	{
		virtual void Run() {printf("%s\n", "hello world!!!!"); }
	};

	PretendRunnable pretendRunnable;
	Runnable runnable = pretendRunnable;

	Thread     thread(pretendRunnable);
	thread.Start();
	thread.Join();

	runnable.Run();

	// Make sure we don't get crashes from deleting the native object.
	PretendRunnable* pretendRunnable2 = new PretendRunnable;
	Runnable runnable2 = *pretendRunnable2;
	runnable2.Run();
	delete pretendRunnable2;
	runnable2.Run(); // <-- should not log anything

	// -------------------------------------------------------------
	// Performance Proxy Test
	// -------------------------------------------------------------
	struct PerformanceRunnable : jni::Proxy<Runnable>
	{
		int i;
		PerformanceRunnable() : i(0) {}
		virtual void Run() {  ++i; }
	};
	PerformanceRunnable* perfRunner = new PerformanceRunnable;
	Runnable perfRunnable = *perfRunner;
	gettimeofday(&start, NULL);
	for (int i = 0; i < 1024; ++i)
		perfRunnable.Run();
	gettimeofday(&stop, NULL);
	printf("count: %d, time: %f ms.\n", perfRunner->i, (stop.tv_sec - start.tv_sec) * 1000.0 + (stop.tv_usec - start.tv_usec) / 1000.0);

	delete perfRunner;
	gettimeofday(&start, NULL);
	for (int i = 0; i < 1024; ++i)
		perfRunnable.Run();
	gettimeofday(&stop, NULL);
	printf("count: %d, time: %f ms.\n", 1024, (stop.tv_sec - start.tv_sec) * 1000.0 + (stop.tv_usec - start.tv_usec) / 1000.0);

	// -------------------------------------------------------------
	// Weak Proxy Test
	// -------------------------------------------------------------
	struct KillMePleazeRunnable : jni::WeakProxy<Runnable>
	{
		virtual ~KillMePleazeRunnable() { printf("%s\n", "KillMePleazeRunnable");}
		virtual void Run() { }
	};

	{
		jni::LocalFrame frame;
		KillMePleazeRunnable* killMeRunnable = new KillMePleazeRunnable;
	}
	for (int i = 0; i < 32; ++i) // Do a couple of loops to massage the GC
	{
		jni::LocalFrame frame;
		jni::Array<int> array(1024*1024);
		System::Gc();
	}

	// -------------------------------------------------------------
	// Multiple Proxy Interface Test
	// -------------------------------------------------------------
	{
		class MultipleInterfaces : public jni::WeakProxy<Runnable, Iterator>
		{
		public:
			MultipleInterfaces()
			: m_Count(10)
			{

			}

			virtual ~MultipleInterfaces()
			{
				printf("destroyed[%p]\n", this);
			}


			virtual void Run()
			{
				printf("Run[%p]!\n", this);
			}

			virtual void Remove()
			{
				jni::ThrowNew(UnsupportedOperationException::__CLASS, "This iterator does not support remove.");
			}

			virtual ::jboolean HasNext()
			{
				printf("HasNext[%p]!\n", this);
				bool result = (--m_Count != 0);
				printf("m_Count[%d][%d]\n", m_Count, result);
				return result;
			}

			virtual ::java::lang::Object Next()
			{
				return ::java::lang::String("this is a string");
			}

		private:
			unsigned m_Count;
		};

		{
			jni::LocalFrame frame;
			MultipleInterfaces* testProxy = new MultipleInterfaces();

			Runnable runnable = *testProxy;
			runnable.Run();

			Iterator iterator = *testProxy;
			while (iterator.HasNext())
			{
				String javaString = jni::Cast<String>(iterator.Next());
				printf("%s\n", static_cast<const char*>(javaString));
			}
		}
		for (int i = 0; i < 32; ++i) // Do a couple of loops to massage the GC
		{
			jni::LocalFrame frame;
			jni::Array<int> array(1024*1024);
			System::Gc();
		}

		printf("%s", "end of multi interface test\n");
	}

	// -------------------------------------------------------------
	// Proxy Object Test
	// -------------------------------------------------------------
	{
		jni::LocalFrame frame;
		struct PretendRunnable : jni::Proxy<Runnable>
		{
			virtual void Run() {printf("%s\n", "hello world!!!!"); }
		};

		PretendRunnable pretendRunnable;
		Runnable runnable = pretendRunnable;

		printf("equals: %d\n", runnable.Equals(runnable));
		printf("hashcode: %d\n", runnable.HashCode());
		printf("toString: %s\n", static_cast<const char*>(runnable.ToString()));
	}

	printf("%s\n", "EOP");

	// print resolution of clock()
	jni::DetachCurrentThread();

	vm->DestroyJavaVM();
	return 0;
}
