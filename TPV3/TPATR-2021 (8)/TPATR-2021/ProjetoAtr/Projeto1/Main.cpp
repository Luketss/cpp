// O processo 1 sera responsavel por criar 4 threads ( Tarefa Leitura do SDCD,Tarefa de Leitura do PIMS,
//Tarefa de Captura de dados do processa, Tarefa de captura de alarmes )
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 
#define _WIN32_WINNT 0X0500

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>						
#include <conio.h>
#include <time.h>
#include <string.h>


typedef unsigned (WINAPI* CAST_FUNCTION)(LPVOID);	// Casting para terceiro e sexto parâmetros da função
													// _beginthreadex
typedef unsigned* CAST_LPDWORD;


#define BUFFSIZE 70
#define MAX_MENSAGENS 100
#define WHITE   FOREGROUND_RED   | FOREGROUND_GREEN      | FOREGROUND_BLUE
#define HLGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define HLRED   FOREGROUND_RED   | FOREGROUND_INTENSITY

#define	ESC				0x1B	// Tecla para encerrar o programa
#define s				0x73    // Tecla para sinalizar o evento que bloqueia/desbloqueia a thread SDCD
#define c				0x63    // Tecla para sinalizar o evento que bloqueia/desbloqueia a thread exibiçao de dados do alarme
#define d				0x64    // Tecla para sinalizar o evento que bloqueia/desbloqueia a thread de captura de dados do processo
#define p				0x70    // Tecla para sinalizar o evento que bloqueia/desbloqueia a thread PIMS
#define a				0x61    // Tecla para sinalizar o evento que bloqueia/desbloqueia a thread de captura de dados de alarmes
#define o				0x6F    // Tecla para sinalizar o evento que bloqueia/desbloqueia a thread exibiçao de dados do processo

char* buffer[MAX_MENSAGENS];
int P_livre, P_ocupado_dado, P_ocupado_alarme = 0;


HANDLE hmutex;						// Handle proteção Plivre, Pocupado
HANDLE hlivre;						// Handle semaforo contador de posiçoes livre buffer
HANDLE hocupado;					// Handle semaforo binario de posiçoes livre buffer
HANDLE sEvent;						// Handle para evento s
HANDLE cEvent;						// Handle para evento c
HANDLE dEvent;						// Handle para evento d
HANDLE pEvent;						// Handle para evento p
HANDLE aEvent;						// Handle para evento a
HANDLE oEvent;						// Handle para evento o
HANDLE escEvent;					// Handle para evento ESC
HANDLE Print;						// Handle para o mutex que protege a impressao da tela
HANDLE hOut;						// Handle para a saida do console
 
HANDLE hfile;						// Handle para o arquivo circular em disco
HANDLE harquivo;					// Handle para o evento que sinaliza a criaçao do arquivo
HANDLE hMailslot;					// Handle para o mailslot
HANDLE hMailslot1;
HANDLE mEvent;						// Handle para o evento que ira sinalizar que o servidor criou o mailslot

HANDLE hocupado2;					// Handle para o semaforo responsavel por limitar a escrita no arquivo em disco

HANDLE htimer;						// Handle para o waitable timer thread SDCD
HANDLE htimercritico;				// Handle para o waitable timer de criaçao alarme critico PIMS
HANDLE htimer_nao_critico;			// Handle para o waitable timer de criaçao alarme nao critico PIMS

void WINAPI tarefa_leitura_do_SDCD(LPVOID id);
void WINAPI tarefa_leitura_do_PIMS(LPVOID id);
void WINAPI tarefa_captura_dados_processo(LPVOID id);
void WINAPI tarefa_captura_alarmes(LPVOID id);
char* Tag();
char* Gerar_Msg_SDCD(int NSEQ);
char* Unidade();
char* Modo();
char* Hora();
char* Valor();
char* _NSEQ(int);
void imprime_dado_processo(char*);
void imprime_alarme(char*);
char* Gerar_Msg_PIMS(int);
char* Gerar_Msg_PIMS_Critico(int);
char* ID(void);
char* Grau(void);
char* PREV(void);
int gera_numero_aleatorio(int valor_min, int valor_max);


