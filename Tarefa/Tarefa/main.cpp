#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <mutex>
#include <process.h>
#include <pthread.h>

#include "supervisorio.h"
#include "ListFunctions.h"
#include "pcp.h"


using namespace std;
static bool s_Finished = false;
struct Node* head = NULL;
std::mutex m;

void StartSupervisorio()
{
	using namespace std::literals::chrono_literals;

	std::cout << "Thread id= " << std::this_thread::get_id() << std::endl;

	while (!s_Finished) 
	{
		string t = GenerateSupervisorioMessage();
		m.lock();
		head = insertAtEnd(head, t);
		printList(head);
		m.unlock();
		//cout << getCount(head) << endl;
		std::this_thread::sleep_for(2s);
	}
}

void* StartPCP(void* ptr)
{
	string t = GeneratePCPMessage();
	std:: cout << t << std::endl;
	return 0;

}


int main()
{
	BOOL status;
	STARTUPINFO si;				    // StartUpInformation para novo processo
	PROCESS_INFORMATION ProcessSupervisorio;	// Informações sobre novo processo criado

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes

	HANDLE hEvent;			// Handle para Evento
	HANDLE hEscEvent;		// Handle para Evento Aborta

	
	pthread_t thread1, thread2;
	std::string message1 = "t1";
	std::string message2 = "Thread 2";
	int  iret1, iret2;

	/* Create independent threads each of which will execute function */

	iret1 = pthread_create(&thread1, NULL, StartPCP, (void*)&message1);
	iret2 = pthread_create(&thread2, NULL, StartPCP, (void*)&message2);

	/* Wait till threads are complete before main continues. Unless we  */
	/* wait we run the risk of executing an exit which will terminate   */
	/* the process and all threads before the threads have completed.   */

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	printf("Thread 1 returns: %d\n", iret1);
	printf("Thread 2 returns: %d\n", iret2);
	exit(0);
	std::cout << "Finished" << std::endl;

	std::cin.get();
}




