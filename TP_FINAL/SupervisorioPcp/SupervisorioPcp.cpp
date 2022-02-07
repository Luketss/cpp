#define WIN32_LEAN_AND_MEAN 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <process.h>
#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <list>
#include <iomanip> 

using std::cout; using std::endl;
using std::string; using std::stringstream;

// Casting para terceiro e sexto parâmetros da função _beginthreadex
typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);
typedef unsigned* CAST_LPDWORD;

#define SP			0x20
#define	ESC			0x1B
#define s			0x73
#define p			0x70
#define d			0x64
#define a			0x61
#define o			0x6F
#define c			0x63


void WINAPI LeSupervisorio();
void WINAPI CapturaSupervisorio();
void WINAPI LePCP();
void WINAPI CapturaPCP();
string GeraMsg(int NSEQ, int tipo);
string gen_random(const int len, int multiplicador);

HANDLE hSemLista;						// Handle para semaforo para acesso exclusivo à lista

HANDLE hEventLeSupervisorio;					// Handle para evento do processo EntradaTeclado -> controla tarefa LeSupervisorio
HANDLE hMutexEstadoLeSupervisorio;				// Handle para mutex para testar variavel de estado de LeSupervisorio
HANDLE hSemLeSupervisorio;						// Handle para semáforo para bloquear/desbloquear tarefa LeSupervisorio

HANDLE hEventCapturaSupervisorio;				// Handle para evento do processo EntradaTeclado -> controla tarefa CapturaSupervisorio
HANDLE hMutexEstadoCapturaSupervisorio;			// Handle para mutex para testar variavel de estado de CapturaSupervisorio
HANDLE hSemCapturaSupervisorio;					// Handle para semáforo para bloquear/desbloquear tarefa CapturaSupervisorio

HANDLE hEventLePCP;					// Handle para evento do processo EntradaTeclado -> controla tarefa LePCP
HANDLE hMutexEstadoLePCP;				// Handle para mutex para testar variavel de estado de LePCP
HANDLE hSemLePCP;						// Handle para semáforo para bloquear/desbloquear tarefa LePCP

HANDLE hEventCapturaPCP;				// Handle para evento do processo EntradaTeclado -> controla tarefa CapturaPCP
HANDLE hMutexEstadoCapturaPCP;			// Handle para mutex para testar variavel de estado de CapturaPCP
HANDLE hSemCapturaPCP;					// Handle para semáforo para bloquear/desbloquear tarefa CapturaPCP

HANDLE hEscEvent;						// Handle para Evento Aborta


bool EstadoLeSupervisorio = 1;					// Variavel que representa o estado de funcionamento da tarefa de leitura do Supervisorio. Desbloqueada=1,Bloqueada=0.
bool EstadoEventLeSupervisorio = 0;				// Variavel que representa o estado *anterior* do evento. Sinalizado=1,Não Sinalizado=0
bool LeSupervisorioFoiBloqueada = 0;				// Variavel que indica se a tarefa foi bloqueada pela EntradaTeclado. Foi Bloqueada=1,Nao Foi=0.

bool EstadoCapturaSupervisorio = 1;				// Variavel que representa o estado de funcionamento da tarefa de captura de dados da lista. Desbloqueada=1,Bloqueada=0.
bool EstadoEventCapturaSupervisorio = 0;		// Variavel que representa o estado *anterior* do evento. Sinalizado=1,Não Sinalizado=0
bool CapturaSupervisorioFoiBloqueada = 0;		// Variavel que indica se a tarefa foi bloqueada pela EntradaTeclado. Foi Bloqueada=1,Nao Foi=0.

bool EstadoLePCP = 1;					// Variavel que representa o estado de funcionamento da tarefa de leitura do PCP. Desbloqueada=1,Bloqueada=0.
bool EstadoEventLePCP = 0;				// Variavel que representa o estado *anterior* do evento. Sinalizado=1,Não Sinalizado=0
bool LePCPFoiBloqueada = 0;			// Variavel que indica se a tarefa foi bloqueada pela EntradaTeclado. Foi Bloqueada=1,Nao Foi=0.

bool EstadoCapturaPCP = 1;				// Variavel que representa o estado de funcionamento da tarefa de captura alarmes da lista. Desbloqueada=1,Bloqueada=0.
bool EstadoEventCapturaPCP = 0;		// Variavel que representa o estado *anterior* do evento. Sinalizado=1,Não Sinalizado=0
bool CapturaPCPFoiBloqueada = 0;		// Variavel que indica se a tarefa foi bloqueada pela EntradaTeclado. Foi Bloqueada=1,Nao Foi=0.


bool ListaCheiaSupervisorio = 0;					// Variavel que indica se a lista esta cheia para tarefa CapturaSupervisorio. Cheia=1, Não Cheia=0 
bool ListaVaziaSupervisorio = 1;					// Variavel que indica se a lista esta vazia para tarefa LeSupervisorio. Vazia=1, Não Vazia=0		

bool ListaCheiaPCP = 0;					// Variavel que indica se a lista esta cheia para tarefa CapturaPCP. Cheia=1, Não Cheia=0 
bool ListaVaziaPCP = 1;					// Variavel que indica se a lista esta vazia para tarefa LePCP. Vazia=1, Não Vazia=0		

int NSEQ_Supervisorio = 1;								// Variável que representa o numero sequencial das mensagens do Supervisorio
int NSEQ_PCP = 1;								// Variável que representa o numero sequencial das mensagens do PCP

std::list<string> listaMemoria;

int CapacidadeMaxLista = 100;