int main() {

	DWORD dwStatus;

	HANDLE hThreads[4];       // 4 threads neste executável 
	DWORD dwIdThreads[4];
	DWORD dwExitCode = 0;
	DWORD dwRet;
	int i = 0;
	int nTecla;


	for (int i = 0; i < MAX_MENSAGENS; i++) {
		buffer[i] = (char*)malloc((53) * sizeof(char));
	}



	//Inicializaçao Waitable timer
	htimer = CreateWaitableTimer(NULL, FALSE, L"tempo");
	htimer_nao_critico = CreateWaitableTimer(NULL, FALSE, L"alarme");
	htimercritico = CreateWaitableTimer(NULL, FALSE, L"alarmecritico");

	//Inicializaçao semforos
	hmutex = CreateSemaphore(NULL, 1, 1, L"mutex");
	if (hmutex == NULL) {
		printf("Erro na criação do mutex: %d\n", GetLastError());
	}
	hlivre = CreateSemaphore(NULL, 100, 100, L"P_livre");
	if (hlivre == NULL) {
		printf("Erro na criação do semaforo hlivre: %d\n", GetLastError());
	}
	hocupado = CreateSemaphore(NULL, 0, 100, L"P_ocupado");
	if (hocupado == NULL) {
		printf("Erro na criação do semaforo hocupado: %d\n", GetLastError());
	}
	hocupado2 = CreateSemaphore(NULL, 0, 100, L"P_ocupado2");
	if (hocupado == NULL) {
		printf("Erro na criação do semaforo hocupado: %d\n", GetLastError());
	}
	Print = CreateSemaphore(NULL, 1, 1, L"Print_mutex");
	if (Print == NULL) {
		printf("Erro na criação do mutex para os prints: %d\n", GetLastError());
	}
	



	// Obtendo handle saida console
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		printf("Erro ao obter handle para a saída da console\n");

	// Inicializando os eventos de sincronização
	sEvent = CreateEvent(NULL, FALSE, FALSE, L"sEvento");
	if (sEvent == NULL) {
		printf("Falha ao criar o evento sEvent: %d\n", GetLastError());
	}
	cEvent = CreateEvent(NULL, FALSE, FALSE, L"cEvento");
	if (cEvent == NULL) {
		printf("Falha ao criar o evento cEvent: %d\n", GetLastError());
	}
	dEvent = CreateEvent(NULL, FALSE, FALSE, L"dEvento");
	if (dEvent == NULL) {
		printf("Falha ao criar o evento dEvent: %d\n", GetLastError());
	}
	pEvent = CreateEvent(NULL, FALSE, FALSE, L"pEvento");
	if (pEvent == NULL) {
		printf("Falha ao criar o evento pEvent: %d\n", GetLastError());
	}
	aEvent = CreateEvent(NULL, FALSE, FALSE, L"aEvento");
	if (aEvent == NULL) {
		printf("Falha ao criar o evento aEvent: %d\n", GetLastError());
	}
	oEvent = CreateEvent(NULL, FALSE, FALSE, L"oEvento");
	if (oEvent == NULL) {
		printf("Falha ao criar o evento oEvent: %d\n", GetLastError());
	}
	escEvent = CreateEvent(NULL, TRUE, FALSE, L"escEvento");
	if (escEvent == NULL) {
		printf("Falha ao criar o evento escEvent: %d\n", GetLastError());
	}
	harquivo = CreateEvent(NULL, FALSE, FALSE, L"harquivo");
	if (oEvent == NULL) {
		printf("Falha ao criar o evento oEvent: %d\n", GetLastError());
	}
	mEvent = CreateEvent(NULL, TRUE, FALSE, L"mEvent");
	if (mEvent == NULL) {
		printf("Falha ao criar o evento escEvent: %d\n", GetLastError());
	}
	//Criando Threads
	hThreads[0] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)tarefa_leitura_do_SDCD,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwIdThreads[0]);
	if (hThreads[0])
		printf("Thread leitura SDCD! Id=%0x\n", dwIdThreads[0]);
	else {
		printf("Erro na criacao da thread leitura SDCD! N = %d Erro = %d\n", i, errno);
		exit(0);
	}

	hThreads[1] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)tarefa_leitura_do_PIMS,
		(LPVOID)1,
		0,
		(CAST_LPDWORD)&dwIdThreads[1]);
	if (hThreads[1])
		printf("Thread Leitura PIMS! Id=%0x\n", dwIdThreads[1]);
	else {
		printf("Erro na criacao da thread Leitura PIMS! N = %d Erro = %d\n", i, errno);
		exit(0);
	}

	hThreads[2] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)tarefa_captura_dados_processo,
		(LPVOID)2,
		0,
		(CAST_LPDWORD)&dwIdThreads[2]);
	if (hThreads[2])
		printf("Thread de captura dados do processo! Id=%0x\n", dwIdThreads[2]);
	else {
		printf("Erro na criacao da thread de captura dados do processo! N = %d Erro = %d\n", i, errno);
		exit(0);
	}

	hThreads[3] = (HANDLE)_beginthreadex(
		NULL,
		0,
		(CAST_FUNCTION)tarefa_captura_alarmes,
		(LPVOID)3,
		0,
		(CAST_LPDWORD)&dwIdThreads[3]);
	if (hThreads[3])
		printf("Thread captura de alarmes! Id=%0x\n", dwIdThreads[3]);
	else {
		printf("Erro na criacao da thread captura de alarmes! N = %d Erro = %d\n", i, errno);
		exit(0);
	}

	hfile = CreateFile(L"Dados.txt", // nome do arquivo
		GENERIC_READ | GENERIC_WRITE, // leitura e escrita
		FILE_SHARE_READ | FILE_SHARE_WRITE, // compartilhado para leitura e escrita
		NULL,		// atributos de segurança 
		CREATE_NEW,// cria novo arquivo 
		FILE_ATTRIBUTE_NORMAL,
		NULL);		// Template para atributos e flags
	SetEvent(harquivo);
	do {
		nTecla = _getch();
		switch (nTecla) {
		case s:
			if (SetEvent(sEvent) == NULL) {
				printf("Erro ao sinalizar o evento s\n");
			}
			dwStatus = WaitForSingleObject(Print, INFINITE);
			if (dwStatus == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObject: %d\n", GetLastError());
			}
			SetConsoleTextAttribute(hOut, 11);
			printf("Evento sEvent sinalizado: Bloqueando/desbloqueando a thread leitura do SDCD\n");
			SetConsoleTextAttribute(hOut, WHITE);
			dwStatus = ReleaseSemaphore(Print, 1, NULL);
			if (dwStatus == NULL) {
				printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
			}
			break;

		case p:
			if (SetEvent(pEvent) == NULL) {
				printf("Erro ao sinalizar o evento p\n");
			}
			dwStatus = WaitForSingleObject(Print, INFINITE);
			if (dwStatus == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObject: %d\n", GetLastError());
			}
			SetConsoleTextAttribute(hOut, 11);
			printf("Evento pEvent sinalizado: Bloqueando/desbloqueando a thread leitura PIMS\n");
			SetConsoleTextAttribute(hOut, WHITE);
			dwStatus = ReleaseSemaphore(Print, 1, NULL);
			if (dwStatus == NULL) {
				printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
			}
			break;

		case d:
			if (SetEvent(dEvent) == NULL) {
				printf("Erro ao sinalizar o evento d\n");
			}
			dwStatus = WaitForSingleObject(Print, INFINITE);
			if (dwStatus == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObject: %d\n", GetLastError());
			}
			SetConsoleTextAttribute(hOut, 11);
			printf("Evento dEvent sinalizado: Bloqueando/desbloqueando a thread de captura de dados do processo\n");
			SetConsoleTextAttribute(hOut, WHITE);
			dwStatus = ReleaseSemaphore(Print, 1, NULL);
			if (dwStatus == NULL) {
				printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
			}
			break;

		case a:
			if (SetEvent(aEvent) == NULL) {
				printf("Erro ao sinalizar o evento a\n");
			}
			dwStatus = WaitForSingleObject(Print, INFINITE);
			if (dwStatus == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObject: %d\n", GetLastError());
			}
			SetConsoleTextAttribute(hOut, 11);
			printf("Evento aEvent sinalizado: Bloqueando/desbloqueando a thread tarefa de captura de alarmes\n");
			SetConsoleTextAttribute(hOut, WHITE);
			dwStatus = ReleaseSemaphore(Print, 1, NULL);
			if (dwStatus == NULL) {
				printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
			}
			break;

		case o:
			if (SetEvent(oEvent) == NULL) {
				printf("Erro ao sinalizar o evento o\n");
			}
			dwStatus = WaitForSingleObject(Print, INFINITE);
			if (dwStatus == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObject: %d\n", GetLastError());
			}
			SetConsoleTextAttribute(hOut, 11);
			printf("Evento oEvent sinalizado: Bloqueando/desbloqueando a thread de exibicao de dados do processo\n");
			SetConsoleTextAttribute(hOut, WHITE);
			dwStatus = ReleaseSemaphore(Print, 1, NULL);
			if (dwStatus == NULL) {
				printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
			}
			break;

		case c:
			if (SetEvent(cEvent) == NULL) {
				printf("Erro ao sinalizar o evento c\n");
			}
			dwStatus = WaitForSingleObject(Print, INFINITE);
			if (dwStatus == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObject: %d\n", GetLastError());
			}
			SetConsoleTextAttribute(hOut, 11);
			printf("Evento cEvent sinalizado: Indicando a thread de exibicao de dados exibiçao de dados do alarme\n");
			SetConsoleTextAttribute(hOut, WHITE);
			dwStatus = ReleaseSemaphore(Print, 1, NULL);
			if (dwStatus == NULL) {
				printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
			}
			break;

		case ESC:
			if (SetEvent(escEvent) == NULL) {
				printf("Erro ao sinalizar o evento ESC\n");
			}
			break;
		}
	} while (nTecla != ESC);


	// Aguarda término das threads
	dwRet = WaitForMultipleObjects(4, hThreads, TRUE, INFINITE);
	if (dwRet == WAIT_FAILED) {
		printf("Falha na funcao WaitForMultipleObjects:%d\n", GetLastError());
	}

	// Fecha todos os handles das threads
	for (int j = 0; j < 4; j++)
		CloseHandle(hThreads[j]);

	// Dando Free no buffer
	for (int t = 0; t < MAX_MENSAGENS; t++) {
		free(buffer[t]);
	}

	// Fecha os handles dos objetos de sincronização
	CloseHandle(sEvent);
	CloseHandle(cEvent);
	CloseHandle(dEvent);
	CloseHandle(pEvent);
	CloseHandle(aEvent);
	CloseHandle(oEvent);
	CloseHandle(escEvent);
	CloseHandle(mEvent);
	
	CloseHandle(hMailslot);
	CloseHandle(hMailslot1);

	CloseHandle(hmutex);
	CloseHandle(hlivre);
	CloseHandle(hocupado);
	
	CloseHandle(Print);
	CloseHandle(hOut);

	CloseHandle(htimer);
	CloseHandle(htimercritico);
	CloseHandle(htimer_nao_critico);
	CloseHandle(hfile);
	DeleteFile(L"Dados.txt");

	return EXIT_SUCCESS;
}

