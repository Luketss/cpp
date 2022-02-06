//
//	Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Função ShowDump: Dump de arquivos
//
//	Versão: 1.1	20/01/1999
//          1.2 28/08/2008 Adaptacao para Visual C++ Express Edition 2008
//

void ShowDump(char *lpBuffer, int iLength)
{	int i=0;
	int iColuna;
	int iLinha;
	int iLinhas;
	int iResto;

	iLinhas = iLength / 16;
	iResto = iLength %  16;
	
	// Imprime cabeçalho
//	printf("\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0 1 2 3 4 5 6 7 8 9 A B C D E F\n");
//	printf("================================================================================\n");
	printf("\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  0123456789ABCDEF\n");
	printf("=================================================================\n");

	for (iLinha=0; iLinha<iLinhas; iLinha++)
	{	for (iColuna=0; iColuna<16; ++iColuna)
			printf("%02x ", lpBuffer[i++] & 0xFF);
		i-= 16;
		printf(" ");
		for (iColuna=0; iColuna<16; ++iColuna)
			// 24/10/2008 No VC++ 2008 a função ISPRINT foi codificada com um
			// "assert (unsigned)(c+1) <=256" que provoca uma janela de erro na
			// execução desta rotina. Isto se deve ao fato de que o tipo CHAR é
			// tratado como um inteiro com sinal, e um CHAR correspondente a um
			// caracter acima de 0x7F (ex. 0x80) é então negativo e promovido
			// a um INT negativo na chamada a ISPRINT. A solução é forçar o tipo
			// a um UNSIGNED CHAR.
			//if (isprint(lpBuffer[i]))
			if (isprint((unsigned char) lpBuffer[i]))
//				printf("%c ", lpBuffer[i++]);
//			else { printf(". "); i++; }
				printf("%c", lpBuffer[i++]);
			else { printf("."); i++; }
		printf("\n\n");
	}  // for

	if (iResto > 0) {
		// printa última linha
		for (iColuna=0; iColuna<16; ++iColuna)
			if (iColuna < iResto) printf("%02x ", lpBuffer[i++] & 0xFF);
			else printf("   "); 
		i-= iResto;
		printf(" ");
		for (iColuna=0; iColuna<iResto; ++iColuna)
			if (isprint(lpBuffer[i]))
//				printf("%c ", lpBuffer[i++]);
//				else { printf(". "); i++; }
				printf("%c", lpBuffer[i++]);
				else { printf("."); i++; }
		printf("\n\n");
	}

} // show_dump

