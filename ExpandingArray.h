#pragma once
#include <pthread.h>

namespace jni {
	template <class T>
	class ExpandingArray {
	public:
		ExpandingArray()
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&lock, &attr);
			pthread_mutexattr_destroy(&attr);
			elements = new T[maxSize];
		}

		~ExpandingArray()
		{
			pthread_mutex_destroy(&lock);
			delete[] elements;
		}

		void Add(T target)
		{
			pthread_mutex_lock(&lock);
			if( count == maxSize)
			{
				ExpandArray();
			}
			elements[count] = target;
			count++;
			pthread_mutex_unlock(&lock);
		}

		void Remove(T target)
		{
			pthread_mutex_lock(&lock);
			for(int i = count - 1; i >= 0; i--)
			{
				if (elements[i] == target)
				{
					if (i != count - 1)
						elements[i] = elements[count-1];
					count--;
					break;
				}
			}
			pthread_mutex_unlock(&lock);
		}

		void Clear()
		{
			pthread_mutex_lock(&lock);
			// Existing data should be overwritten or deleted if the ExpandingArray is deleted
			count = 0;
			pthread_mutex_unlock(&lock);
		}

		int GetCount()
		{
			pthread_mutex_lock(&lock);
			int currentCount = count;
			pthread_mutex_unlock(&lock);
			return currentCount;
		}

		T operator[] (int index)
		{
			pthread_mutex_lock(&lock);
			T targetElement = elements[index];
			pthread_mutex_unlock(&lock);
			return targetElement;
		}

	private:
		T* elements;
		int count = 0;
		int maxSize = 32;
		pthread_mutex_t lock;

		void ExpandArray()
		{
			auto biggerArray = new T[maxSize*2];
			for (int i = 0; i < maxSize; i++)
			{
				biggerArray[i] = elements[i];
			}

			delete[] elements;
			elements = biggerArray;
			maxSize *= 2;
		}
	};
}