void WINAPI tarefa_leitura_do_SDCD(LPVOID id) {
	HANDLE Events[3] = { sEvent, escEvent, hlivre };
	HANDLE Events2[2] = { sEvent, escEvent };
	HANDLE Events3[2] = { escEvent, htimer };
	DWORD ret;
	DWORD status;
	LARGE_INTEGER Tempo_de_espera;
	int nTipoEvento;
	int NSEQ = 1;



	do {

		if (NSEQ == 1000000) {
			NSEQ = 1;
		}

		Tempo_de_espera.QuadPart = -(500 * 10000);
		//thread se bloqueia esperando o timer
		SetWaitableTimer(htimer, &Tempo_de_espera, 0, NULL, NULL, FALSE); // Inicia o temporizador da thread
		ret = WaitForMultipleObjects(2, Events3, FALSE, INFINITE);
		if (ret == WAIT_FAILED) {
			printf("Falha na funcao WaitForMultipleObjects %d:\n", GetLastError());
		}
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			break;
		}
		else {
			//continua no loop quando o timer sinalizar
		}

		ret = WaitForMultipleObjects(3, Events, FALSE, INFINITE);
		if (ret == WAIT_FAILED) {
			printf("Falha na funcao WaitForMultipleObjects %d:\n", GetLastError());
		}
		nTipoEvento = ret - WAIT_OBJECT_0;


		if (nTipoEvento == 0) {
			printf("Thread de leitura do SDCD bloqueada\n");
			ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
			if (ret == WAIT_FAILED) {
				printf("Falha na funcao WaitForMultipleObjects %d:\n", GetLastError());
			}
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 1) {
				break;
			}
			else {
				printf("Thread de leitura do SDCD desbloqueada\n");
			}
		}
		else if (nTipoEvento == 1) {
			break;
		}
		else if (nTipoEvento == 2) {
			status = WaitForSingleObject(hmutex, INFINITE);
			if (status == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
			}
			while (strncmp(buffer[P_livre], "HORA", 4) == 0 || strncmp(buffer[P_livre], "NSEQ", 4) == 0) {
				P_livre = (P_livre + 1) % MAX_MENSAGENS;
			}
			strcpy(buffer[P_livre], Gerar_Msg_SDCD(NSEQ));
			P_livre = (P_livre + 1) % MAX_MENSAGENS;
			status = ReleaseSemaphore(hmutex, 1, NULL);
			if (status == NULL) {
				printf("Falha na funcao ReleaseSemaphore2: %d\n", GetLastError());
			}
			status = ReleaseSemaphore(hocupado, 1, NULL);
			if (status == NULL) {
				printf("Falha na funcao ReleaseSemaphore1: %d\n", GetLastError());
			}
			NSEQ++;
		}
	} while (1);    // Esc foi escolhido
	printf("Thread de leitura do SDCD (%d) terminando...\n", id);

	_endthreadex(0);
	return;
}

