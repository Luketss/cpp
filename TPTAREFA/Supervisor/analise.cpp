#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<process.h>
#include<locale.h>

#include"message.h"

void clear_screen() {

    DWORD count;
    DWORD cell_count;

    COORD home_coords = { 0, 0 };

    HANDLE std_out;
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    std_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (std_out == INVALID_HANDLE_VALUE) return;

    /* recupera numero de celular em buffer atual */

    if (!GetConsoleScreenBufferInfo(std_out, &csbi)) return;
    cell_count = csbi.dwSize.X * csbi.dwSize.Y;

    /* preenche o buffer inteiro com espacos */

    if (!FillConsoleOutputCharacter(
        std_out,
        (TCHAR)' ',
        cell_count,
        home_coords,
        &count
    )) return;

    SetConsoleCursorPosition(std_out, home_coords);

}

int main() {

    // Habilitando acentuacão gráfica
    setlocale(LC_ALL, "Portuguese");

    int event_id = -1;
    int buffer_2_size = 100;
    DWORD ret;

    /* abre eventos para pausa e finalizacao */

    HANDLE end_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, TEXT("end_event"));
    HANDLE toggle_event = OpenEvent(EVENT_ALL_ACCESS, TRUE, TEXT("analise_supervisor_toggle_event"));

    /* abre semaforos para sincronizacao da lista 2 */

    HANDLE sem_livre = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, TEXT("sem_livre"));
    HANDLE sem_ocupado = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, TEXT("sem_ocupado"));
    HANDLE sem_rw = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, TEXT("sem_rw"));

    /* abre segunda lista mapeada em memoria */

    HANDLE mapped_memory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"lista_2");
 


    Message* second_buffer_local = (Message*)MapViewOfFile(
        mapped_memory,
        FILE_MAP_WRITE,
        0,
        0,
        sizeof(Message) * buffer_2_size);

    /* abre variaveis de sincronizacao de segunda lista */

    HANDLE mapped_p_ocupado = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"p_ocupado");
    HANDLE mapped_p_livre = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"p_livre");

    int second_p_ocupado_offset = sizeof(Message) * buffer_2_size + 100;

    int p_ocupado = (int)MapViewOfFile(
        mapped_p_ocupado,
        FILE_MAP_WRITE,
        0,
        second_p_ocupado_offset, // desclocamento para nao sobrepor a segunda lista
        sizeof(int));

    int second_p_livre_offset = sizeof(Message) * buffer_2_size + 200;

    int p_livre = (int)MapViewOfFile(
        mapped_p_livre,
        FILE_MAP_WRITE,
        0,
        second_p_livre_offset, // desclocamento para nao sobrepor a segunda lista nem p_ocupado
        sizeof(int));

    printf("\nProcesso analise de supervisor disparado\n");

    HANDLE mslot;
    mslot = CreateMailslot(
        L"\\\\.\\mailslot\\analise_mailslot",
        0,
        0, // se nao houver mensagem  prossegue com execucao
        NULL);

    HANDLE events[2] = { toggle_event, end_event };
    HANDLE occupied_sem_events[3] = { sem_ocupado, end_event, toggle_event };

    int data_index;
    Message recovered_data;

    int signal;

    do {
        signal = 0;

        bool status = ReadFile(mslot, &signal, sizeof(int), NULL, NULL);

        if (signal) clear_screen();

        ret = WaitForSingleObject(sem_ocupado, 100);

        /* caso onde nao ha nada para consumir na lista */

        if (ret == WAIT_TIMEOUT) {

            /* aguarda os seguintes eventos */
            /* 1 - surge um dado para se consumir na lista */
            /* 2 - ocorre comando de termino */
            /* 3 - ocorre sinal de bloqueio */

            ret = WaitForMultipleObjects(3, occupied_sem_events, FALSE, INFINITE);
            ret = ret - WAIT_OBJECT_0;
            /* 2 - caso onde se recebe sinal de termino */
            if (ret == 1) { break; }
            /* 3 - caso onde se recebe sinal de bloqueio */
            if (ret == 2) {

                printf("\n...................................................................................."
                    "\nProcesso Exibição de dados *BLOQUEADO*. Para desbloquear, tecle <a>."
                    "\n....................................................................................\n");

                ret = WaitForMultipleObjects(2, events, FALSE, INFINITE);

                ret = ret - WAIT_OBJECT_0;

                if (ret == 0) {
                    printf("\n...................................................................................."
                        "\nProcesso Exibição de dados *DESBLOQUEADO*."
                        "\n....................................................................................\n");
                }

                /* ao se desbloquear volta para inicio de laco para verificar se ha dados a serem consumidos */
                continue;
            }
            /* 1 - prossegue normalmente com o consumo dos dados */
        }

        WaitForSingleObject(sem_rw, INFINITE);

        data_index = p_ocupado % buffer_2_size;

        recovered_data = second_buffer_local[data_index];
        show_message(recovered_data.supervisor);
        p_ocupado++;

        ReleaseSemaphore(sem_rw, 1, NULL);
        ReleaseSemaphore(sem_livre, 1, NULL);

        /* verifica se recebeu sinal de encerramento ou bloqueio */

        ret = WaitForMultipleObjects(2, events, FALSE, 100);

        if (ret == WAIT_TIMEOUT) {  // o codigo desse ponto para frente so eh executado se houver sinalizacao de bloqueio ou termino
            continue;
        }

        event_id = ret - WAIT_OBJECT_0;

        if (event_id == 0) {
            printf("\n...................................................................................."
                "\nProcesso Exibição de dados *BLOQUEADO*. Para desbloquear, tecle <a>."
                "\n....................................................................................\n");

            ret = WaitForMultipleObjects(2, events, FALSE, INFINITE);

            event_id = ret - WAIT_OBJECT_0;

            if (event_id == 0) {
                printf("\n...................................................................................."
                    "\nProcesso Exibição de dados *DESBLOQUEADO*."
                    "\n....................................................................................\n");
            }

        }

    } while (event_id != 1);

    /* o mapeamento de memoria fecha com o fim do programa */

};