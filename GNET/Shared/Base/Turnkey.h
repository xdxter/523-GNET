#pragma once

template<class T = bool>

///A class which handles the mutex and semaphores for the threads.
class Turnkey {
	HANDLE control_semaphore;
	T result;

public:
	///Function which creates a semaphore
	Turnkey();

	///Waits for the control semaphore
	void Wait();
	
	///Releases the semaphore
	void Pulse();

	///Sets the result
	void SetResult(T result);

	///Returns the result
	T GetResult();
};

///Function which creates a semaphore
template<class T> Turnkey<T>::Turnkey() {
	control_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
}

///Waits for the control semaphore
template<class T> void Turnkey<T>::Wait() {
	WaitForSingleObject(control_semaphore, INFINITE);
}

///Releases the semaphore
template<class T> void Turnkey<T>::Pulse() {
	ReleaseSemaphore(control_semaphore,1,NULL);
}

///Sets the result
template<class T> void Turnkey<T>::SetResult(T result) {
	this->result = result;
}

///Returns the result
template<class T> T Turnkey<T>::GetResult() {
	return result;
}