void WINAPI tarefa_leitura_do_PIMS(LPVOID id) {
	HANDLE Events[3] = { pEvent, escEvent, hlivre };
	HANDLE Events2[2] = { pEvent, escEvent };
	HANDLE Events3[2] = { htimercritico, htimer_nao_critico };
	DWORD ret;
	DWORD status;
	int nTipoEvento;
	int NSEQ = 1;



	//abrindo o mailslot
	DWORD dwBytesEnviados;
	
	WaitForSingleObject(mEvent, INFINITE);

	hMailslot = CreateFile(
		L"\\\\.\\mailslot\\Debug\\CaixaPostal",
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	
	char* alarme = (char*)malloc(sizeof(char) * 35);
	char dado[36];
	LARGE_INTEGER Tempo_de_espera, Tempo_de_espera2;
	Tempo_de_espera.QuadPart = -(gera_numero_aleatorio(1000, 5000) * 10000);
	Tempo_de_espera2.QuadPart = -(gera_numero_aleatorio(3000, 8000) * 10000);
	SetWaitableTimer(htimer_nao_critico, &Tempo_de_espera, 0, NULL, NULL, FALSE);
	SetWaitableTimer(htimercritico, &Tempo_de_espera2, 0, NULL, NULL, FALSE);
	

	do {

		if (NSEQ == 1000000) {
			NSEQ = 1;
		}

		ret = WaitForMultipleObjects(2, Events3, FALSE, INFINITE);
		if (ret == WAIT_FAILED) {
			printf("Falha na funcao WaitForMultipleObjects %d:\n", GetLastError());
		}
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			alarme = Gerar_Msg_PIMS_Critico(NSEQ);
			for (int i = 0; i < strlen(alarme); i++) {
				dado[i] = alarme[i];
			}
			for (int i = strlen(alarme); i < 36; i++) {
				dado[i] = '\0';
			}
			status = WriteFile(hMailslot, &dado, sizeof(dado), &dwBytesEnviados, NULL);
			NSEQ++;
			Tempo_de_espera2.QuadPart = -(gera_numero_aleatorio(3, 8) * 1000 * 10000);
			SetWaitableTimer(htimercritico, &Tempo_de_espera2, 0, NULL, NULL, FALSE);
		}
		else {
			ret = WaitForMultipleObjects(3, Events, FALSE, INFINITE);
			if (ret == WAIT_FAILED) {
				printf("Falha na funcao WaitForMultipleObjects %d:\n", GetLastError());
			}
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				printf("Thread de leitura do PIMS bloqueada\n");
				ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
				if (ret == WAIT_FAILED) {
					printf("Falha na funcao WaitForMultipleObjects %d:\n", GetLastError());
				}
				nTipoEvento = ret - WAIT_OBJECT_0;
				if (nTipoEvento == 1) {
					break;
				}
				else {
					printf("Thread de leitura do PIMS desbloqueada\n");
				}
			}
			else if (nTipoEvento == 1) {
				break;
			}
			else if (nTipoEvento == 2) {
				status = WaitForSingleObject(hmutex, INFINITE);
				if (status == WAIT_FAILED) {
					printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
				}
				while (strncmp(buffer[P_livre], "HORA", 4) == 0 || strncmp(buffer[P_livre], "NSEQ", 4) == 0) {
					P_livre = (P_livre + 1) % MAX_MENSAGENS;
				}
				strcpy(buffer[P_livre], Gerar_Msg_PIMS(NSEQ));
				P_livre = (P_livre + 1) % MAX_MENSAGENS;
				status = ReleaseSemaphore(hocupado, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
				}
				status = ReleaseSemaphore(hmutex, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
				}
				NSEQ++;
				Tempo_de_espera.QuadPart = -(gera_numero_aleatorio(1, 5) * 1000 * 10000);
				SetWaitableTimer(htimer_nao_critico, &Tempo_de_espera, 0, NULL, NULL, FALSE);
			}
		}
	} while (1);    // Esc foi escolhido
	printf("Thread de leitura do PIMS (%d) terminando...\n", id);

	_endthreadex(0);
	return;
}
void WINAPI tarefa_captura_dados_processo(LPVOID id) {
	HANDLE Events[3] = { dEvent, escEvent, hocupado };
	HANDLE Events2[2] = { dEvent, escEvent };
	DWORD ret;
	DWORD status;
	DWORD dwBytesWritten, dwPos;
	LONG lFilePosLow;
	int nTipoEvento;
	char dado[53];
	int iInput = 1;
	
	do {

		ret = WaitForMultipleObjects(3, Events, FALSE, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			printf("thread de captura de dados do processo bloqueada\n");
			ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				printf("thread de captura de dados do processo desbloqueada\n");
			}
		}
		else if (nTipoEvento == 1) {
			break;
		}
		else {
			status = WaitForSingleObject(hmutex, INFINITE);
			if (status == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
			}
			int t = strlen(buffer[P_ocupado_dado]);
			
			if (t>=46 && t <= 53) {

				
				for (int i = 0; i < strlen(buffer[P_ocupado_dado]); i++) {
					dado[i] = buffer[P_ocupado_dado][i];
				}
				for (int i = strlen(buffer[P_ocupado_dado]); i < 53; i++) {
					dado[i] = '\0';
				}
				status = WriteFile(hfile, &dado, sizeof(dado), &dwBytesWritten, NULL);
				status = ReleaseSemaphore(hocupado2, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore1: %d\n", GetLastError());
				}
				dwPos = iInput * sizeof(dado);
				SetFilePointer(hfile, dwPos, NULL, FILE_BEGIN);
				iInput++;
				if (iInput > 100) {
					SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
					iInput = 1;
				}
				free(buffer[P_ocupado_dado]);
				buffer[P_ocupado_dado] = (char*)malloc((53) * sizeof(char));
				P_ocupado_dado = (P_ocupado_dado + 1) % MAX_MENSAGENS; //proxima posiçao a ser lida
				status = ReleaseSemaphore(hlivre, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
				}
				status = ReleaseSemaphore(hmutex, 1, NULL);
				if (status == WAIT_FAILED) {
					printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
				}
			}
			else {
				P_ocupado_dado = (P_ocupado_dado + 1) % MAX_MENSAGENS;
				status = ReleaseSemaphore(hocupado, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
				}
				status = ReleaseSemaphore(hmutex, 1, NULL);
				if (status == WAIT_FAILED) {
					printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
				}
			}
		}
	} while (1);    // Esc foi escolhido
	FlushFileBuffers(hfile);
	printf("Thread de captura de dados do processo (%d) terminando...\n", id);
	_endthreadex(0);
	
	return;
}

