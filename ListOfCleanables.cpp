#include "ListOfCleanables.h"

namespace jni {
	ListOfCleanables::ListOfCleanables()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&lock, &attr);
		pthread_mutexattr_destroy(&attr);
		head = NULL;
	}

	ListOfCleanables::~ListOfCleanables()
	{
		pthread_mutex_destroy(&lock);
	}

	void ListOfCleanables::Add(ICleanable* obj)
	{
		pthread_mutex_lock(&lock);
		head = new LinkedCleanable(obj, head);
		pthread_mutex_unlock(&lock);
	}

	void ListOfCleanables::Remove(ICleanable* obj)
	{
		pthread_mutex_lock(&lock);
		LinkedCleanable* current = head;
		LinkedCleanable* previous = NULL;
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

	void ListOfCleanables::CleanupAll()
	{
		pthread_mutex_lock(&lock);
		LinkedCleanable* current = head;
		head = NULL; // Destructor will call StopTracking, this will prevent it from looping through the whole list
		while (current != NULL)
		{
			LinkedCleanable* previous = current;
			current = current->next;
			previous->obj->Cleanup();
			delete previous;
		}
		pthread_mutex_unlock(&lock);
	}
}