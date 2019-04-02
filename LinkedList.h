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
			head = new Link(obj, head);
			pthread_mutex_unlock(&lock);
		}

		void Remove(T* obj)
		{
			pthread_mutex_lock(&lock);
			Link* current = head;
			Link* previous = NULL;
			while (current != NULL && current->obj != obj)
			{
				previous = current;
				current = current->next;
			}

			if (current != NULL)
			{
				if (previous == NULL)
					head = current->next;
				else
					previous->next = current->next;
				delete current;
			}
			pthread_mutex_unlock(&lock);
		}

		virtual void CleanupAll()
		{
			pthread_mutex_lock(&lock);
			Link* current = head;
			head = NULL; // Destructor will call StopTracking, this will prevent it from looping through the whole list
			while (current != NULL)
			{
				Link* previous = current;
				current = current->next;
				previous->obj->Cleanup();
				delete previous;
			}
			pthread_mutex_unlock(&lock);
		}

	private:
		class Link
		{
		public:
			Link(T* target, Link* link) : obj(target), next(link) {}

			T* obj;
			Link* next;
		};

		Link* head;
		pthread_mutex_t lock;
	};
}