#include "message.h"
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <ctime>
#include <iostream>


MessageSupervisor create_message(int nseq, double t_zona1, double t_zona2, double t_zona3,
    double volume, double pressao, Timestamp time, int tipo = 5) {
    MessageSupervisor message;
    message.tipo = tipo;
    message.nseq = nseq;
    message.t_zona1 = t_zona1;
    message.t_zona2 = t_zona2;
    message.t_zona3 = t_zona3;
    message.volume = volume;
    message.pressao = pressao;
    message.time = time;
    return message;

}

MessageSupervisor generate_message_sup(int& nseq) {

    double t_zona1;
    double t_zona2;
    double t_zona3;
    double volume;
    double pressao;
    SYSTEMTIME time;
    Timestamp timestamp;

    GetLocalTime(&time);

    timestamp.hour = time.wHour;
    timestamp.minute = time.wMinute;
    timestamp.second = time.wSecond;
    timestamp.millisecond = time.wMilliseconds;

    t_zona1 = (rand() % 10000 - 1);
    t_zona2 = (rand() % 10000 - 1);
    t_zona3 = (rand() % 10000 - 1);
    volume = (rand() % 10000 - 1);
    pressao = (rand() % 10000 - 1);



    nseq = (nseq + 1) % 9999;
    MessageSupervisor messagesup = create_message(nseq, t_zona1, t_zona2, t_zona3, volume, pressao, timestamp);

    return messagesup;

}

void show_message(MessageSupervisor msg) {
    printf("NSEQ: %i, TZ1: %.1f TZ2: %.1f TZ3: %.1f VOL: %.1f PRES: %.1f HORA: %i:%i:%i.%i\n",
        msg.nseq, msg.t_zona1, msg.t_zona2,
        msg.t_zona3, msg.volume, msg.pressao,
        msg.time.hour, msg.time.minute, msg.time.second, msg.time.millisecond);
}

void Gen(char* op) {
    srand(time(0));

    int len = 9;
    char letters[] = "abcdefghijklmnopqrstuvwxyz";
    char numbers[] = "1234567890";
    // 62 is the length of string 'valid'

    for (int i = 0; i < len; ++i)
    {
        if (i == 3)
        {
            op[i] = '-';
        }
        else if (i < 4)
        {
            op[i] = letters[rand() % 26];
        }
        else
        {
            op[i] = numbers[rand() % 10];
        }
    }
}

MessagePCP create_message(int nseq, int duracao, Timestamp time, int tipo = 0) {
    MessagePCP message;
    message.tipo = tipo;
    message.nseq = nseq;
    message.op[8];
    message.duracao = duracao;
    message.time = time;
    Gen(message.op);
    return message;

}

//MUDAR NOME
MessagePCP generate_message_pcp(int& nseq) {

    int duracao;
    SYSTEMTIME time;
    Timestamp timestamp;

    duracao = (rand() % 10000);

    GetLocalTime(&time);

    timestamp.hour = time.wHour;
    timestamp.minute = time.wMinute;
    timestamp.second = time.wSecond;
    timestamp.millisecond = time.wMilliseconds;

    nseq = (nseq + 1) % 9999;

    MessagePCP messageplc = create_message(nseq, duracao, timestamp);

    return messageplc;

}

void show_message(MessagePCP msg) {

    printf("NSEQ: %i OP: %s HORA DE INICIO: %i:%i:%i DURACAO: %i\n",
        msg.nseq, msg.op, msg.time.hour, msg.time.minute, msg.time.second, msg.duracao);
}