int main() 
{
	DWORD dwIdThread;
	DWORD dwExitCode = 0;
	DWORD dwRet;

	HANDLE hLeSupervisorio = NULL;
	HANDLE hCapturaSupervisorio = NULL;
	HANDLE hLePCP = NULL;
	HANDLE hCapturaPCP = NULL;

	hEscEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EscEvento");
	if (hEscEvent == NULL)
	{
		cout << "(Supervisorio_PCP): Erro na criacao da handle pro Esc Evento." << endl;
	}

	hEventLeSupervisorio = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoLeSupervisorio");
	if (hEventCapturaSupervisorio == NULL)
	{
		cout << "(Supervisorio_PCP): Erro na criacao da handle pro EventoSupervisorio." << endl;
	}

	hEventCapturaSupervisorio = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoCapturaSupervisorio");
	if (hEventCapturaSupervisorio == NULL)
	{
		cout << "(Supervisorio_PCP): Erro na criacao da handle pro EventCapturaSupervisorio." << endl;
	}

	hEventLePCP = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoLePCP");
	if (hEventLePCP == NULL)
	{
		cout << "(PCP_PCP): Erro na criacao da handle pro EventoPCP." << endl;
	}

	hEventCapturaPCP = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"EventoCapturaPCP");
	if (hEventCapturaPCP == NULL)
	{
		cout << "(PCP_PCP): Erro na criacao da handle pro EventCapturaPCP." << endl;
	}

	// Controla acesso à lista circular
	hSemLista = CreateSemaphore(NULL, 1, 1, L"SemAcessoLista");
	if (hSemLista == NULL) {
		cout << "(Supervisorio_PCP): Falha ao criar o SemLista:" << GetLastError() << endl;
	}

	// Controla acesso à variavel de estado da tarefa LeSupervisorio
	hMutexEstadoLeSupervisorio = CreateMutex(NULL, FALSE, L"MutexEstadoLeSupervisorio");
	if (hMutexEstadoLeSupervisorio == NULL) {
		cout << "(Supervisorio_PCP): Falha ao criar o MutexEstadoLeSupervisorio:" << GetLastError() << endl;
	}

	// Bloqueia/desbloqueia tarefa LeSupervisorio
	hSemLeSupervisorio = CreateSemaphore(NULL, 0, 1, L"SemLeSupervisorio");
	if (hSemLeSupervisorio == NULL) {
		cout << "(Supervisorio_PCP): Falha ao criar o semaforo SemLeSupervisorio:" << GetLastError() << endl;
	}

	// Controla acesso à variavel de estado da tarefa CapturaSupervisorio
	hMutexEstadoCapturaSupervisorio = CreateMutex(NULL, FALSE, L"MutexEstadoCapturaSupervisorio");
	if (hMutexEstadoCapturaSupervisorio == NULL) {
		cout << "(Supervisorio_PCP): Falha ao criar o MutexEstadoCapturaSupervisorio:" << GetLastError() << endl;
	}

	// Bloqueia/desbloqueia tarefa LeSupervisorio
	hSemCapturaSupervisorio = CreateSemaphore(NULL, 0, 1, L"SemCapturaSupervisorio");
	if (hSemCapturaSupervisorio == NULL) {
		cout << "(Supervisorio_PCP): Falha ao criar o semaforo SemCapturaSupervisorio:" << GetLastError() << endl;
	}

	// Controla acesso à variavel de estado da tarefa LePCP
	hMutexEstadoLePCP = CreateMutex(NULL, FALSE, L"MutexEstadoLePCP");
	if (hMutexEstadoLePCP == NULL) {
		cout << "(PCP_PCP): Falha ao criar o MutexEstadoLePCP:" << GetLastError() << endl;
	}

	// Bloqueia/desbloqueia tarefa LePCP
	hSemLePCP = CreateSemaphore(NULL, 0, 1, L"SemLePCP");
	if (hSemLePCP == NULL) {
		cout << "(PCP_PCP): Falha ao criar o semaforo SemLePCP:" << GetLastError() << endl;
	}

	// Controla acesso à variavel de estado da tarefa CapturaPCP
	hMutexEstadoCapturaPCP = CreateMutex(NULL, FALSE, L"MutexEstadoCapturaPCP");
	if (hMutexEstadoCapturaPCP == NULL) {
		cout << "(PCP_PCP): Falha ao criar o MutexEstadoCapturaPCP:" << GetLastError() << endl;
	}

	// Bloqueia/desbloqueia tarefa LePCP
	hSemCapturaPCP = CreateSemaphore(NULL, 0, 1, L"SemCapturaPCP");
	if (hSemCapturaPCP == NULL) {
		cout << "(PCP_PCP): Falha ao criar o semaforo SemCapturaPCP:" << GetLastError() << endl;
	}

	hLeSupervisorio = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)LeSupervisorio,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThread);

	if (hLeSupervisorio)
	{
		printf("(Supervisorio_PCP): Thread de leitura do Supervisorio criada com sucesso! Id=%0x\n", dwIdThread);
	}
	else
	{
		printf("(Supervisorio_PCP): Erro na criacao da thread de leitura do Supervisorio! Erro = %d\n", errno);
		exit(0);
	}
	/* --------------- FIM Thread LeSupervisorio --------------- */

	/* --------------- Thread CapturaSupervisorio --------------- */
	hCapturaSupervisorio = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)CapturaSupervisorio,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThread);

	if (hCapturaSupervisorio)
	{
		printf("(Supervisorio_PCP): Thread CapturaSupervisorio criada com sucesso! Id=%0x\n", dwIdThread);
	}
	else
	{
		printf("(Supervisorio_PCP): Erro na criacao da thread CapturaSupervisorio! Erro = %d\n", errno);
		exit(0);
	}
	/* --------------- FIM Thread CapturaSupervisorio --------------- */

	/* --------------- Thread LePCP --------------- */
	hLePCP = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)LePCP,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThread);

	if (hLePCP)
	{
		printf("(Supervisorio_PCP): Thread de leitura do PCP criada com sucesso! Id=%0x\n", dwIdThread);
	}
	else
	{
		printf("(Supervisorio_PCP): Erro na criacao da thread de leitura do PCP! Erro = %d\n", errno);
		exit(0);
	}
	/* --------------- FIM Thread LePCP --------------- */

	/* --------------- Thread CapturaPCP --------------- */
	hCapturaPCP = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)CapturaPCP,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThread);

	if (hCapturaPCP)
	{
		printf("(Supervisorio_PCP): Thread CapturaPCP criada com sucesso! Id=%0x\n", dwIdThread);
	}
	else
	{
		printf("(Supervisorio_PCP): Erro na criacao da thread CapturaPCP! Erro = %d\n", errno);
		exit(0);
	}
	/* --------------- FIM Thread CapturaPCP --------------- */

	while (true)
	{
		// +-------------------- Testa EscEvent --------------------+ //
		//cout << "(Supervisorio_PCP): Testa EscEvent" << endl;
		dwRet = WaitForSingleObject(hEscEvent, 0);
		if (dwRet == WAIT_TIMEOUT)
		{
			//cout << "(Supervisorio_PCP): EscEvent time out." << endl;
		}
		// Se evento estiver sinalizado, termina processo
		else if (dwRet == WAIT_OBJECT_0)
		{
			cout << "(Supervisorio_PCP): EscEvent sinalizado." << endl;
			CloseHandle(hLeSupervisorio);
			CloseHandle(hCapturaSupervisorio);
			cout << "(Supervisorio_PCP): Process Supervisorio_PCP sendo finalizado." << endl;
			ExitProcess(0);
		}
		else
		{
			cout << "(Supervisorio_PCP): Erro no retorno do EscEvent:" << GetLastError() << endl;
		}
		// +---------------------- Fim EscEvent ----------------------+ //

		// +-------------------- Testa EventLeSupervisorio --------------------+ //
		dwRet = WaitForSingleObject(hEventLeSupervisorio, 0);
		if (dwRet == WAIT_TIMEOUT) // Equivalente ao estado 0
		{
			// Testa se houve mudança de estado
			if (EstadoEventLeSupervisorio == 1)
			{
				WaitForSingleObject(hMutexEstadoLeSupervisorio, INFINITE);
				// Bloqueia tarefa LeSupervisorio pelo semaforo
				if (EstadoLeSupervisorio == 1)
				{
					EstadoLeSupervisorio = 0;
					LeSupervisorioFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemLeSupervisorio.\n" << endl;
					dwRet = WaitForSingleObject(hSemLeSupervisorio, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemLeSupervisorio time out.[EstadoEventLeSupervisorio == 1]" << endl;
					}
					// Se evento estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemLeSupervisorio diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemLeSupervisorio." << endl;
						break;
					}
				}
				// Desbloqueia tarefa LeSupervisorio pelo semaforo
				else
				{
					EstadoLeSupervisorio = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (LeSupervisorioFoiBloqueada)
					{
						ReleaseSemaphore(hSemLeSupervisorio, 1, NULL);
						LeSupervisorioFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemLeSupervisorio, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemLeSupervisorio.\n" << endl;
				}
				ReleaseMutex(hMutexEstadoLeSupervisorio);
				// Atualiza estado do evento
				EstadoEventLeSupervisorio = 0;
			}
			//cout << "(Supervisorio_PCP): EventLeSupervisorio time out." << endl;
		}
		else if (dwRet == WAIT_OBJECT_0) // Equivalente ao estado 1
		{
			// Testa se houve mudança de estado
			if (EstadoEventLeSupervisorio == 0)
			{
				WaitForSingleObject(hMutexEstadoLeSupervisorio, INFINITE);
				// Bloqueia tarefa LeSupervisorio pelo semaforo
				if (EstadoLeSupervisorio == 1)
				{
					EstadoLeSupervisorio = 0;
					LeSupervisorioFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemLeSupervisorio." << endl;
					dwRet = WaitForSingleObject(hSemLeSupervisorio, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemLeSupervisorio time out.[EstadoEventLeSupervisorio == 0]" << endl;
					}
					// Se semaforo estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemLeSupervisorio diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemLeSupervisorio." << endl;
						break;
					}
				}
				// Desbloqueia tarefa LeSupervisorio pelo semaforo
				else
				{
					EstadoLeSupervisorio = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (LeSupervisorioFoiBloqueada)
					{
						ReleaseSemaphore(hSemLeSupervisorio, 1, NULL);
						LeSupervisorioFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemLeSupervisorio, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemLeSupervisorio." << endl;
				}
				ReleaseMutex(hMutexEstadoLeSupervisorio);
				// Atualiza estado do evento
				EstadoEventLeSupervisorio = 1;
			}
			//cout << "(Supervisorio_PCP): EventLeSupervisorio sinalizado." << endl;
		}
		else
		{
			cout << "(Supervisorio_PCP): Erro no retorno do EventLeSupervisorio:" << GetLastError() << endl;
		}
		// +---------------------- Fim EventLeSupervisorio ----------------------+ //

		// +-------------------- Testa EventCapturaSupervisorio --------------------+ //
		dwRet = WaitForSingleObject(hEventCapturaSupervisorio, 0);
		if (dwRet == WAIT_TIMEOUT) // Equivalente ao estado 0
		{
			// Testa se houve mudança de estado
			if (EstadoEventCapturaSupervisorio == 1)
			{
				WaitForSingleObject(hMutexEstadoCapturaSupervisorio, INFINITE);
				// Bloqueia tarefa CapturaSupervisorio pelo semaforo
				if (EstadoCapturaSupervisorio == 1)
				{
					EstadoCapturaSupervisorio = 0;
					CapturaSupervisorioFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemCapturaSupervisorio.\n" << endl;
					dwRet = WaitForSingleObject(hSemCapturaSupervisorio, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemCapturaSupervisorio time out.[EstadoEventCapturaSupervisorio == 1]" << endl;
					}
					// Se evento estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemCapturaSupervisorio diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemCapturaSupervisorio." << endl;
						break;
					}
				}
				// Desbloqueia tarefa CapturaSupervisorio pelo semaforo
				else
				{
					EstadoCapturaSupervisorio = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (CapturaSupervisorioFoiBloqueada)
					{
						ReleaseSemaphore(hSemCapturaSupervisorio, 1, NULL);
						CapturaSupervisorioFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemCapturaSupervisorio, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemCapturaSupervisorio.\n" << endl;
				}
				ReleaseMutex(hMutexEstadoCapturaSupervisorio);
				// Atualiza estado do evento
				EstadoEventCapturaSupervisorio = 0;
			}
			//cout << "(Supervisorio_PCP): EventCapturaSupervisorio time out." << endl;
		}
		else if (dwRet == WAIT_OBJECT_0) // Equivalente ao estado 1
		{
			// Testa se houve mudança de estado
			if (EstadoEventCapturaSupervisorio == 0)
			{
				WaitForSingleObject(hMutexEstadoCapturaSupervisorio, INFINITE);
				// Bloqueia tarefa CapturaSupervisorio pelo semaforo
				if (EstadoCapturaSupervisorio == 1)
				{
					EstadoCapturaSupervisorio = 0;
					CapturaSupervisorioFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemCapturaSupervisorio." << endl;
					dwRet = WaitForSingleObject(hSemCapturaSupervisorio, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemCapturaSupervisorio time out.[EstadoEventCapturaSupervisorio == 0]" << endl;
					}
					// Se semaforo estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemCapturaSupervisorio diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemCapturaSupervisorio." << endl;
						break;
					}
				}
				// Desbloqueia tarefa CapturaSupervisorio pelo semaforo
				else
				{
					EstadoCapturaSupervisorio = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (CapturaSupervisorioFoiBloqueada)
					{
						ReleaseSemaphore(hSemCapturaSupervisorio, 1, NULL);
						CapturaSupervisorioFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemCapturaSupervisorio, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemCapturaSupervisorio." << endl;
				}
				ReleaseMutex(hMutexEstadoCapturaSupervisorio);
				// Atualiza estado do evento
				EstadoEventCapturaSupervisorio = 1;
			}
			//cout << "(Supervisorio_PCP): EventCapturaSupervisorio sinalizado." << endl;
		}
		else
		{
			cout << "(Supervisorio_PCP): Erro no retorno do EventCapturaSupervisorio:" << GetLastError() << endl;
		}
		// +---------------------- Fim EventCapturaSupervisorio ----------------------+ //

		// +-------------------- Testa EventLePCP --------------------+ //
		dwRet = WaitForSingleObject(hEventLePCP, 0);
		if (dwRet == WAIT_TIMEOUT) // Equivalente ao estado 0
		{
			// Testa se houve mudança de estado
			if (EstadoEventLePCP == 1)
			{
				WaitForSingleObject(hMutexEstadoLePCP, INFINITE);
				// Bloqueia tarefa LePCP pelo semaforo
				if (EstadoLePCP == 1)
				{
					EstadoLePCP = 0;
					LePCPFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemLePCP.\n" << endl;
					dwRet = WaitForSingleObject(hSemLePCP, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemLePCP time out.[EstadoEventLePCP == 1]" << endl;
					}
					// Se evento estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemLePCP diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemLePCP." << endl;
						break;
					}
				}
				// Desbloqueia tarefa LePCP pelo semaforo
				else
				{
					EstadoLePCP = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (LePCPFoiBloqueada)
					{
						ReleaseSemaphore(hSemLePCP, 1, NULL);
						LePCPFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemLePCP, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemLePCP.\n" << endl;
				}
				ReleaseMutex(hMutexEstadoLePCP);
				// Atualiza estado do evento
				EstadoEventLePCP = 0;
			}
			//cout << "(Supervisorio_PCP): EventLePCP time out." << endl;
		}
		else if (dwRet == WAIT_OBJECT_0) // Equivalente ao estado 1
		{
			// Testa se houve mudança de estado
			if (EstadoEventLePCP == 0)
			{
				WaitForSingleObject(hMutexEstadoLePCP, INFINITE);
				// Bloqueia tarefa LePCP pelo semaforo
				if (EstadoLePCP == 1)
				{
					EstadoLePCP = 0;
					LePCPFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemLePCP." << endl;
					dwRet = WaitForSingleObject(hSemLePCP, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemLePCP time out.[EstadoEventLePCP == 0]" << endl;
					}
					// Se semaforo estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemLePCP diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemLePCP." << endl;
						break;
					}
				}
				// Desbloqueia tarefa LePCP pelo semaforo
				else
				{
					EstadoLePCP = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (LePCPFoiBloqueada)
					{
						ReleaseSemaphore(hSemLePCP, 1, NULL);
						LePCPFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemLePCP, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemLePCP." << endl;
				}
				ReleaseMutex(hMutexEstadoLePCP);
				// Atualiza estado do evento
				EstadoEventLePCP = 1;
			}
			//cout << "(Supervisorio_PCP): EventLePCP sinalizado." << endl;
		}
		else
		{
			cout << "(Supervisorio_PCP): Erro no retorno do EventLePCP:" << GetLastError() << endl;
		}
		// +---------------------- Fim EventLePCP ----------------------+ //

		// +-------------------- Testa EventCapturaPCP --------------------+ //
		dwRet = WaitForSingleObject(hEventCapturaPCP, 0);
		if (dwRet == WAIT_TIMEOUT) // Equivalente ao estado 0
		{
			// Testa se houve mudança de estado
			if (EstadoEventCapturaPCP == 1)
			{
				WaitForSingleObject(hMutexEstadoCapturaPCP, INFINITE);
				// Bloqueia tarefa CapturaPCP pelo semaforo
				if (EstadoCapturaPCP == 1)
				{
					EstadoCapturaPCP = 0;
					CapturaPCPFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemCapturaPCP.\n" << endl;
					dwRet = WaitForSingleObject(hSemCapturaPCP, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemCapturaPCP time out.[EstadoEventCapturaPCP == 1]" << endl;
					}
					// Se evento estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemCapturaPCP diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemCapturaPCP." << endl;
						break;
					}
				}
				// Desbloqueia tarefa CapturaPCP pelo semaforo
				else
				{
					EstadoCapturaPCP = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (CapturaPCPFoiBloqueada)
					{
						ReleaseSemaphore(hSemCapturaPCP, 1, NULL);
						CapturaPCPFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemCapturaPCP, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemCapturaPCP.\n" << endl;
				}
				ReleaseMutex(hMutexEstadoCapturaPCP);
				// Atualiza estado do evento
				EstadoEventCapturaPCP = 0;
			}
			//cout << "(Supervisorio_PCP): EventCapturaPCP time out." << endl;
		}
		else if (dwRet == WAIT_OBJECT_0) // Equivalente ao estado 1
		{
			// Testa se houve mudança de estado
			if (EstadoEventCapturaPCP == 0)
			{
				WaitForSingleObject(hMutexEstadoCapturaPCP, INFINITE);
				// Bloqueia tarefa CapturaPCP pelo semaforo
				if (EstadoCapturaPCP == 1)
				{
					EstadoCapturaPCP = 0;
					CapturaPCPFoiBloqueada = 1;
					//cout << "(Supervisorio_PCP): Wait SemCapturaPCP." << endl;
					dwRet = WaitForSingleObject(hSemCapturaPCP, 0);
					if (dwRet == WAIT_TIMEOUT)
					{
						//cout << "(Supervisorio_PCP): SemCapturaPCP time out.[EstadoEventCapturaPCP == 0]" << endl;
					}
					// Se semaforo estiver sinalizado, continua
					else if (dwRet == WAIT_OBJECT_0)
					{
						//cout << "(Supervisorio_PCP): SemCapturaPCP diminuido de 1." << endl;
					}
					else
					{
						cout << "(Supervisorio_PCP): Erro no retorno do SemCapturaPCP." << endl;
						break;
					}
				}
				// Desbloqueia tarefa CapturaPCP pelo semaforo
				else
				{
					EstadoCapturaPCP = 1;
					// Se tiver sido bloqueada pelo teclado, libera 2 no contador
					if (CapturaPCPFoiBloqueada)
					{
						ReleaseSemaphore(hSemCapturaPCP, 1, NULL);
						CapturaPCPFoiBloqueada = 0;
					}
					// Se tiver sido bloqueada por ela mesma, libera 1 no contador
					else
					{
						ReleaseSemaphore(hSemCapturaPCP, 1, NULL);
					}
					//cout << "(Supervisorio_PCP): Release SemCapturaPCP." << endl;
				}
				ReleaseMutex(hMutexEstadoCapturaPCP);
				// Atualiza estado do evento
				EstadoEventCapturaPCP = 1;
			}
			//cout << "(Supervisorio_PCP): EventCapturaPCP sinalizado." << endl;
		}
		else
		{
			cout << "(Supervisorio_PCP): Erro no retorno do EventCapturaPCP:" << GetLastError() << endl;
		}
		// +---------------------- Fim EventCapturaPCP ----------------------+ //

	}// Fim WHILE

	// --------------------------------------------------------------------------
	// Aguarda término das threads e encerra programa
	// --------------------------------------------------------------------------

	//dwRet = WaitForSingleObject(hLeSupervisorio, INFINITE);

	//Fecha todos os handles de objetos do kernel
	//CloseHandle(hLeSupervisorio);
}

