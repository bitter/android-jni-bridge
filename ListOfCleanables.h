#pragma once

#include "ICleanable.h"
#include <pthread.h>


namespace jni {
	class ListOfCleanables {
public:
	ListOfCleanables();
	~ListOfCleanables();
	void Add(ICleanable* obj);
	void Remove(ICleanable* obj);
	void CleanupAll();

private:
	class LinkedCleanable
	{
	public:
		LinkedCleanable(ICleanable* target, LinkedCleanable* link) : obj(target), next(link) {}

		ICleanable* obj;
		LinkedCleanable* next;
	};

	LinkedCleanable* head;
	pthread_mutex_t lock;
	};
}