#include <iostream>
#include <cctype>
#include <random>
#include <iomanip>
#include <sstream>
#include <string>
#include <list>
#include <iomanip> 
#include <Windows.h>
#include <cstdlib>
#include <ctime>

using namespace std;

using std::string;

void LogMessage(string message)
{
    std::cout << message << std::endl;
}

//string ShapeStringResult(string arr)
//{
//	int arrSize = *(&arr + 1) - arr;
//	for(int i = 0; i)
//}


string RandomString(size_t length)
{
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}


string CreateTimestamp()
{
    auto current_time = std::time(nullptr);
    tm time_info{};
    const auto local_time_error = localtime_s(&time_info, &current_time);
    if (local_time_error != 0)
    {
        throw std::runtime_error("localtime_s() failed: " + std::to_string(local_time_error));
    }
    std::ostringstream output_stream;
    output_stream << std::put_time(&time_info, "%H:%M:%S");
    string timestamp(output_stream.str());
    return timestamp;
}

string GenerateSupervisorioMessage()
{
    const int spTipo = 5;
    int spNSEQ = 0;
    string spDate = CreateTimestamp();
	string type = RandomString(8);
	//string arrText[2] = {type, spDate};
	//return ShapeStringResult(arrText[2]);
	return type;
}

/*
string GeraMsg(int NSEQ, int tipo)
{
	stringstream ssMsg;
	stringstream ssNSEQ;

	if (tipo == 1)
	{
		stringstream ssValor;
		char cModo;

		// Campo = NSEQ
		// Completa tamanho do campo NSEQ
		ssNSEQ << std::setfill('0') << std::setw(6) << NSEQ;

		// Campo = VALOR
		// Gera numero aleatorio de 7 algarismos + 1 ponto = 8 caracteres
		for (int n = 0; n < 8; n++)
		{
			if (n == 5)
			{
				ssValor << '.';
			}
			else
			{
				ssValor << rand() % 10;
			}
		}

		// Campo = MODO
		// Define o modo
		if ((rand() % 2) == 0)
		{
			cModo = 'A';
		}
		else
		{
			cModo = 'M';
		}

		// Campo = TIMESTAMP
		// Pega o horario atual
		stringstream strTimestamp;
		SYSTEMTIME timestamp;
		GetLocalTime(&timestamp);
		strTimestamp << std::setfill('0') << std::setw(2) << timestamp.wHour << ':' << std::setfill('0') << std::setw(2) << timestamp.wMinute
			<< ':' << std::setfill('0') << std::setw(2) << timestamp.wSecond << '.' << std::setfill('0') << std::setw(3) << timestamp.wMilliseconds;

		// Monta mensagem de dados que será retornada para thread LeSDCD
		ssMsg << ssNSEQ.str() << '|' << tipo << '|' << gen_random(10, timestamp.wMilliseconds) << '|' << ssValor.str() << '|'
			<< gen_random(8, timestamp.wMilliseconds + timestamp.wSecond) << '|' << cModo << '|' << strTimestamp.str() << endl;

		return ssMsg.str();
	}// Fim tipo 1
	else if (tipo == 2 || tipo == 9)
	{
		stringstream ssIdAlarme;
		stringstream ssGrau;
		stringstream ssPrev;

		// Campo = NSEQ
		// Completa tamanho do campo NSEQ
		ssNSEQ << std::setfill('0') << std::setw(6) << NSEQ;

		// Campo = ID ALARME
		// Gera numero aleatorio de 4 algarismos
		//for (int n = 0; n < 4; n++)
		//{			
		//}
		ssIdAlarme << std::setfill('0') << std::setw(4) << rand() % 10000;

		// Campo = GRAU
		// Gera numero aleatorio de 2 algarismos
		//for (int n = 0; n < 2; n++)
		//{
		//}
		ssGrau << std::setfill('0') << std::setw(2) << rand() % 100;
		//cout << "ssGrau:" << ssGrau.str() << endl;

		// Campo = PREV
		// Define o modo
		ssPrev << std::setfill('0') << std::setw(5) << rand() % 14441;
		//cout << "ssPrev:" << ssPrev.str() << endl;

		// Pega o horario atual
		// Campo = TIMESTAMP
		stringstream strTimestamp;
		SYSTEMTIME timestamp;
		GetLocalTime(&timestamp);
		strTimestamp << std::setfill('0') << std::setw(2) << timestamp.wHour << ':' << std::setfill('0') << std::setw(2) << timestamp.wMinute
			<< ':' << std::setfill('0') << std::setw(2) << timestamp.wSecond;


		// Monta mensagem de alarme que será retornada para thread LePIMS
		ssMsg << ssNSEQ.str() << '|' << tipo << '|' << ssIdAlarme.str() << '|' << ssGrau.str() << '|' << ssPrev.str() << '|' << strTimestamp.str() << endl;

		return ssMsg.str();
	}// Fim tipo 2 e 9

	// Não deve chegar aqui
	cout << "Erro na função GeraMsg. Tipo da msg =" << tipo << endl;
	return 0;
}// ------------------------------- FIM GeraMsg ------------------------------- //


// ------------------------------- Função gen_random ------------------------------- //
// Função que gera string aleatoria com base no conjunto de caracteres determinado
// e de tamanho "len"
std::string gen_random(const int len, int multiplicador)
{
	std::string tmp_s;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ-"
		"abcdefghijklmnopqrstuvwxyz";

	// Torna o processo aleatorio usando o tempo atual * multiplicador (que é mseg ou seg+mseg)
	// como seed para o gerador.
	srand((unsigned)time(NULL) * multiplicador);

	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i)
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}// ------------------------------- FIM gen_random ------------------------------- //
*/