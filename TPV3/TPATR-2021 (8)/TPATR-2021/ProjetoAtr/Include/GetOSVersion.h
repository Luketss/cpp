//
//	Programação Multithreaded em ambiente Windows NT© - uma visão de  Automação
// 
//	Autores: Constantino Seixas Filho/ Marcelo Szuster
//
//	GetOSVersion
//
//	Determina versão do sistema operacional e número de processadores do computador
//
//	Versão: 1.1	20/02/1999
//          1.2 16/10/2008 Adaptacao para Visual C++ Express Edition 2008
//

#pragma warning(disable: 4996) //Acrescentado em 16/10/2008
                               //VC++2008 Express ficava repetindo o warning
                               //mesmo com #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#include <string.h>

BOOL GetOSVersion(char *strVersion, int *Processadores) 
{
	SYSTEM_INFO sysinfo;
	OSVERSIONINFO Version;
	BOOL ret;
	BOOL bIsNT = FALSE;
	DWORD Maior, Menor;

	GetSystemInfo(&sysinfo); // Le informação do sistema
	*Processadores = sysinfo.dwNumberOfProcessors;

	Version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	ret = GetVersionEx(&Version);
	if (ret) {
		Maior = Version.dwMajorVersion;
		Menor = Version.dwMinorVersion;
		if (Version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			 sprintf(strVersion, "WINDOWS_NT %d.%d", Maior, Menor);
			 bIsNT = TRUE;
		}
		else if (Version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			 sprintf(strVersion, "WINDOWS_95 %d.%d", Maior, Menor);
		else if (Version.dwPlatformId == VER_PLATFORM_WIN32s)
			 sprintf(strVersion, "WINDOWS_3.1 %d.%d", Maior, Menor);
		}  // if
	else strcpy(strVersion, "Erro de leitura de versão");

	return bIsNT;

} // GetOSVersion

