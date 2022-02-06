
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>						
#include <conio.h>




int main()
{
	BOOL status;
	STARTUPINFO si, si2, si3;
	PROCESS_INFORMATION Processo1, Processo2, Processo3;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if (!CreateProcess(
		L"..\\Debug\\Projeto1.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		L"..\\Debug",
		&si,
		&Processo1)
		)
	{
		printf("Criacao do processo 1 falhou (%d) \n", GetLastError());
	}

	ZeroMemory(&si2, sizeof(si2));
	si2.cb = sizeof(si2);

	if (!CreateProcess(
		L"..\\Debug\\Projeto2.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		L"..\\Debug",
		&si2,
		&Processo2)
		)
	{
		printf("Criacao do processo 2 falhou (%d) \n", GetLastError());
	}

	ZeroMemory(&si3, sizeof(si3));
	si3.cb = sizeof(si3);

	if (!CreateProcess(
		L"..\\Debug\\Projeto3.exe",
		NULL,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		L"..\\Debug",
		&si3,
		&Processo3)
		)
	{
		printf("Criacao do processo 3 falhou (%d) \n", GetLastError());
	}


	CloseHandle(Processo1.hProcess);
	CloseHandle(Processo1.hThread);

	CloseHandle(Processo2.hProcess);
	CloseHandle(Processo2.hThread);

	CloseHandle(Processo3.hProcess);
	CloseHandle(Processo3.hThread);

	return EXIT_SUCCESS;
}