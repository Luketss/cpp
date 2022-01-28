//  Automa��o em tempo Real - ELT012 - UFMG
//  
//  EXEMPLO 2b - Cria��o de threads com sincronismo via pthread_join
//  ----------------------------------------------------------------
//
//  Vers�o 1.0 - 26/02/2010 - Prof. Luiz T. S. Mendes
//
//	NOTA: Este programa � funcionalmente equivalente ao programa "Exemplo 1 -
//  Cria��o de Threads" contido no Cap. 2 do livro "Programa��o Multithreaded
//  em ambiente Windows NT� - uma vis�o de  Automa��o", autores Constantino
//  Seixas Filho e Marcelo Szuster.
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
#include <errno.h>

// DECLARACAO DO PROTOTIPO DE FUNCAO CORRESPONDENTES �S THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void* TestFunc(void* arg);

// THREAD PRIMARIA
int main()
{
	pthread_t hThreads[3];
	int i;
	int status;
    void *tRetStatus;
	
	// Particularidade do Windows - Define o t�tulo da janela
	SetConsoleTitle("Exemplo 1 - Criando threads via Pthreads");


	// Loop de criacao das threads
    for (i=0; i<3; ++i) {	// cria 3 threads	
		status = pthread_create(&hThreads[i], NULL, TestFunc, (void *) i);
		if (!status) printf("Thread %d criada com Id= %0d \n", i, (int) &hThreads[i]);
		else printf ("Erro na criacao da thread %d! Codigo = %d\n", i, status);
	}	// for

    // Aguarda termino das threads
	for (i=0; i<3; i++){
	   printf("Aguardando termino da thread %d...\n", (int) &hThreads[i]);
	   pthread_join(hThreads[i], &tRetStatus);
	   printf ("Thread %d: status de retorno = %d\n", i, (int) tRetStatus);
	}

	//return EXIT_SUCCESS;
	pthread_exit(NULL);
}	// main

// THREADS SECUNDARIAS
// Atencao para o formato conforme o exemplo abaixo!
void *TestFunc(void *arg){
	int i, index;
	
	index = (int) arg;
	for(i=0; i<100000; ++i) {
		printf("%d ", index);
		Sleep(1);	// delay de 1 ms
	}
	printf("\n");
	pthread_exit((void *)index);
	// O comando "return" abaixo � desnecess�rio, mas presente aqui para compatibilidade
	// com o Visual Studio da Microsoft
	return (void *) index;
} // TestFunc
