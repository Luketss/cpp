//  Automa��o em tempo Real - ELT012 - UFMG1
//  
//  EXEMPLO 7 - Sincronismo entre threads via Vari�veis de Condi��o
//  ---------------------------------------------------------------
//
//  Vers�o 1.0 - 31/03/2010 - Prof. Luiz T. S. Mendes
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
//  Para a compila��o correta deste programa, assumindo que a biblioteca Pthreads
//  Win32 ja� esteja instalada em seu computador, fa�a os seguintes ajustes no
//  Visual Studio 2008 Express Edition:
//
//  1. Selecione Project -> Properties -> Configuration Properties -> C/C++ -> General
//     e defina em "Additional Include Directories" o diret�rio onde encontram-se os
//     "header files" da distribui��o pthreads, em geral
//
//          [...]\pthreads\Pre-built.2\include
// 
//  2. Selecione Project -> Properties -> Configuration Properties -> Linker -> General
//     e defina em "Additional Library Directories" o diret�rio onde se encontra
//     a biblioteca Pthreads (extens�o .LIB), em geral
//
//          [...]\pthreads\Pre-built.2\lib
//  
//  3. Selecione Project -> Properties -> Configuration Properties -> Linker -> Input
//     e declare a biblioteca "pthreadVC2.lib" em "Additional Dependencies".
//

#include <windows.h>
#include <stdio.h>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <conio.h>
#include <errno.h>

#define NTHREADS	5
#define	ESC			0x1B
#define TeclaA      0x61
#define TeclaB      0x62
#define TeclaEsp	0x20

// DECLARACAO DOS PROTOTIPOS DE FUNCAO CORRESPONDENTES �S THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void* ThreadFunc(void* arg);


// VARIAVEIS GLOBAIS
long contador = 0;   // Variavel incrementada continuamente pelas threads
int  tecla = 0;      // Caracter digitado no teclado

// Declara��o das vari�veis de condi��o e mutex
pthread_cond_t CondVarA, CondVarB, CondVarEsp;
//pthread_cond_t *condvars[5] = {&CondVarA, &CondVarB, &CondVarEsp, &CondVarEsp, &CondVarEsp};
pthread_mutex_t Mutex;
pthread_mutexattr_t MutexAttr;

struct thread_data {int tecla;
		            int index;
                    pthread_cond_t *condvar;};

