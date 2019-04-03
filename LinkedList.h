#pragma once

#include <pthread.h>

namespace jni 
{
	template<class T>
	class LinkedList 
	{
	public:
		LinkedList()
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);
			pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
			pthread_mutex_init(&lock, &attr);
			pthread_mutexattr_destroy(&attr);
			head = NULL;
		}

		~LinkedList()
		{
			pthread_mutex_destroy(&lock);
		}

		void Add(T* obj)
		{
			pthread_mutex_lock(&lock);
			obj->nextCleanListLink = head;
			head = obj;
			pthread_mutex_unlock(&lock);
		}

		void Remove(T* obj)
		{
			pthread_mutex_lock(&lock);
			T* current = head;
			T* previous = NULL;
			while (current != NULL && current != obj)
			{
				previous = current;
				current = current->nextCleanListLink;
			}

			if (current != NULL)
			{
				if (previous == NULL)
					head = current->nextCleanListLink;
				else
					previous->nextCleanListLink = current->nextCleanListLink;
				current->nextCleanListLink = NULL;
			}
			pthread_mutex_unlock(&lock);
		}

		virtual void CleanupAll()
		{
			pthread_mutex_lock(&lock);
			T* current = head;
			head = NULL; // Destructor will call StopTracking, this will prevent it from looping through the whole list
			while (current != NULL)
			{
				T* previous = current;
				current = current->nextCleanListLink;
				previous->Cleanup();
			}
			pthread_mutex_unlock(&lock);
		}

		T* head;
		pthread_mutex_t lock;
	};
}