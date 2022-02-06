//
//	Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Função bGetInt: Entrada de dados de valor Inteiro
//
//	Versão: 1.1	20/01/1999
//

#include <stdio.h>
#include <conio.h>		// _getch
#include <ctype.h>		// _isdigit

#define WIN32_LEAN_AND_MEAN
#ifndef _WINDOWS_
#include <windows.h>
#endif

#define	ESC		0x1B
#define CR		0x0D
#define BS		0x08

BOOL bGetInt(int *result)
{	int ch;
	int num = 0;
	int numdig = 0;
	
	do {
		ch=_getch();
		if (isdigit(ch)) {
			printf("%c", ch); // echoa
			num = num*10 + (ch - 0x30);
			numdig++;
		} // if
	} while ((ch != ESC) && (numdig<=6) && (ch != CR));
	if (ch != ESC) {
		*result = num; 
		return TRUE;}
	else return FALSE;
}	// bGetInt