// THREAD PRIMARIA
int main()
{
	pthread_t hThreads[NTHREADS];
	int i;
	int status;
    void *tRetStatus;
	int teclas[5] = {TeclaA, TeclaB, TeclaEsp, TeclaEsp, TeclaEsp};
	pthread_cond_t *condvars[5] = {&CondVarA, &CondVarB, &CondVarEsp, &CondVarEsp, &CondVarEsp};
    thread_data parametros[5];

	//-----------------------------------------------------------------------------
	// Particularidade do Windows - Define o t�tulo da janela
	//-----------------------------------------------------------------------------

	SetConsoleTitle("Exemplo 1 - Criando threads via Pthreads");

	//-----------------------------------------------------------------------------
	// Cria��o de mutex do tipo PTHREAD_MUTEX_ERRORCHECK
	// Coment�rios espec�ficos sobre esta a��o est�o contidos no programa
	// de exemplo 4
	//-----------------------------------------------------------------------------

	pthread_mutexattr_init(&MutexAttr); //sempre retorna 0
    status = pthread_mutexattr_settype(&MutexAttr, PTHREAD_MUTEX_ERRORCHECK);
	if (status != 0){
		printf ("Erro nos atributos do Mutex ! Codigo = %d\n", status);
		exit(0);
	}
	status = pthread_mutex_init(&Mutex, &MutexAttr);
	if (status !=0){
		printf ("Erro na criacao do Mutex! Codigo = %d\n", status);
		return 0;
	}

	//-----------------------------------------------------------------------------
	// Cria��o das vari�veis de condi��o. S�o utilizados os atributos "default"
	// para estas vari�veis.
	//-----------------------------------------------------------------------------

	status = pthread_cond_init(&CondVarA, NULL);
	if (status !=0){
		printf ("Erro na criacao de CondVarA! Codigo = %d\n", status);
		return 0;
	}
	status = pthread_cond_init(&CondVarB, NULL);
	if (status !=0){
		printf ("Erro na criacao de CondVarB! Codigo = %d\n", status);
		return 0;
	}
	status = pthread_cond_init(&CondVarEsp, NULL);
	if (status !=0){
		printf ("Erro na criacao de CondVarEsp! Codigo = %d\n", status);
		return 0;
	}

	//-----------------------------------------------------------------------------
	// Loop de criacao das threads
	//-----------------------------------------------------------------------------

    for (i=0; i<NTHREADS; ++i) {	// cria 5 threads
		parametros[i].tecla = teclas[i];
		parametros[i].index = i;
		parametros[i].condvar = condvars[i];
		status = pthread_create(&hThreads[i], NULL, ThreadFunc, (void *) &parametros[i]);
		if (!status) printf("Thread %d criada com Id= %0d \n", i, (int) &hThreads[i]);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", i, status);
	}	// for

	//-----------------------------------------------------------------------------
	// Aguarda usuario digitar a tecla ESC para encerrar
	//-----------------------------------------------------------------------------

	do {
		printf("Digite \"a\", \"b\", <Espaco> ou <Esc> para terminar:\n");
		tecla = _getch();
		if (tecla == TeclaA || tecla == ESC){
			status = pthread_cond_signal(&CondVarA);
	        if (status !=0){
		       printf ("Erro na sinalizacao de CondVarA! Codigo = %d\n", status);
		       return 0;
	        }
		}
		if (tecla == TeclaB || tecla == ESC){
			status = pthread_cond_signal(&CondVarB);
	        if (status !=0){
		       printf ("Erro na sinalizacao de CondVarB! Codigo = %d\n", status);
		       return 0;
	        }
		}
		if (tecla == TeclaEsp || tecla == ESC){
			status = pthread_cond_broadcast(&CondVarEsp);
	        if (status !=0){
		       printf ("Erro na sinalizacao de CondVarEsp! Codigo = %d\n", status);
		       return 0;
	        }
		}
	} while (tecla != ESC);

 	//-----------------------------------------------------------------------------
    // Aguarda termino das threads
	//-----------------------------------------------------------------------------

	for (i=0; i<NTHREADS; i++){
	   printf("Aguardando termino da thread %d...\n", (int) &hThreads[i]);
	   pthread_join(hThreads[i], &tRetStatus);
	   printf ("Thread %d: status de retorno = %d\n", i, (int) tRetStatus);
	}

	//-----------------------------------------------------------------------------
	// Destrui��o do Mutex e das vari�veis de condi��o
	//-----------------------------------------------------------------------------

	pthread_mutex_destroy(&Mutex);
	if (status !=0){
		printf ("Erro na destruicao do Mutex! C�digo = %d\n", status);
		exit(0);
	}
	pthread_cond_destroy(&CondVarA);
	if (status !=0){
		printf ("Erro na destruicao de CondVarA! C�digo = %d\n", status);
		exit(0);
	}
	pthread_cond_destroy(&CondVarB);
	if (status !=0){
		printf ("Erro na destruicao de CondVarB! C�digo = %d\n", status);
		exit(0);
	}
	pthread_cond_destroy(&CondVarEsp);
	if (status !=0){
		printf ("Erro na destruicao de CondVarEsp! C�digo = %d\n", status);
		exit(0);
	}

	return EXIT_SUCCESS;
}	// main

// THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void *ThreadFunc(void *thread_arg){
	int index, status;
	struct thread_data *parametros;
	pthread_cond_t *condvar;

	parametros = (struct thread_data *) thread_arg;
	index = parametros ->index;
	condvar = parametros->condvar;
	
	do {
		// Devemos nos lembrar do processo de espera por uma vari�vel de condi��o:
		// 1. Conquista-se o MUTEX associado � vari�vel de condi��o;
		// 2. while (CONDICAO NAO VERDADEIRA) {
		//      "Aguarda sinalizacao da variavel de condicao";
		//    }
		// 3. Libera MUTEX associado � vari�vel de condi��o.
		//
		status = pthread_mutex_lock(&Mutex);
		if (status !=0){
		  printf ("Erro na conquista do Mutex! Codigo = %d\n", status);
		  exit(0);
		}
		while (tecla != parametros->tecla){
		    status = pthread_cond_wait(parametros->condvar, &Mutex);
		    if (status !=0){
				printf ("Thread %d: Erro %d na espera da condicao!\n", index, status);
		        exit(0);
		    }
			printf ("Thread %d: condicao sinalizada! tecla = \"%c\"\n", index, tecla);
			if (tecla == ESC) break;
		}

		if (tecla != ESC) tecla = 0;
		status = pthread_mutex_unlock(&Mutex);
		if (status !=0){
		  printf ("Erro na liberacao do Mutex! Codigo = %d\n", status);
		  exit(0);
		}
	} while(tecla != ESC);

	pthread_exit((void *)index);
	// O comando "return" abaixo � desnecess�rio, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return (void *) index;
}