void WINAPI LeSupervisorio()
{
	//DWORD retorno;
	string msg;
	bool status;
	int tipo = 1;

	while (true)
	{
		// Testa se lista circular está cheia
		WaitForSingleObject(hSemLista, INFINITE);
		if (listaMemoria.size() >= CapacidadeMaxLista)
		{
			cout << "(LeSupervisorio): Lista circular cheia. Bloqueando thread LeSupervisorio." << endl;
			ListaCheiaSupervisorio = 1;
			WaitForSingleObject(hMutexEstadoLeSupervisorio, INFINITE);
			EstadoLeSupervisorio = 0;
			ReleaseMutex(hMutexEstadoLeSupervisorio);
			ReleaseSemaphore(hSemLista, 1, NULL);
			// Espera semaforo -> bloqueia thread
			WaitForSingleObject(hSemLeSupervisorio, INFINITE);
			cout << "(LeSupervisorio): Desbloqueando thread LeSupervisorio. " << endl;
			Sleep(1000);
		}
		else
		{
			ReleaseSemaphore(hSemLista, 1, NULL);
		}

		//Testa se foi bloqueada ou nao
		WaitForSingleObject(hMutexEstadoLeSupervisorio, INFINITE);
		if (EstadoLeSupervisorio == 0)
		{
			cout << "(LeSupervisorio): Bloqueando thread LeSupervisorio. " << endl;
			ReleaseMutex(hMutexEstadoLeSupervisorio);
			// Espera semaforo -> bloqueia thread
			WaitForSingleObject(hSemLeSupervisorio, INFINITE);
			cout << "(LeSupervisorio): Desbloqueando thread LeSupervisorio. " << endl;
		}
		else
		{
			ReleaseMutex(hMutexEstadoLeSupervisorio);
		}

		// Domina acesso à lista
		WaitForSingleObject(hSemLista, INFINITE);
		// +-------------- SEÇÃO CRÍTICA --------------+ //		
		msg.assign(GeraMsg(NSEQ_Supervisorio, tipo));
		//cout << msg << endl;
		if (listaMemoria.size() >= CapacidadeMaxLista)
		{
			listaMemoria.pop_front();
			listaMemoria.push_back(msg);
		}
		else
		{
			listaMemoria.push_back(msg);
		}

		if (NSEQ_Supervisorio >= 999999)
		{
			NSEQ_Supervisorio = 1;
		}
		else
		{
			NSEQ_Supervisorio++;
		}

		// Testa se tarefa de captura está esperando inserção na lista
		WaitForSingleObject(hMutexEstadoCapturaSupervisorio, INFINITE);
		if (!EstadoCapturaSupervisorio && ListaVaziaSupervisorio && !CapturaSupervisorioFoiBloqueada)
		{
			ListaVaziaSupervisorio = 0;
			EstadoCapturaSupervisorio = 1;
			ReleaseSemaphore(hSemCapturaSupervisorio, 1, NULL);
		}
		ReleaseMutex(hMutexEstadoCapturaSupervisorio);
		// +-------------- Fim SEÇÃO CRÍTICA --------------+ //

		status = ReleaseSemaphore(hSemLista, 1, NULL);
		if (status == NULL) {
			printf("(LeSupervisorio): Erro ao liberar SemLista: %d\n", GetLastError());
		}
		//cout << "(LeSupervisorio): Fim WHILE Thread LeSupervisorio." << endl;
		Sleep(1000);
	}

	CloseHandle(hSemLeSupervisorio);
	CloseHandle(hMutexEstadoLeSupervisorio);

	cout << "(LeSupervisorio): Thread LeSupervisorio encerrando execucao..." << endl;
	_endthreadex(0);
}// ------------------------------- FIM LeSupervisorio ------------------------------- //


