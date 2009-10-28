template <class T>
class Turnkey {
	HANDLE control_semaphore;
	T result;

	Turnkey() {
		control_semaphore = CreateSemaphore(NULL, 0, 1000, NULL);
	}

	void Wait() {
		WaitForSingleObject(control_semaphore, INFINITE);
	}

	void Pulse() {
		result = t;
		ReleaseSemaphore(control_semaphore,i,NULL);
	}

	void SetResult(T t) {
		result = t;
	}
	T GetResult() {
		return result;
	}
}