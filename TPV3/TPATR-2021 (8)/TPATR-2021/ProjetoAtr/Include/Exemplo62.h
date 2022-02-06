//
//	Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Capítulo 6 - Exemplo 2 e 3 - DumpFunction
//
//	Versão: 1.1	28/02/1999
//

#define ERRO_ARQUIVO_INEXISTENTE	1

typedef struct {
	char FileName[50];
	DWORD InitialPosition; 
	DWORD Length;
} FileReadDesc;

typedef struct {
	DWORD Erro;		// Não foi possível ler do arquivo
	BOOL EndOfFile;		// Fim do arquivo atingido
	DWORD BytesRead;	// Number of Bytes read from file
} DumpFileAck; 