void WINAPI CapturaSupervisorio()
{
	Sleep(1000);		// Espera primeira inserção
	string msg;
	DWORD dwRet;
	bool status;
	while (true)
	{
		// Testa se lista circular está vazia
		WaitForSingleObject(hSemLista, INFINITE);
		if (listaMemoria.size() == 0)
		{
			//cout << "(CapturaSupervisorio): Lista circular vazia. Bloqueando thread CapturaSupervisorio." << endl;
			ListaVaziaSupervisorio = 1;
			WaitForSingleObject(hMutexEstadoCapturaSupervisorio, INFINITE);
			EstadoCapturaSupervisorio = 0;
			ReleaseMutex(hMutexEstadoCapturaSupervisorio);
			ReleaseSemaphore(hSemLista, 1, NULL);
			// Espera semaforo -> bloqueia thread
			dwRet = WaitForSingleObject(hSemCapturaSupervisorio, INFINITE);
			if (dwRet == WAIT_TIMEOUT)
			{
				//cout << "(CapturaSupervisorio): SemCapturaSupervisorio time out." << endl;
			}
			// Se semaforo estiver sinalizado, continua
			else if (dwRet == WAIT_OBJECT_0)
			{
				//cout << "(CapturaSupervisorio): SemCapturaSupervisorio liberado." << endl;
			}
			else
			{
				cout << "(CapturaSupervisorio): Erro no retorno do SemCapturaSupervisorio." << endl;
				break;
			}
			cout << "(LeSupervisorio): Desbloqueando thread CapturaSupervisorio. " << endl;
			Sleep(1000);
		}
		else
		{
			ReleaseSemaphore(hSemLista, 1, NULL);
		}

		//Testa se foi bloqueada ou nao
		WaitForSingleObject(hMutexEstadoCapturaSupervisorio, INFINITE);
		if (EstadoCapturaSupervisorio == 0)
		{
			cout << "(CapturaSupervisorio): Bloqueando thread CapturaSupervisorio. " << endl;
			ReleaseMutex(hMutexEstadoCapturaSupervisorio);
			// Espera semaforo -> bloqueia thread
			dwRet = WaitForSingleObject(hSemCapturaSupervisorio, INFINITE);
			if (dwRet == WAIT_TIMEOUT)
			{
				//cout << "(CapturaSupervisorio): SemCapturaSupervisorio time out." << endl;
			}
			// Se semaforo estiver sinalizado, continua
			else if (dwRet == WAIT_OBJECT_0)
			{
				//cout << "(CapturaSupervisorio): SemCapturaSupervisorio liberado." << endl;
			}
			else
			{
				cout << "(CapturaSupervisorio): Erro no retorno do SemCapturaSupervisorio." << endl;
				break;
			}
			cout << "(LeSupervisorio): Desbloqueando thread CapturaSupervisorio. " << endl;
		}
		else
		{
			ReleaseMutex(hMutexEstadoCapturaSupervisorio);
		}


		WaitForSingleObject(hSemLista, INFINITE);
		// +-------------- SEÇÃO CRÍTICA --------------+ //
		if (listaMemoria.size() > 0)
		{
			std::list<string>::iterator it;
			for (it = listaMemoria.begin(); it != listaMemoria.end(); ++it)
			{
				msg.assign(it->data());
				cout << msg << endl;
				listaMemoria.erase(it);
				break;
				// Testa se a mensagem é um alarme. Se for, vai pra proxima iteração.
				//if (it->data()[7] == '2' || it->data()[7] == '9')
				//{
				//	continue;
				//}
				//// Testa se a mensagem é um dado do processo. Se for, pega e exibe na tela.
				//else if (it->data()[7] == '1')
				//{
				//	// Pega mensagem da lista e imprime no console
				//	msg.assign(it->data());
				//	//cout << "(CapturaSupervisorio): " << msg << endl;
				//	cout << msg << endl;

				//	//Apaga mensagem da lista
				//	listaMemoria.erase(it);
				//	break;
				//}
				//else
				//{
				//	cout << "(CapturaSupervisorio): Erro ao pegar msg na lista." << endl;
				//}
			}

			// Testa se tarefa de leitura do Supervisorio está esperando esvaziar lista
			WaitForSingleObject(hMutexEstadoLeSupervisorio, INFINITE);
			if (!EstadoLeSupervisorio && ListaCheiaSupervisorio && !LeSupervisorioFoiBloqueada)
			{
				ListaCheiaSupervisorio = 0;
				EstadoLeSupervisorio = 1;
				ReleaseSemaphore(hSemLeSupervisorio, 1, NULL);
			}
			ReleaseMutex(hMutexEstadoLeSupervisorio);
		}
		// +-------------- Fim SEÇÃO CRÍTICA --------------+ //

		status = ReleaseSemaphore(hSemLista, 1, NULL);
		if (status == NULL) {
			printf("(LeSupervisorio): Erro ao liberar SemLista: %d\n", GetLastError());
		}
		Sleep(1000);
	}// Fim WHILE

	CloseHandle(hSemCapturaSupervisorio);
	CloseHandle(hMutexEstadoCapturaSupervisorio);

	_endthreadex(0);
} // ------------------------------- Fim CapturaSupervisorio ------------------------------- //