void WINAPI tarefa_captura_alarmes(LPVOID id) {
	HANDLE Events[3] = { aEvent, escEvent, hocupado };
	HANDLE Events2[2] = { aEvent, escEvent };
	DWORD ret;
	DWORD status;
	int nTipoEvento;
	char dado[36];


	//abrindo o mailslot
	DWORD dwBytesEnviados;
	
	WaitForSingleObject(mEvent, INFINITE);

	hMailslot1 = CreateFile(
		L"\\\\.\\mailslot\\Debug\\CaixaPostal",
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	

	do {

		ret = WaitForMultipleObjects(3, Events, FALSE, INFINITE);
		nTipoEvento = ret - WAIT_OBJECT_0;
		if (nTipoEvento == 0) {
			printf("thread de captura de alarmes bloqueada\n");
			ret = WaitForMultipleObjects(2, Events2, FALSE, INFINITE);
			nTipoEvento = ret - WAIT_OBJECT_0;
			if (nTipoEvento == 0) {
				printf("thread captura de alarmes desbloqueada\n");
			}
			else {
				break;
			}
		}
		else if (nTipoEvento == 1) {
			break;
		}
		else {
			status = WaitForSingleObject(hmutex, INFINITE);
			if (status == WAIT_FAILED) {
				printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
			}
			int t = strlen(buffer[P_ocupado_alarme]);
			if (t <= 35 && (strstr(buffer[P_ocupado_alarme], "ç") == NULL)) {
				for (int i = 0; i < strlen(buffer[P_ocupado_alarme]); i++) {
					dado[i] = buffer[P_ocupado_alarme][i];
				}
				for (int i = strlen(buffer[P_ocupado_alarme]); i < 36; i++) {
					dado[i] = '\0';
				}
				status = WriteFile(hMailslot1, &dado, sizeof(dado), &dwBytesEnviados, NULL);

				free(buffer[P_ocupado_alarme]);
				buffer[P_ocupado_alarme] = (char*)malloc((53) * sizeof(char));
				P_ocupado_alarme = (P_ocupado_alarme + 1) % MAX_MENSAGENS;
				status = ReleaseSemaphore(hlivre, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
				}
				status = ReleaseSemaphore(hmutex, 1, NULL);
				if (status == WAIT_FAILED) {
					printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
				}
			}
			else {
				P_ocupado_alarme = (P_ocupado_alarme + 1) % MAX_MENSAGENS;
				status = ReleaseSemaphore(hocupado, 1, NULL);
				if (status == NULL) {
					printf("Falha na funcao ReleaseSemaphore: %d\n", GetLastError());
				}
				status = ReleaseSemaphore(hmutex, 1, NULL);
				if (status == WAIT_FAILED) {
					printf("Falha na funcao WaitForSingleObjects %d:", GetLastError());
				}
			}
		}
	} while (1);    // Esc foi escolhido
	printf("Thread de captura de alarmes (%d) terminando...\n", id);


	_endthreadex(0);
	return;
}

char* Gerar_Msg_SDCD(int NSEQ) {
	char* dado = (char*)malloc((53) * sizeof(char));

	strcpy(dado, _NSEQ(NSEQ));
	strcat(dado, "|");
	strcat(dado, "1"); // TIPO SEMPRE IGUAL A 1
	strcat(dado, "|");
	strcat(dado, Tag());
	strcat(dado, "|");
	strcat(dado, Valor());
	strcat(dado, "|");
	strcat(dado, Unidade());
	strcat(dado, "|");
	strcat(dado, Modo());
	strcat(dado, "|");
	strcat(dado, Hora());

	return dado;
}

char* Tag() {
	srand(time(NULL));

	char caracteresvalidos[35] = "ABCDEFGHJKLMNOPQRSTUVWYZ0123456789";
	char* retorno = (char*)malloc((11) * sizeof(char));
	for (int i = 0; i < 10; i++) {
		retorno[i] = caracteresvalidos[rand() % 34];
		retorno[i + 1] = 0x0;
		if (i == 3 || i == 6) {
			retorno[i] = '-';
			retorno[i + 1] = 0x0;
		}
	}
	return retorno;
}

char* Valor() {
	srand(time(NULL));

	char caracteresvalidos[11] = "0123456789";
	char* valor = (char*)malloc((9) * sizeof(char));
	for (int i = 0; i < 7; i++) {
		valor[i] = caracteresvalidos[rand() % 10];
		valor[i + 1] = 0x0;
		if (i == 4) {
			valor[i] = '.';
			valor[i + 1] = 0x0;
		}
	}
	return valor;
}

char* Unidade() {
	srand(time(NULL));
	char* caracteresvalidos[6];
	caracteresvalidos[0] = (char*)malloc((9) * sizeof(char));
	caracteresvalidos[1] = (char*)malloc((9) * sizeof(char));
	caracteresvalidos[2] = (char*)malloc((9) * sizeof(char));
	caracteresvalidos[3] = (char*)malloc((9) * sizeof(char));
	caracteresvalidos[4] = (char*)malloc((9) * sizeof(char));
	caracteresvalidos[5] = (char*)malloc((9) * sizeof(char));

	strcpy_s(caracteresvalidos[0], 8, "K      ");
	strcpy_s(caracteresvalidos[1], 8, "Kgf    ");
	strcpy_s(caracteresvalidos[2], 8, "Kg/m^2 ");
	strcpy_s(caracteresvalidos[3], 8, "Kg/m^3 ");
	strcpy_s(caracteresvalidos[4], 8, "N.m    ");
	strcpy_s(caracteresvalidos[5], 8, "Pa     ");
	int aux = rand() % 6;
	char* unidade = (char*)malloc((9) * sizeof(char));
	unidade = caracteresvalidos[aux];
	for (int i = 0; i < 6; i++) {
		if (i != aux) {
			free(caracteresvalidos[i]);
		}
	}
	return unidade;
}

char* Modo() {
	srand(time(NULL));
	char* modo = (char*)malloc((2) * sizeof(char));

	int r = 1 + rand() % 2;
	if (r == 1) {
		strcpy(modo, "M");
	}
	else {
		strcpy(modo, "A");
	}

	return modo;
}

char* Hora() {
	char* retorno = (char*)malloc((13) * sizeof(char));

	SYSTEMTIME sys;
	GetLocalTime(&sys);
	char* hora = (char*)malloc((3) * sizeof(char));
	char* minutos = (char*)malloc((3) * sizeof(char));
	char* segundos = (char*)malloc((3) * sizeof(char));
	char* milisegundos = (char*)malloc((4) * sizeof(char));
	_itoa(sys.wHour, hora, 10);
	_itoa(sys.wMinute, minutos, 10);
	_itoa(sys.wSecond, segundos, 10);
	_itoa(sys.wMilliseconds, milisegundos, 10);

	strcpy(retorno, hora);
	strcat(retorno, ":");
	strcat(retorno, minutos);
	strcat(retorno, ":");
	strcat(retorno, segundos);
	strcat(retorno, ":");
	strcat(retorno, milisegundos);

	free(hora);
	free(minutos);
	free(segundos);
	free(milisegundos);

	return retorno;
}

char* _NSEQ(int nseq) {
	char* resultado = (char*)malloc(7 * sizeof(char));
	char* zeros = (char*)malloc(7 * sizeof(char));
	_itoa(nseq, resultado, 10);
	if (nseq < 10) {
		strcpy(zeros, "00000");
		strcat(zeros, resultado);

	}
	else if (nseq >= 10 && nseq < 100) {

		strcpy(zeros, "0000");
		strcat(zeros, resultado);
	}
	else if (nseq >= 100 && nseq < 1000) {

		strcpy(zeros, "000");
		strcat(zeros, resultado);
	}
	else if (nseq >= 1000 && nseq < 10000) {

		strcpy(zeros, "00");
		strcat(zeros, resultado);
	}
	else if (nseq >= 10000 && nseq < 100000) {

		strcpy(zeros, "0");
		strcat(zeros, resultado);
	}
	else {
		strcpy(zeros, resultado);
	}
	free(resultado);
	return zeros;
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

char* Gerar_Msg_PIMS(int NSEQ) {
	char* alarme = (char*)malloc(32 * sizeof(char));

	strcpy(alarme, _NSEQ(NSEQ));
	strcat(alarme, "|");
	strcat(alarme, "2");
	strcat(alarme, "|");
	strcat(alarme, ID());
	strcat(alarme, "|");
	strcat(alarme, Grau());
	strcat(alarme, "|");
	strcat(alarme, PREV());
	strcat(alarme, "|");
	strcat(alarme, Hora());

	return alarme;
}

char* Gerar_Msg_PIMS_Critico(int NSEQ) {
	char* alarme = (char*)malloc(32 * sizeof(char));

	strcpy(alarme, _NSEQ(NSEQ));
	strcat(alarme, "|");
	strcat(alarme, "9");
	strcat(alarme, "|");
	strcat(alarme, ID());
	strcat(alarme, "|");
	strcat(alarme, Grau());
	strcat(alarme, "|");
	strcat(alarme, PREV());
	strcat(alarme, "|");
	strcat(alarme, Hora());

	return alarme;
}

/*char* Tipo2() {
	srand(time(NULL));
	char* tipo2 = (char*)malloc(sizeof(char) * 2);

	int r = 1 + rand() % 2;
	if (r == 1) {
		strcpy(tipo2, "2");
	}
	else {
		strcpy(tipo2, "9");
	}

	return tipo2;
}*/

char* ID() {
	srand(time(NULL));
	char* id = (char*)malloc(sizeof(char) * 5);
	char* zeros = (char*)malloc(5 * sizeof(char));
	int r = 1 + rand() % 9999;
	_itoa(r, id, 10);

	if (r < 10) {
		strcpy(zeros, "000");
		strcat(zeros, id);

	}
	else if (r >= 10 && r < 100) {

		strcpy(zeros, "00");
		strcat(zeros, id);
	}
	else if (r >= 100 && r < 1000) {

		strcpy(zeros, "0");
		strcat(zeros, id);
	}
	else {
		strcpy(zeros, id);
	}

	free(id);
	return zeros;
}

char* Grau() {
	srand(time(NULL));
	char* grau = (char*)malloc(sizeof(char) * 3);
	char* zeros = (char*)malloc(3 * sizeof(char));
	int r = 1 + rand() % 99;

	_itoa(r, grau, 10);

	if (r < 10) {
		strcpy(zeros, "0");
		strcat(zeros, grau);

	}
	else {
		strcpy(zeros, grau);
	}

	free(grau);
	return zeros;
}

char* PREV() {
	srand(time(NULL));
	char* prev = (char*)malloc(sizeof(char) * 6);
	char* zeros = (char*)malloc(6 * sizeof(char));
	int r = 1 + rand() % 14440;
	_itoa(r, prev, 10);

	if (r < 10) {
		strcpy(zeros, "0000");
		strcat(zeros, prev);

	}
	else if (r >= 10 && r < 100) {

		strcpy(zeros, "000");
		strcat(zeros, prev);
	}
	else if (r >= 100 && r < 1000) {

		strcpy(zeros, "00");
		strcat(zeros, prev);
	}
	else if (r >= 1000 && r < 10000) {
		strcpy(zeros, "0");
		strcat(zeros, prev);
	}
	else {
		strcpy(zeros, prev);
	}

	free(prev);
	return zeros;

}

int gera_numero_aleatorio(int valor_min, int valor_max)
{	
	time_t t;
	srand(time(&t));
	return (rand() % valor_max + valor_min);
}

