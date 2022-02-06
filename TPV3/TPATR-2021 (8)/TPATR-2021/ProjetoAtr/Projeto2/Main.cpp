// Processo 3 tera uma unica thread, Tarefa de exibição dos dados do processo
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>						
#include <conio.h>	


#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN      | FOREGROUND_BLUE

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto parâmetros da função
													// _beginthreadex
typedef unsigned* CAST_LPDWORD;

void WINAPI tarefa_exibicao_dodos_processo(LPVOID id);
void imprime_dado_processo(char*);

HANDLE oEvent;
HANDLE escEvent;
HANDLE hOut;
HANDLE hfile;
HANDLE harquivo;
HANDLE hocupado2;


int main() {

	Sleep(1000); // Necessario para garantir que o evento seja criado antes da funçao open_event

	DWORD dwStatus;
	HANDLE hThread;
	DWORD dwIdThread;
	DWORD dwExitCode = 0;
	DWORD dwRet;

	oEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"oEvento");
	if (oEvent == NULL) {
		printf("Falha ao abrir o evento 'o':%d\n", GetLastError());
	}
	escEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"escEvento");
	if (escEvent == NULL) {
		printf("Falha ao abrir o evento 'ESC':%d\n", GetLastError());
	}
	hocupado2 = harquivo = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"P_ocupado2");;
	if (hocupado2 == NULL) {
		printf("Falha ao abrir o evento 'ESC':%d\n", GetLastError());
	};
	harquivo = OpenEvent(EVENT_ALL_ACCESS, FALSE, L"harquivo");
	if (harquivo == NULL) {
		printf("Falha ao abrir o evento 'ESC':%d\n", GetLastError());
	}
	WaitForSingleObject(harquivo, INFINITE);

	hfile = CreateFile(L"Dados.txt", // nome do arquivo
		GENERIC_READ | GENERIC_WRITE, // leitura e escrita
		FILE_SHARE_READ | FILE_SHARE_WRITE, // compartilhado para leitura e escrita
		NULL,		// atributos de segurança 
		OPEN_EXISTING,// cria novo arquivo 
		FILE_ATTRIBUTE_NORMAL,
		NULL);		// Template para atributos e flags


	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a saída da console\n");

	hThread = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)tarefa_exibicao_dodos_processo,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThread);
	if (hThread)
		printf("Thread tarefa exibicao de dados do processo! Id=%0x\n", dwIdThread);
	else {
		printf("Erro na criacao da thread tarefa exibiçao de dados do processo! N = %d Erro = %d\n", 0, errno);
		exit(0);
	}

	dwRet = WaitForSingleObject(escEvent, INFINITE);
	if (dwRet == WAIT_FAILED) {
		printf("Falha na funcao WaitForSingleObject:%d\n", GetLastError);
	}

	CloseHandle(oEvent);
	CloseHandle(escEvent); 
	CloseHandle(hOut);
	CloseHandle(hfile);
	CloseHandle(harquivo);
	CloseHandle(hocupado2);

	return EXIT_SUCCESS;

}

void WINAPI tarefa_exibicao_dodos_processo(LPVOID id) {



	HANDLE Events[3] = { oEvent, escEvent, hocupado2 };
	HANDLE Events2[2] = { oEvent, escEvent};
	DWORD ret;
	DWORD status;
	DWORD dwBytesRead;
	char dado[53];
	int nTipoEvento;
	int count = 1;

	do {
		ret = WaitForMultipleObjects(3, Events, FALSE, INFINITE);
		if (ret == WAIT_FAILED) {
			printf("Falha na funcao WaitForMultipleObjects:%d\n", GetLastError);
		}
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
			if (ret == WAIT_FAILED) {
				printf("Falha na funcao WaitForMultipleObjects:%d\n", GetLastError);
			}
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 1) {
				break;
			}
			else {
				//continua
			}
		}
		else if (nTipoEvento == 1) {
			break;
		}
		else {
			if (count > 100) {
				SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
				count = 1;
			}
			status = ReadFile(hfile, &dado, sizeof(dado), &dwBytesRead, NULL);
			imprime_dado_processo(dado);
			count++;
		}
	} while (1);    // Esc foi escolhido
	printf("Thread  de exibicao dos dados do processo (%d) terminando...\n", id);

	_endthreadex(0);
	return;
}

void imprime_dado_processo(char* resultado) {

	SetConsoleTextAttribute(hOut, 14);
	printf("NSEQ= ");
	for (int i = 0; i < 6; i++) {
		printf("%c", resultado[i]);
	}
	printf(" HORA= ");

	for (int i = 38; i < strlen(resultado); i++) {
		printf("%c", resultado[i]);
	}
	printf(" TAG= ");
	for (int i = 9; i < 19; i++) {
		printf("%c", resultado[i]);
	}

	printf(" VALOR= ");
	for (int i = 20; i < 28; i++) {
		printf("%c", resultado[i]);
	}
	printf(" UE =");
	for (int i = 28; i < 36; i++) {
		printf("%c", resultado[i]);
	}
	printf(" MODO =");
	printf("%c", resultado[36]);
	printf("\n");
	SetConsoleTextAttribute(hOut, WHITE);
}