void WINAPI LePCP()
{
	string msg;
	bool status;
	int tipo;

	//cout << "(LePCP): Entrou na thread LePCP." << endl;

	while (true)
	{
		// Testa se lista circular está cheia
		WaitForSingleObject(hSemLista, INFINITE);
		if (listaMemoria.size() >= CapacidadeMaxLista)
		{
			cout << "(LePCP): Lista circular cheia. Bloqueando thread LePCP." << endl;
			ListaCheiaPCP = 1;
			WaitForSingleObject(hMutexEstadoLePCP, INFINITE);
			EstadoLePCP = 0;
			ReleaseMutex(hMutexEstadoLePCP);
			ReleaseSemaphore(hSemLista, 1, NULL);
			// Espera semaforo -> bloqueia thread
			WaitForSingleObject(hSemLePCP, INFINITE);
			cout << "(LePCP): Desbloqueando thread LePCP. " << endl;
			Sleep(1000);
		}
		else
		{
			ReleaseSemaphore(hSemLista, 1, NULL);
		}

		//Testa se foi bloqueada ou nao
		WaitForSingleObject(hMutexEstadoLePCP, INFINITE);
		if (EstadoLePCP == 0)
		{
			cout << "(LePCP): Bloqueando thread LePCP. " << endl;
			ReleaseMutex(hMutexEstadoLePCP);
			// Espera semaforo -> bloqueia thread
			WaitForSingleObject(hSemLePCP, INFINITE);
			cout << "(LePCP): Desbloqueando thread LePCP. " << endl;
		}
		else
		{
			ReleaseMutex(hMutexEstadoLePCP);
		}

		// Domina acesso à lista
		WaitForSingleObject(hSemLista, INFINITE);
		// +-------------- SEÇÃO CRÍTICA --------------+ //		

		if ((rand() % 2) == 0)
		{
			tipo = 2;
		}
		else
		{
			tipo = 9;
		}
		msg.assign(GeraMsg(NSEQ_PCP, tipo));
		if (listaMemoria.size() >= CapacidadeMaxLista)
		{
			listaMemoria.pop_front();
			listaMemoria.push_back(msg);
		}
		else
		{
			listaMemoria.push_back(msg);
		}

		if (NSEQ_PCP >= 999999)
		{
			NSEQ_PCP = 1;
		}
		else
		{
			NSEQ_PCP++;
		}

		//cout << "(LePCP): " << listaMemoria.back() << endl;

		// Testa se tarefa de captura está esperando inserção na lista
		WaitForSingleObject(hMutexEstadoCapturaPCP, INFINITE);
		if (!EstadoCapturaPCP && ListaVaziaPCP && !CapturaPCPFoiBloqueada)
		{
			ListaVaziaPCP = 0;
			EstadoCapturaPCP = 1;
			ReleaseSemaphore(hSemCapturaPCP, 1, NULL);
		}
		ReleaseMutex(hMutexEstadoCapturaPCP);
		// +-------------- Fim SEÇÃO CRÍTICA --------------+ //

		status = ReleaseSemaphore(hSemLista, 1, NULL);
		if (status == NULL) {
			printf("(LePCP): Erro ao liberar SemLista: %d\n", GetLastError());
		}
		//cout << "(LePCP): Fim WHILE Thread LePCP." << endl;
		Sleep(1000);
	}

	CloseHandle(hSemLePCP);
	CloseHandle(hMutexEstadoLePCP);

	cout << "(LePCP): Thread LePCP encerrando execucao..." << endl;
	_endthreadex(0);
}// ------------------------------- FIM LePCP ------------------------------- //

