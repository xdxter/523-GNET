#include <windows.h>

template<class T>
class Monitor {
private:
	HANDLE mutex;
	HANDLE control_semaphore;
	T ptr;

	void InitSemaphore(int semaphore_capacity = 1); 

public:
	Monitor();
	Monitor(int semaphore_capacity);
	~Monitor();
	
	void Lock();
	void Unlock();
	void Pulse(int i = 1);
	void Wait();
	
	T* operator->() { return &ptr; }
};

template <class T> Monitor<T>::Monitor() {
	mutex = CreateMutex(NULL, false, NULL);
}
template <class T> Monitor<T>::Monitor(int semaphore_capacity) {
	mutex = CreateMutex(NULL, false, NULL);
	InitSemaphore(semaphore_capacity);
}
template <class T> void Monitor<T>::InitSemaphore(int semaphore_capacity) {
	control_semaphore = CreateSemaphore(NULL, 0, semaphore_capacity, NULL);
}

template<class T> Monitor<T>::~Monitor() {
	CloseHandle(mutex);
	if (control_semaphore)
		CloseHandle(control_semaphore);
}

template<class T> void Monitor<T>::Lock() {
	WaitForSingleObject(mutex, INFINITE);
}
template<class T> void Monitor<T>::Unlock() {
	ReleaseMutex(mutex);
}
template<class T> void Monitor<T>::Pulse(int i) {
	if (!control_semaphore) InitSemaphore();
	ReleaseSemaphore(control_semaphore,i,NULL);
}
template<class T> void Monitor<T>::Wait() {
	if (!control_semaphore) InitSemaphore();
	ReleaseMutex(mutex);	
	WaitForSingleObject(control_semaphore, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
}