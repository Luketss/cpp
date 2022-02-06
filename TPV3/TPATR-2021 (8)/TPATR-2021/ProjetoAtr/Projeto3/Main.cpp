// Processo 3 tera uma unica thread, Tarefa de exibição de alarmes
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>						
#include <conio.h>	

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN      | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto parâmetros da função
													// _beginthreadex
typedef unsigned* CAST_LPDWORD;

void WINAPI tarefa_exibicao_alarmes(LPVOID id);
void imprime_alarme(char* resultado);

#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN      | FOREGROUND_BLUE

HANDLE cEvent;
HANDLE escEvent;
HANDLE hOut;
HANDLE hMailslot;
HANDLE mEvent;

int main() {

	Sleep(1000); // Necessario para garantir que o evento seja criado antes da funçao open_event
	// trocar pra waitabletimer

	DWORD dwStatus;
	HANDLE hThread;
	DWORD dwIdThread;
	DWORD dwExitCode = 0;
	DWORD dwRet;

	
	mEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"mEvent");
	if (mEvent == NULL) {
		printf("Falha ao abrir o evento 'mEvent':%d\n", GetLastError());
	}
	cEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"cEvento");
	if (cEvent == NULL) {
		printf("Falha ao abrir o evento 'c':%d\n", GetLastError());
	}
	escEvent = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"escEvento");
	if (escEvent == NULL) {
		printf("Falha ao abrir o evento 'ESC':%d\n", GetLastError());
	}
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a saída da console\n");

	hThread = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)tarefa_exibicao_alarmes,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThread);
	if (hThread)
		printf("Thread tarefa exibicao de alarmes! Id=%0x\n", dwIdThread);
	else {
		printf("Erro na criacao da thread tarefa exibiçao de alarmes! N = %d Erro = %d\n", 0, errno);
		exit(0);
	}

	dwRet = WaitForSingleObject(escEvent, INFINITE);
	if (dwRet == WAIT_FAILED) {
		printf("Falha na funcao WaitForSingleObject:%d\n", GetLastError);
	}

	CloseHandle(cEvent);
	CloseHandle(escEvent);
	CloseHandle(hOut);
	CloseHandle(hMailslot);
	CloseHandle(mEvent);

	return EXIT_SUCCESS;

}

void WINAPI tarefa_exibicao_alarmes(LPVOID id) {
	HANDLE Events[2] = { cEvent, escEvent};

	DWORD ret;
	DWORD status;
	int nTipoEvento;
	DWORD dwBytesLidos;
	char chBuff[36];
	//criando o mailslot
	hMailslot = CreateMailslot(
		L"\\\\.\\mailslot\\Debug\\CaixaPostal",
		0,
		MAILSLOT_WAIT_FOREVER,
		NULL);

	SetEvent(mEvent);

	do {
		ret = WaitForMultipleObjects(2, Events, FALSE, 1000);
		if (ret == WAIT_FAILED) {
			printf("Falha na funcao WaitForMultipleObjects:%d\n", GetLastError);
		}
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
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
		else {//O TEMPO DE ESPERA ACABOU E ELA LEU O MAILSLOT
			status = ReadFile(hMailslot, &chBuff, sizeof(chBuff), &dwBytesLidos, NULL);
			imprime_alarme(chBuff);
		}
	} while (1);    // Esc foi escolhido
	printf("Thread de exibicao de alarmes (%d) terminando...\n", id);

	_endthreadex(0);
	return;
}

void imprime_alarme(char* resultado) {


	if (resultado[7] == '9') {
		SetConsoleTextAttribute(hOut, HLRED);
		printf("HORA= ");
		for (int i = 23; i < strlen(resultado); i++) {
			printf("%c", resultado[i]);
		}
		printf(" NSEQ= ");
		for (int i = 0; i < 6; i++) {
			printf("%c", resultado[i]);
		}
		printf(" ID ALARME= ");
		for (int i = 9; i < 13; i++) {
			printf("%c", resultado[i]);
		}
		printf(" GRAU= ");
		for (int i = 14; i < 16; i++) {
			printf("%c", resultado[i]);
		}
		printf(" PREV= ");
		for (int i = 17; i < 22; i++) {
			printf("%c", resultado[i]);
		}
		printf("\n");
		SetConsoleTextAttribute(hOut, WHITE);
	}
	else {
		SetConsoleTextAttribute(hOut, HLGREEN);
		printf("HORA= ");
		for (int i = 23; i < strlen(resultado); i++) {
			printf("%c", resultado[i]);
		}
		printf(" NSEQ= ");
		for (int i = 0; i < 6; i++) {
			printf("%c", resultado[i]);
		}
		printf(" ID ALARME=");
		for (int i = 9; i < 13; i++) {
			printf("%c", resultado[i]);
		}
		printf(" GRAU= ");
		for (int i = 14; i < 16; i++) {
			printf("%c", resultado[i]);
		}
		printf(" PREV= ");
		for (int i = 17; i < 22; i++) {
			printf("%c", resultado[i]);
		}
		printf("\n");
		SetConsoleTextAttribute(hOut, WHITE);
	}
}