void WINAPI CapturaPCP()
{
	Sleep(1000);		// Espera primeira inserção
	//cout << "(CapturaPCP): Criou thread CapturaPCP." << endl;
	string msg;
	DWORD dwRet;
	bool status;
	while (true)
	{
		// Testa se lista circular está vazia
		WaitForSingleObject(hSemLista, INFINITE);
		if (listaMemoria.size() == 0)
		{
			//cout << "(CapturaPCP): Lista circular vazia. Bloqueando thread CapturaPCP." << endl;
			ListaVaziaPCP = 1;
			WaitForSingleObject(hMutexEstadoCapturaPCP, INFINITE);
			EstadoCapturaPCP = 0;
			ReleaseMutex(hMutexEstadoCapturaPCP);
			ReleaseSemaphore(hSemLista, 1, NULL);
			// Espera semaforo -> bloqueia thread
			dwRet = WaitForSingleObject(hSemCapturaPCP, INFINITE);
			if (dwRet == WAIT_TIMEOUT)
			{
				//cout << "(CapturaPCP): SemCapturaPCP time out." << endl;
			}
			// Se semaforo estiver sinalizado, continua
			else if (dwRet == WAIT_OBJECT_0)
			{
				//cout << "(CapturaPCP): SemCapturaPCP liberado." << endl;
			}
			else
			{
				cout << "(CapturaPCP): Erro no retorno do SemCapturaPCP." << endl;
				break;
			}
			cout << "(LePCP): Desbloqueando thread CapturaPCP. " << endl;
			Sleep(1000);
		}
		else
		{
			ReleaseSemaphore(hSemLista, 1, NULL);
		}

		//Testa se foi bloqueada ou nao
		WaitForSingleObject(hMutexEstadoCapturaPCP, INFINITE);
		if (EstadoCapturaPCP == 0)
		{
			cout << "(CapturaPCP): Bloqueando thread CapturaPCP. " << endl;
			ReleaseMutex(hMutexEstadoCapturaPCP);
			// Espera semaforo -> bloqueia thread
			dwRet = WaitForSingleObject(hSemCapturaPCP, INFINITE);
			if (dwRet == WAIT_TIMEOUT)
			{
				//cout << "(CapturaPCP): SemCapturaPCP time out." << endl;
			}
			// Se semaforo estiver sinalizado, continua
			else if (dwRet == WAIT_OBJECT_0)
			{
				//cout << "(CapturaPCP): SemCapturaPCP liberado." << endl;
			}
			else
			{
				cout << "(CapturaPCP): Erro no retorno do SemCapturaPCP." << endl;
				break;
			}
			cout << "(LePCP): Desbloqueando thread CapturaPCP. " << endl;
		}
		else
		{
			ReleaseMutex(hMutexEstadoCapturaPCP);
		}


		WaitForSingleObject(hSemLista, INFINITE);
		// +-------------- SEÇÃO CRÍTICA --------------+ //
		if (listaMemoria.size() > 0)
		{
			//cout << "(CapturaPCP): Tamanho da lista > 0." << endl;
			std::list<string>::iterator it;
			for (it = listaMemoria.begin(); it != listaMemoria.end(); ++it)
			{
				msg.assign(it->data());
				cout << msg << endl;
				listaMemoria.erase(it);
				break;
				//cout << "(CapturaPCP): Entrou no for." << endl;
				// Testa se a mensagem é um dado do processo. Se for, vai pra proxima iteração.
				//if (it->data()[7] == '1')
				//{
				//	continue;
				//}
				//// Testa se a mensagem é um alarme. Se for, pega e exibe na tela.
				//else if (it->data()[7] == '2' || it->data()[7] == '9')
				//{

				//	// Pega mensagem da lista e imprime no console
				//	msg.assign(it->data());
				//	//cout << "(CapturaPCP): " << msg << endl;
				//	cout << msg << endl;

				//	//Apaga mensagem da lista
				//	listaMemoria.erase(it);
				//	break;
				//}
				//else
				//{
				//	cout << "(CapturaPCP): Erro ao pegar msg na lista." << endl;
				//}
			}

			// Testa se tarefa de leitura do PCP está esperando esvaziar lista
			WaitForSingleObject(hMutexEstadoLePCP, INFINITE);
			if (!EstadoLePCP && ListaCheiaPCP && !LePCPFoiBloqueada)
			{
				ListaCheiaPCP = 0;
				EstadoLePCP = 1;
				ReleaseSemaphore(hSemLePCP, 1, NULL);
			}
			ReleaseMutex(hMutexEstadoLePCP);
		}
		// +-------------- Fim SEÇÃO CRÍTICA --------------+ //

		status = ReleaseSemaphore(hSemLista, 1, NULL);
		if (status == NULL) {
			printf("(LePCP): Erro ao liberar SemLista: %d\n", GetLastError());
		}

		Sleep(1000);
	}// Fim WHILE

	CloseHandle(hSemCapturaPCP);
	CloseHandle(hMutexEstadoCapturaPCP);

	_endthreadex(0);
} // ------------------------------- Fim CapturaPCP ------------------------------- //

