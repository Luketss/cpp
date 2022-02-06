#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <mutex>
#include <process.h>

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

void StartPCP()
{
	using namespace std::literals::chrono_literals;

	std::cout << "Thread id= " << std::this_thread::get_id() << std::endl;

	while (!s_Finished)
	{
		string t = GeneratePCPMessage();
		m.lock();
		head = insertAtEnd(head, t);
		printList(head);
		m.unlock();
		//cout << getCount(head) << endl;
		std::this_thread::sleep_for(2s);
	}
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

	status = CreateProcess(
		L"..\\x64\\Debug\\supervisorio.exe", // Caminho do arquivo executável
		NULL,                       // Apontador p/ parâmetros de linha de comando
		NULL,                       // Apontador p/ descritor de segurança
		NULL,                       // Idem, threads do processo
		FALSE,	                     // Herança de handles
		NORMAL_PRIORITY_CLASS,	     // Flags de criação
		NULL,	                     // Herança do amniente de execução
		L"..\\x64\\Debug",              // Diretório do arquivo executável
		&si,			             // lpStartUpInfo
		&ProcessSupervisorio);	             // lpProcessInformation
	if (!status) printf("Erro na criacao do supervisorio! Codigo = %d\n", GetLastError());

	//SetConsoleTitle("Programa 2.1 - Criando Threads");
	//_getch();
	
	/*Node* ptrDelete = searchNode(head, 40);*/
	//bool delSuccess = deleteNode(head, ptrDelete);
	std::thread worker(StartSupervisorio);
	std::thread worker2(StartPCP);
	
	std::cin.get();
	s_Finished = true;

	worker.join();
	std::cout << "Finished" << std::endl;

	std::cin.get();
}




