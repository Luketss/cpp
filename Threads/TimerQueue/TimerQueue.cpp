//  Automa��o em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 8 - Temporiza��o via "Timer Queues"
//  ---------------------------------------------------------------
//
//  Vers�o 1.0 - 20/04/2010 - Prof. Luiz T. S. Mendes
//
//	Este programa exemplifica o uso de  "Timer Queues" para fins de temporiza��o,
//  e baseia-se integralmente no Programa 4.5 do livro "Programa��o Concorrente
//  em ambiente Windows", de C. S. Filho e M. Szuster. As �nicas diferen�as
//  com rela��o a este �ltimo s�o:
//  - O uso de "Timer Queues" ao inv�s "Timers Multim�dia";
//  - A thread secund�ria enfileira uma segunda rotina callback (TimerFunc2)
//    na fila de temporizadores, para fins de demonstra��o da fila de mensagens.
//
//  A API de "Queue Timers" s� existe nas vers�es do Microsoft Windows 2000 em
//  diante, portanto este programa n�o compilar� em vers�es anteriores dos S.O.
//  Microsoft.
//

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>		// _getch
#include <ctype.h>		// _isdigit
#include <process.h>	// _beginthreadex() e _endthreadex() 

#include "bGetFloat.h"	// le valor floating point

#define _CHECKERROR	1	// Ativa fun��o CheckForError
#include "CheckForError.h"

// Casting para terceiro e sexto par�metros da fun��o _beginthreadex
typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);
typedef unsigned *CAST_LPDWORD;

#define	ESC			0x1B
#define CR			0x0D

HANDLE hEvent;			// Handle para Evento
HANDLE hEscEvent;		// Handle para Evento Aborta

DWORD WINAPI PidControlFunc(LPVOID);	  // declara��o da fun��o
void CALLBACK Pid(PVOID, BOOLEAN);	      // Fun��o PID
void CALLBACK TimerFunc2(PVOID, BOOLEAN); // Fun��o de temporiza��o secund�ria
void CALLBACK TimerFunc3(PVOID, BOOLEAN); // Fun��o de temporiza��o secund�ria

DOUBLE SetPoint= 0.0;
DOUBLE dInput;

SYSTEMTIME tempo;

int main()
{
	HANDLE hThread;
	
	DWORD dwThreadId;
	DWORD dwExitCode = 0;
	BOOL  bStatus;

	// apenas uma thread � acordada a cada pulso
	hEvent		= CreateEvent(NULL, FALSE, FALSE,"Evento");
	CheckForError(hEvent);
	// todas as threads s�o acordadas a cada pulso
	hEscEvent	= CreateEvent(NULL, TRUE, FALSE,"EscEvento");
	CheckForError(hEscEvent);
	// Cria controlador PID
	hThread = (HANDLE) _beginthreadex(
			NULL,
			0,
			(CAST_FUNCTION)PidControlFunc,	// casting necess�rio
			(LPVOID)0,
			0,
			(CAST_LPDWORD)&dwThreadId		// casting necess�rio
		);
	CheckForError(hThread);
	
	do {
		printf("\nEscreva novo valor de SetPoint:\n");
		// le string ou ESC
		bStatus = bGetFloat(&dInput, 9);
		if (bStatus) PulseEvent(hEvent); // SetPoint mudou
		else PulseEvent(hEscEvent);		// Termina threads
	} while (bStatus);

	// Espera thread terminar
	if (WaitForSingleObject(hThread, INFINITE) != WAIT_OBJECT_0) 
		printf ("Erro em WaitForSingleObject! Codigo = %d\n", (int) GetLastError);
	GetExitCodeThread(hThread, &dwExitCode);

	CloseHandle(hThread);	// apaga refer�ncia ao objeto
	CloseHandle(hEvent);
	CloseHandle(hEscEvent);

	return EXIT_SUCCESS;
}  // main