string GeraMsg(int NSEQ, int tipo)
{
	stringstream ssMsg;
	stringstream ssNSEQ;

	//Supervisorio
	if (tipo == 1)
	{
		stringstream ssValor;
		char cModo;

		// Campo = NSEQ
		// Completa tamanho do campo NSEQ
		ssNSEQ << std::setfill('0') << std::setw(6) << NSEQ;

		// Campo = VALOR
		// Gera numero aleatorio de 7 algarismos + 1 ponto = 8 caracteres
		for (int n = 0; n < 8; n++)
		{
			if (n == 5)
			{
				ssValor << '.';
			}
			else
			{
				ssValor << rand() % 10;
			}
		}

		// Campo = MODO
		// Define o modo
		if ((rand() % 2) == 0)
		{
			cModo = 'A';
		}
		else
		{
			cModo = 'M';
		}

		// Campo = TIMESTAMP
		// Pega o horario atual
		stringstream strTimestamp;
		SYSTEMTIME timestamp;
		GetLocalTime(&timestamp);
		strTimestamp << std::setfill('0') << std::setw(2) << timestamp.wHour << ':' << std::setfill('0') << std::setw(2) << timestamp.wMinute
			<< ':' << std::setfill('0') << std::setw(2) << timestamp.wSecond << '.' << std::setfill('0') << std::setw(3) << timestamp.wMilliseconds;

		// Monta mensagem de dados que será retornada para thread LeSupervisorio
		ssMsg << ssNSEQ.str() << '#' << tipo << '#' << gen_random(10, timestamp.wMilliseconds) << '#' << ssValor.str() << '#'
			<< gen_random(8, timestamp.wMilliseconds + timestamp.wSecond) << '#' << cModo << '#' << strTimestamp.str() << endl;

		return ssMsg.str();
	}// Fim tipo 1
	else if (tipo == 2 || tipo == 9)
	{
		stringstream ssIdAlarme;
		stringstream ssGrau;
		stringstream ssPrev;

		// Campo = NSEQ
		// Completa tamanho do campo NSEQ
		ssNSEQ << std::setfill('0') << std::setw(6) << NSEQ;

		// Campo = ID ALARME
		// Gera numero aleatorio de 4 algarismos
		//for (int n = 0; n < 4; n++)
		//{			
		//}
		ssIdAlarme << std::setfill('0') << std::setw(4) << rand() % 10000;

		// Campo = GRAU
		// Gera numero aleatorio de 2 algarismos
		//for (int n = 0; n < 2; n++)
		//{
		//}
		ssGrau << std::setfill('0') << std::setw(2) << rand() % 100;
		//cout << "ssGrau:" << ssGrau.str() << endl;

		// Campo = PREV
		// Define o modo
		ssPrev << std::setfill('0') << std::setw(5) << rand() % 14441;
		//cout << "ssPrev:" << ssPrev.str() << endl;

		// Pega o horario atual
		// Campo = TIMESTAMP
		stringstream strTimestamp;
		SYSTEMTIME timestamp;
		GetLocalTime(&timestamp);
		strTimestamp << std::setfill('0') << std::setw(2) << timestamp.wHour << ':' << std::setfill('0') << std::setw(2) << timestamp.wMinute
			<< ':' << std::setfill('0') << std::setw(2) << timestamp.wSecond;


			// Monta mensagem de alarme que será retornada para thread LePCP
			//ssMsg << ssNSEQ.str() << '|' << tipo << '|' << ssIdAlarme.str() << '|' << ssGrau.str() << '|' << ssPrev.str() << '|' << strTimestamp.str() << endl;
			ssMsg << 5;
		return ssMsg.str();
	}// Fim tipo 2 e 9

	// Não deve chegar aqui
	cout << "a" << endl;
	return 0;
}// ------------------------------- FIM GeraMsg ------------------------------- //

// ------------------------------- Função gen_random ------------------------------- //
// Função que gera string aleatoria com base no conjunto de caracteres determinado
// e de tamanho "len"
string gen_random(const int len, int multiplicador)
{
	string tmp_s;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ-"
		"abcdefghijklmnopqrstuvwxyz";

	// Torna o processo aleatorio usando o tempo atual * multiplicador (que é mseg ou seg+mseg)
	// como seed para o gerador.
	srand((unsigned)time(NULL) * multiplicador);

	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}// ------------------------------- FIM gen_random ------------------------------- //

string gen_random_num(const int len)
{
	string tmp_s;
	static const char alphanum[] =
		"0123456789";

	for (int i = 0; i < len; ++i)
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}// ------------------------------- FIM gen_random ------------------------------- //