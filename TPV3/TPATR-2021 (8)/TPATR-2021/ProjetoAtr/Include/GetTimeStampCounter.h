//
//	Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	Capítulo 4 - Exemplo 6 - GetTimeStampCounter/GetTimeInterval
//
//	Versão: 1.1	20/02/1999
//


typedef struct {
	unsigned long low;
	unsigned long high;
} time_stamp;


#define CPUID  __asm __emit 0fh  __asm  __emit 0a2h
#define RDTSC  __asm __emit 0fh  __asm  __emit 031h


VOID GetTimeStampCounter(time_stamp &time)
{

	__asm {
			pushad
			CPUID		// garante serialização de instruções no Pentium II,...
			RDTSC		// Le Pentium Cycle counter
			mov ebx, time		// ebx é o endereço de time
			mov [ebx].low, eax	// contador low
			mov [ebx].high,edx  // contador high
			popad
	} // asm
} // GetTimeStampCounter



DOUBLE GetTimeInterval(time_stamp init_timer, time_stamp last_timer, unsigned MHz_frequency)
	// Retorna intervalo de tempo em microsegundos
	// MHz_frequency é a frequencia do clock em MHz
{
	DOUBLE	interval;		// intervalo de tempo em micro segundos	
	__int64 cycles1, cycles2;	// número de ciclos de clock do Pentium
 
	cycles1 = ((unsigned __int64) init_timer.high <<32) | init_timer.low;
	cycles2 = ((unsigned __int64) last_timer.high <<32) | last_timer.low;
	interval = (double) (cycles2 - cycles1) / MHz_frequency;
	//printf("Intervalo = %f milisegundos\n", interval/1000);

	return interval;
} // GetTimeIntreval


