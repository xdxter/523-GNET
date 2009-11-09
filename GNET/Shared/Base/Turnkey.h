#pragma once

template<class T = bool>
class Turnkey {
	HANDLE control_semaphore;
	T result;

public:
	Turnkey();

	void Wait();
	void Pulse();
	void SetResult(T result);
	T GetResult();
};

template<class T> Turnkey<T>::Turnkey() {
	control_semaphore = CreateSemaphore(NULL, 0, 1, NULL);
}

template<class T> void Turnkey<T>::Wait() {
	WaitForSingleObject(control_semaphore, INFINITE);
}

template<class T> void Turnkey<T>::Pulse() {
	ReleaseSemaphore(control_semaphore,1,NULL);
}

template<class T> void Turnkey<T>::SetResult(T result) {
	this->result = result;
}
template<class T> T Turnkey<T>::GetResult() {
	return result;
}