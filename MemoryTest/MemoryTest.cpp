#include <windows.h>
#include <iostream>
#include <queue>
#include <process.h>
#include "Monitor.h"
#include "winsock2.h"

using namespace std;

DWORD WINAPI threadConsumer(void*);
DWORD WINAPI threadProducer(void*);

#define GETMYRAND() (int)(((double)rand()/(double)RAND_MAX)*100)

queue<int> food;
struct Int {};
Int i;
Monitor<Int> easier(&i);
Monitor<queue<int>> food_ptr(&food);
bool producerfinished;

int main (){
	//SOCKADDR s;

	Monitor<queue<int>>* mydata = &food_ptr;
    producerfinished=false;

    HANDLE handles[3];

    handles[0] = CreateThread(0,0,&threadConsumer, 0, 0,0);
    handles[1] = CreateThread(0,0,&threadProducer, 0, 0,0);
    handles[2] = CreateThread(0,0,&threadConsumer, 0, 0,0);

	CloseHandle( handles[1] );
    WaitForMultipleObjects(3, handles, true, INFINITE); //"Join" threads
    
}

DWORD WINAPI threadConsumer(void* lp){
	
	food_ptr.Lock();
    while(!producerfinished){

        Sleep(GETMYRAND());
		
		food_ptr.Wait();

        while(food_ptr->size()>0){
            char tmps[32];
            sprintf(tmps, "\t%d\t(%d)\n",food_ptr->front(), GetCurrentThreadId());
            cout<<tmps;
            food_ptr->pop();
        }

    }
	food_ptr.Unlock();
    return 0;
}

DWORD WINAPI threadProducer(void* lp){
    for(int i =0 ; i < 100; i++){
        char tmps[32];
        sprintf(tmps, "%d\t\t%d\n",i,GetCurrentThreadId());
        cout<<tmps;
    
        Sleep(GETMYRAND());

		food_ptr.Lock();
		food_ptr->push(i);
		if (food_ptr->size() > 0)
			food_ptr.Pulse();
		food_ptr.Unlock();
    }
    producerfinished=true;

	food_ptr.Pulse(2);
    return 0;
}