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

using std::cout; using std::endl;
using std::string;

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


HANDLE hEventLeSupervisorio = NULL;					// Handle para Evento da tarefa de leitura do SDCD
HANDLE hEventCapturaSupervisorio = NULL;			// Handle para Evento da tarefa de captura de dados da lista

HANDLE hEventLePCP = NULL;					// Handle para Evento da tarefa de leitura do PIMS
HANDLE hEventCapturaPCP = NULL;			// Handle para Evento da tarefa de captura de alarmes da lista

HANDLE hEventExibeAlarme = NULL;

HANDLE hEscEvent = NULL;					// Handle para Evento Aborta

int nTecla;								//Variável que armazena a tecla digitada para sair


// THREAD PRIMÁRIA
int main()
{
	DWORD dwIdThread;
	DWORD dwExitCode = 0;
	DWORD dwRet;
	bool ret;

	// --------------------------------------------------------------------------
	//  Cria objetos de sincronização
	// --------------------------------------------------------------------------

	// Controle fim da execução da aplicação
	hEscEvent = CreateEvent(NULL, TRUE, FALSE, L"EscEvento"); //Evento de reset manual
	if (hEscEvent == NULL) {
		printf("(EntradaTeclado): Erro na criação do EscEvent: %d\n", GetLastError());
	}

	// Controle bloqueio/desbloqueio da tarefa LeSDCD
	hEventLeSupervisorio = CreateEvent(NULL, TRUE, FALSE, L"EventoLeSupervisorio"); //Evento de reset manual
	if (hEventLeSupervisorio == NULL) {
		printf("(EntradaTeclado): Erro na criação do EventoLeSupervisorio: %d\n", GetLastError());
	}

	// Controle bloqueio/desbloqueio da tarefa CapturaSDCD
	hEventCapturaSupervisorio = CreateEvent(NULL, TRUE, FALSE, L"EventoCapturaSupervisorio"); //Evento de reset manual
	if (hEventCapturaSupervisorio == NULL) {
		printf("(EntradaTeclado): Erro na criação do EventoCapturaSupervisorio: %d\n", GetLastError());
	}

	// Controle bloqueio/desbloqueio da tarefa LePIMS
	hEventLePCP = CreateEvent(NULL, TRUE, FALSE, L"EventoLePCP"); //Evento de reset manual
	if (hEventLePCP == NULL) {
		printf("(EntradaTeclado): Erro na criação do EventoLePCP: %d\n", GetLastError());
	}

	// Controle bloqueio/desbloqueio da tarefa CapturaPIMS
	hEventCapturaPCP = CreateEvent(NULL, TRUE, FALSE, L"EventoCapturaPCP"); //Evento de reset manual
	if (hEventCapturaPCP == NULL) {
		printf("(EntradaTeclado): Erro na criação do EventoCapturaPCP: %d\n", GetLastError());
	}

	// Controle bloqueio/desbloqueio da tarefa CapturaPIMS
	hEventExibeAlarme = CreateEvent(NULL, TRUE, FALSE, L"EventoExibeAlarme");
	if (hEventExibeAlarme == NULL)
	{
		cout << "(main): Erro na criacao da handle pro EventoSDCD." << endl;
	}


	// --------------------------------------------------------------------------
	// Criação dos processos
	// --------------------------------------------------------------------------

	BOOL status;
	STARTUPINFO si;				    // StartUpInformation para novo processo
	PROCESS_INFORMATION ProcessSupervisorioPcp;	// Informações sobre novo processo criado

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes

	// Cria processo que gera dados do SDCD e alarmes do PIMS e escrevem na memoria
	status = CreateProcess(
		L"..\\x64\\Debug\\SupervisorioPcp.exe",								// Caminho do arquivo executável
		NULL,															// Apontador p/ parâmetros de linha de comando
		NULL,															// Apontador p/ descritor de segurança
		NULL,															// Idem, threads do processo
		NULL,															// Herança de handles
		NORMAL_PRIORITY_CLASS,											// Flags de criação
		NULL,															// Herança do ambiente de execução
		L"..\\x64\\Debug",												// Diretório do arquivo executável
		&si,															// lpStartUpInfo
		&ProcessSupervisorioPcp);												// lpProcessInformation
	if (!status) printf("(EntradaTeclado): Erro na criacao do SUPERVISORIO Codigo = %d\n", GetLastError());


	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes

	// Cria processo que gera dados do SDCD e alarmes do PIMS e escrevem na memoria
	//status = CreateProcess(
	//	L"..\\x64\\Debug\\Pcp_controler.exe", // Caminho do arquivo executável
	//	NULL,															// Apontador p/ parâmetros de linha de comando
	//	NULL,															// Apontador p/ descritor de segurança
	//	NULL,															// Idem, threads do processo
	//	NULL,															// Herança de handles
	//	CREATE_NEW_CONSOLE,												// Flags de criação
	//	NULL,															// Herança do ambiente de execução
	//	L"..\\x64\\Debug",												// Diretório do arquivo executável
	//	&si,															// lpStartUpInfo
	//	&ProcessPcp);											// lpProcessInformation
	//if (!status) printf("(EntradaTeclado): Erro na criacao do PCP Codigo = %d\n", GetLastError());



	// --------------------------------------------------------------------------
	// Leitura do teclado
	// --------------------------------------------------------------------------

	while (true)
	{
		nTecla = _getch();
		//printf("(EntradaTeclado): %c", nTecla);

		if (nTecla == ESC)
		{
			ret = SetEvent(hEscEvent);
			if (ret == NULL) {
				cout << "(EntradaTeclado): Erro ao setar EscEvent:" << GetLastError() << endl;
			}
			else if (ret != 0)
			{
				cout << "(EntradaTeclado): Sucesso ao setar EscEvent." << endl;
			}
			break;
		}// Fim tecla 'ESC'
		else if (nTecla == s)
		{
			//cout << "(EntradaTeclado): Tecla 's'." << endl;
			dwRet = WaitForSingleObject(hEventLeSupervisorio, 0);
			if (dwRet == WAIT_TIMEOUT)// Não estava sinalizado (estado == 0)
			{
				ret = SetEvent(hEventLeSupervisorio);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao setar EventLeSDCD:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventLeSDCD setado." << endl;
				}
			}
			else if (dwRet == WAIT_OBJECT_0)// Estava sinalizado (estado == 1)
			{
				ret = ResetEvent(hEventLeSupervisorio);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao resetar EventLeSDCD:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventLeSDCD resetado." << endl;
				}
			}
		} // Fim tecla 's'
		else if (nTecla == d)
		{
			//cout << "(EntradaTeclado): Tecla 'd'." << endl;
			dwRet = WaitForSingleObject(hEventCapturaSupervisorio, 0);
			if (dwRet == WAIT_TIMEOUT)// Não estava sinalizado (estado == 0)
			{
				ret = SetEvent(hEventCapturaSupervisorio);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao setar EventCapturaSDCD:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventCapturaSDCD setado." << endl;
				}
			}
			else if (dwRet == WAIT_OBJECT_0)// Estava sinalizado (estado == 1)
			{
				ret = ResetEvent(hEventCapturaSupervisorio);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao resetar EventCapturaSDCD:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventCapturaSDCD resetado." << endl;
				}
			}
		}// Fim tecla 'd'
		else if (nTecla == p)
		{
			//cout << "(EntradaTeclado): Tecla 's'." << endl;
			dwRet = WaitForSingleObject(hEventLePCP, 0);
			if (dwRet == WAIT_TIMEOUT)// Não estava sinalizado (estado == 0)
			{
				ret = SetEvent(hEventLePCP);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao setar EventLePIMS:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventLePIMS setado." << endl;
				}
			}
			else if (dwRet == WAIT_OBJECT_0)// Estava sinalizado (estado == 1)
			{
				ret = ResetEvent(hEventLePCP);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao resetar EventLePIMS:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventLePIMS resetado." << endl;
				}
			}
		}// Fim tecla 'p'
		else if (nTecla == a)
		{
			//cout << "(EntradaTeclado): Tecla 'd'." << endl;
			dwRet = WaitForSingleObject(hEventCapturaPCP, 0);
			if (dwRet == WAIT_TIMEOUT)// Não estava sinalizado (estado == 0)
			{
				ret = SetEvent(hEventCapturaPCP);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao setar EventCapturaPIMS:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventCapturaPIMS setado." << endl;
				}
			}
			else if (dwRet == WAIT_OBJECT_0)// Estava sinalizado (estado == 1)
			{
				ret = ResetEvent(hEventCapturaPCP);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao resetar EventCapturaPIMS:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventCapturaPIMS resetado." << endl;
				}
			}
		}// Fim tecla 'a'
		else if (nTecla == o)
		{
			continue;
		}// Fim tecla 'o'
		else if (nTecla == c)
		{
			//cout << "(EntradaTeclado): Tecla 'c'." << endl;
			dwRet = WaitForSingleObject(hEventExibeAlarme, 0);
			if (dwRet == WAIT_TIMEOUT)// Não estava sinalizado (estado == 0)
			{
				ret = SetEvent(hEventExibeAlarme);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao setar EventExibeAlarme:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventCapturaSDCD setado." << endl;
				}
			}
			else if (dwRet == WAIT_OBJECT_0)// Estava sinalizado (estado == 1)
			{
				ret = ResetEvent(hEventExibeAlarme);
				if (ret == NULL) {
					cout << "(EntradaTeclado): Erro ao resetar EventExibeAlarme:" << GetLastError() << endl;
				}
				else if (ret != 0)
				{
					//cout << "(EntradaTeclado): EventCapturaSDCD resetado." << endl;
				}
			}
		}// Fim tecla 'c'
		else
		{
			continue;
		}
	}


	// --------------------------------------------------------------------------
	// Aguarda término dos processos e encerra programa
	// --------------------------------------------------------------------------

	WaitForSingleObject(ProcessSupervisorioPcp.hProcess, INFINITE);
	//WaitForSingleObject(ProcessPcp.hProcess, INFINITE);

	//CheckForError(dwRet == WAIT_OBJECT_0);

	//Fecha todos os handles de objetos do kernel

	// Fechar handle dos processos que fica dentro da estrutura PROCESS_INFORMATION
	CloseHandle(hEscEvent);
	CloseHandle(hEventLeSupervisorio);

	CloseHandle(ProcessSupervisorioPcp.hThread);
	CloseHandle(ProcessSupervisorioPcp.hProcess);
	//CloseHandle(ProcessPcp.hThread);
	//CloseHandle(ProcessPcp.hProcess);

	return EXIT_SUCCESS;

}// ------------------------------- FIM Main -------------------------------

