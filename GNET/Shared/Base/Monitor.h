#include <windows.h>

template<class T>
class Monitor {
private:
	HANDLE mutex;
	HANDLE control_semaphore;
	T ptr;

public:
	Monitor();
	~Monitor();
	
	void Lock();
	void Unlock();
	void Pulse(int i = 1);
	void Wait();
	
	T* operator->() { return &ptr; }
};

template <class T> Monitor<T>::Monitor() {
	mutex = CreateMutex(NULL, false, NULL);
	control_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
}

template<class T> Monitor<T>::~Monitor() {
	CloseHandle(mutex);
	CloseHandle(control_semaphore);
}

template<class T> void Monitor<T>::Lock() {
	WaitForSingleObject(mutex, INFINITE);
}
template<class T> void Monitor<T>::Unlock() {
	ReleaseMutex(mutex);
}
template<class T> void Monitor<T>::Pulse(int i) {
	ReleaseSemaphore(control_semaphore,i,NULL);
}
template<class T> void Monitor<T>::Wait() {
	ReleaseMutex(mutex);
	WaitForSingleObject(control_semaphore, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
}