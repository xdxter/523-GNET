#include <windows.h>

template<class T>

///A class which handles the mutex and semaphores for the threads.
class Monitor {
private:
	HANDLE mutex;
	HANDLE control_semaphore;
	T ptr;

	///A function which Initializes a semaphore.
	void InitSemaphore(int semaphore_capacity = 1); 

public:
	///Creates a mutex.
	Monitor();

	///Creates a mutex according to the given semaphore capacity.
	Monitor(int semaphore_capacity);

	///Closes the Handle for mutex and semaphore
	~Monitor();
	
	///Waits for a semaphore
	void Lock();

	///Releases the mutex
	void Unlock();

	///Releases the semaphore
	void Pulse(int i = 1);

	///Releases the mutex first and waits for semaphore and a mutex.
	void Wait();
	
	T* operator->() { return &ptr; }
};

///Creates a Mutex
template <class T> Monitor<T>::Monitor() {
	mutex = CreateMutex(NULL, false, NULL);
}

///Creates a Mutex and Initializes it according to the semaphore capacity provided
template <class T> Monitor<T>::Monitor(int semaphore_capacity) {
	mutex = CreateMutex(NULL, false, NULL);
	InitSemaphore(semaphore_capacity);
}

///Creates a Semaphore
template <class T> void Monitor<T>::InitSemaphore(int semaphore_capacity) {
	control_semaphore = CreateSemaphore(NULL, 0, semaphore_capacity, NULL);
}

///Destructor - Removes the Handle for the Mutex and Semaphore
template<class T> Monitor<T>::~Monitor() {
	CloseHandle(mutex);
	if (control_semaphore)
		CloseHandle(control_semaphore);
}

///Waits for a Mutex
template<class T> void Monitor<T>::Lock() {
	WaitForSingleObject(mutex, INFINITE);
}

///Releases the Mutex
template<class T> void Monitor<T>::Unlock() {
	ReleaseMutex(mutex);
}

///Releases the Semaphore
template<class T> void Monitor<T>::Pulse(int i) {
	if (!control_semaphore) InitSemaphore();
	ReleaseSemaphore(control_semaphore,i,NULL);
}

///Releases the Mutex first and waits for Semaphore and a Mutex.
template<class T> void Monitor<T>::Wait() {
	if (!control_semaphore) InitSemaphore();
	ReleaseMutex(mutex);	
	WaitForSingleObject(control_semaphore, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
}