DWORD WINAPI PidControlFunc(LPVOID id)
{	
	HANDLE Events[2]= {hEvent, hEscEvent};
	DWORD ret;
	int TipoEvento= 0;

	HANDLE hTimerQueue;
	HANDLE hTimerID1, hTimerID2, hTimerID3;
	BOOL status;
	
	// Cria fila de temporizadores
	hTimerQueue = CreateTimerQueue();
	if (hTimerQueue == NULL){
        printf("Falha em CreateTimerQueue! Codigo =%d)\n", GetLastError());
        return 0;
    }
	// Enfileira primeiro temporizador com sua fun��o callback
	status = CreateTimerQueueTimer(&hTimerID1, hTimerQueue, (WAITORTIMERCALLBACK)Pid,
								   NULL, 7000, 5000, WT_EXECUTEDEFAULT);
								   //NULL, 1000, 1000, WT_EXECUTEDEFAULT);
	if (!status){
        printf("Erro em CreateTimerQueueTimer [1]! Codigo = %d)\n", GetLastError());
        return 0;
    }
    // Enfileira segundo temporizador com sua fun��o callback
	status = CreateTimerQueueTimer(&hTimerID2, hTimerQueue, (WAITORTIMERCALLBACK)TimerFunc2,
								   NULL, 1000, 1000, WT_EXECUTEDEFAULT);
	if (!status){
        printf("Erro em CreateTimerQueueTimer [2]! Codigo = %d)\n", GetLastError());
        return 0;
	}

    // Enfileira segundo temporizador com sua fun��o callback
	status = CreateTimerQueueTimer(&hTimerID3, hTimerQueue, (WAITORTIMERCALLBACK)TimerFunc3,
								   NULL, 3000, 3000, WT_EXECUTEDEFAULT);
	if (!status){
        printf("Erro em CreateTimerQueueTimer [3]! Codigo = %d)\n", GetLastError());
        return 0;
    }

	do {// observe: quando ocorre evento a temporiza��o � perdida !  
		ret=WaitForMultipleObjects(2, Events, FALSE, INFINITE);
		TipoEvento = ret - WAIT_OBJECT_0;
		if (TipoEvento == 0){// Novo valor de Pid
			SetPoint= dInput;
			printf("Novo valor de set point: %6.2f\n", SetPoint);
		}
	} while (TipoEvento != 1);	// Esc foi escolhido

	// Cancela a fila de temporizadores
	if (!DeleteTimerQueueEx(hTimerQueue, NULL))
        printf("Falha em DeleteTimerQueue! Codigo = %d\n", GetLastError());
	
	printf("Thread Pid terminando...\n");	

	printf("\nAcione uma tecla para terminar\n");
	_getch(); // // Pare aqui, caso n�o esteja executando no ambiente MDS

	_endthreadex((DWORD) 0);

	return 0;
} // WaitEventFunc

void CALLBACK Pid(PVOID nTimerID, BOOLEAN TimerOrWaitFired)
{	// Algoritmo Pid;
	//printf("Pid rodando... SP= %6.2f\n", SetPoint);
	GetLocalTime(&tempo);
	printf("Pid rodando... SP= %6.2f  %02d:%02d:%02d\n", SetPoint, tempo.wHour,
		   tempo.wMinute, tempo.wSecond);
};	// Pid

void CALLBACK TimerFunc2(PVOID nTimerID, BOOLEAN TimerOrWaitFired){
	// Rotina callback para o segundo temporizador
	GetLocalTime(&tempo);
	printf("Rotina Secundaria executando... %02d:%02d:%02d\n", tempo.wHour,
		   tempo.wMinute, tempo.wSecond);
};	// Pid
void CALLBACK TimerFunc3(PVOID nTimerID, BOOLEAN TimerOrWaitFired){
	// Rotina callback para o segundo temporizador
	GetLocalTime(&tempo);
	printf("Rotina Terciaria executando... %02d:%02d:%02d\n", tempo.wHour,
		   tempo.wMinute, tempo.wSecond);
};	// Pid
