//  Automa��o em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 8 - Sincronismo entre threads via Vari�veis de Condi��o na API Win32
//  ----------------------------------------------------------------------------
//
//  Vers�o 1.0 - 22/04/2010 - Prof. Luiz T. S. Mendes
//
//  NOTA: Este programa � funcionalmente id�ntico ao Exemplo 7, com a �nica
//  diferen�a de que as chamadas referentes � cria��o de threads e vari�veis de
//  condi��o foram portadas para a API Win32.
//
//	Este programa exemplifica o uso de vari�veis de condi��o para o sincronismo
//  entre threads. Para tal, a thread prim�ria do programa cria 5 threads
//  secund�rias, que ficam bloqueadas em 3 eventos distintos:
//
//    thread secund�ria   A       - desbloqueia-se quando as teclas "A" ou ESC
//                                  forem digitadas no teclado
//    thread secund�ria   B       - idem, referente �s teclas "B" ou ESC
//    threads secund�rias C, D, E - desbloqueiam-se ao mesmo tempo quando a
//                                  tecla de espa�o ou ESC forem acionados.
//
//  Estes eventos s�o modelados como vari�veis de condi��o. A tecla ESC,
//  quando digitada, sinaliza o t�rmino de execu��o para todas as threads.
//

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <process.h>	// _beginthreadex() e _endthreadex()
#include <stdio.h>
#include <conio.h>
#include <errno.h>

#define NTHREADS	5
#define	ESC			0x1B
#define TeclaA      0x61
#define TeclaB      0x62
#define TeclaEsp	0x20

// Casting para terceiro e sexto par�metros da fun��o _beginthreadex
typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);
typedef unsigned *CAST_LPDWORD;

// DECLARACAO DOS PROTOTIPOS DE FUNCAO CORRESPONDENTES �S THREADS SECUNDARIAS
DWORD WINAPI ThreadFunc(LPVOID);	// declara��o da fun��o


// VARIAVEIS GLOBAIS
long contador = 0;   // Variavel incrementada continuamente pelas threads
int  tecla = 0;      // Caracter digitado no teclado

// Declara��o das vari�veis de condi��o e CRITICAL SECTION
CONDITION_VARIABLE CondVarA, CondVarB, CondVarEsp;
CRITICAL_SECTION CritSec;

struct thread_data {int tecla;
		            int index;
                    CONDITION_VARIABLE *condvar;};

// THREAD PRIMARIA
int main()
{
	HANDLE hThreads[NTHREADS];
	int i;
	DWORD status, dwThreadID, dwExitCode;
	int teclas[5] = {TeclaA, TeclaB, TeclaEsp, TeclaEsp, TeclaEsp};
	CONDITION_VARIABLE *condvars[5] = {&CondVarA, &CondVarB, &CondVarEsp, &CondVarEsp, &CondVarEsp};
    thread_data parametros[5];

	//-----------------------------------------------------------------------------
	// Particularidade do Windows - Define o t�tulo da janela
	//-----------------------------------------------------------------------------

	SetConsoleTitle("Exemplo 1 - Criando threads via Pthreads");

	//-----------------------------------------------------------------------------
	// Cria��o da CRITICAL SECTION
	//-----------------------------------------------------------------------------

	InitializeCriticalSection(&CritSec);
	
	//-----------------------------------------------------------------------------
	// Cria��o das vari�veis de condi��o. S�o utilizados os atributos "default"
	// para estas vari�veis.
	//-----------------------------------------------------------------------------

	InitializeConditionVariable(&CondVarA);
	InitializeConditionVariable(&CondVarB);
    InitializeConditionVariable(&CondVarEsp);

	//-----------------------------------------------------------------------------
	// Loop de criacao das threads
	//-----------------------------------------------------------------------------

    for (i=0; i<NTHREADS; ++i) {	// cria 5 threads
		parametros[i].tecla = teclas[i];
		parametros[i].index = i;
		parametros[i].condvar = condvars[i];
		hThreads[i] = (HANDLE) _beginthreadex(
		                          NULL,
								  0,
								  (CAST_FUNCTION)ThreadFunc,	// casting necess�rio
								  (void *) &parametros[i],
								  0,
								  (CAST_LPDWORD)&dwThreadID	// cating necess�rio
		);
		
		if (hThreads[i]) printf("Thread %d criada com Id= %0d \n", i, dwThreadID);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());
	}	// for

	//-----------------------------------------------------------------------------
	// Aguarda usuario digitar a tecla ESC para encerrar
	//-----------------------------------------------------------------------------

	do {
		printf("Digite \"a\", \"b\", <Espaco> ou <Esc> para terminar:\n");
		tecla = _getch();

		if (tecla == TeclaA || tecla == ESC)
			WakeConditionVariable(&CondVarA);
		
		if (tecla == TeclaB || tecla == ESC)
			WakeConditionVariable(&CondVarB);

		if (tecla == TeclaEsp || tecla == ESC)
			WakeAllConditionVariable(&CondVarEsp);

	} while (tecla != ESC);

 	//-----------------------------------------------------------------------------
    // Aguarda termino das threads e fecha seus handles
	//-----------------------------------------------------------------------------

	status = WaitForMultipleObjects(NTHREADS, hThreads, TRUE, INFINITE);
	if (status != WAIT_OBJECT_0){
		printf ("Erro em WaitForMultipleObjects! Codigo = %d\n", GetLastError());
		return 0;
	}

	for (i=0; i<NTHREADS; ++i) {
		GetExitCodeThread(hThreads[i], &dwExitCode);
		printf("thread %d terminou: codigo=%d\n", i, dwExitCode); 
		CloseHandle(hThreads[i]);	// apaga refer�ncia ao objeto
	}

	/* DESTRUIR VARIAVEIS DE CONDICAO */

	return 0;
}

// THREADS SECUNDARIAS
DWORD WINAPI ThreadFunc(LPVOID thread_arg){
	int index;
	BOOL status;
	struct thread_data *parametros;
	CONDITION_VARIABLE *condvar;

	parametros = (struct thread_data *) thread_arg;
	index = parametros ->index;
	condvar = parametros->condvar;
	
	do {
		// Devemos nos lembrar do processo de espera por uma vari�vel de condi��o:
		// 1. Conquista-se a CRITICAL SECTION associada ao "predicado" a ser testado;
		// 2. while (PREDICADO NAO VERDADEIRO) {
		//      "Aguarda sinalizacao da variavel de condicao";
		//    }
		// 3. Libera CRITICAL SECTION associada � vari�vel de condi��o.
		//
		EnterCriticalSection(&CritSec);
		while (tecla != parametros->tecla){
		    status = SleepConditionVariableCS(parametros->condvar, &CritSec, INFINITE);
		    if (!status){
				printf ("Thread %d: Erro %d na espera da condicao!\n", index, GetLastError());
		        exit(0);
		    }
			printf ("Thread %d: condicao sinalizada! tecla = \"%c\"\n", index, tecla);
			if (tecla == ESC) break;
		}

		if (tecla != ESC) tecla = 0;
		LeaveCriticalSection(&CritSec);
	} while(tecla != ESC);

	_endthreadex((DWORD) index);
	// O comando "return" abaixo � desnecess�rio, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return index;
}
