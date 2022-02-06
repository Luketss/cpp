//
//	Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Capítulo 5 - Exemplo 4 - Mailslots
//
//	Versão: 1.1	13/03/1999
//

typedef enum {INVALID, GOOD, NOT_RELIABLE, ABORT} Quality;
// Abort está sendo usada como flag para abortar o servidor

typedef struct {
	unsigned Dia;	// dd de 1 a 31
	unsigned Mes;	// mm de 1 a 12
	unsigned Ano;	// yyyy de 0000 a 3000
} DataType;

typedef struct {
	unsigned Hora;	// hh de 00 a 24
	unsigned Min;	// minutos de 0 a 60
	unsigned Seg;	// segundos de 0 a 60
	unsigned Ms;	// milisegundos de 0 a 999
} TimeType;

typedef	struct {
	DataType Date;
	TimeType Time;
} TimeStamp;

typedef struct{
	TimeStamp TimeStp;
	Quality	Q;
	double Value;
	char Tag[12];
	char Description[80];
	char Status[12];
} AlarmMsg;
