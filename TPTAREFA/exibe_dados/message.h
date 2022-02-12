#pragma once
#ifndef MESSAGE_H_
#define MESSAGE_H_

typedef struct {
    int hour;
    int minute;
    int second;
    int millisecond;
} Timestamp;

/* Definicao de mensagens da leitura de supervisor */

typedef struct {
    int tipo;
    int nseq;
    double t_zona1;
    double t_zona2;
    double t_zona3;
    double volume;
    double pressao;
    Timestamp time;

} MessageSupervisor;

MessageSupervisor create_message(int nseq, double t_zona1, double t_zona2, double t_zona3,
    double volume, double pressao, Timestamp time, int tipo);

MessageSupervisor generate_message_sup(int& nseq);

void show_message(MessageSupervisor msg);

/* Definicao de mensagens da leitura de PCP */

typedef struct {
    int tipo;
    int nseq;
    char op[8];
    int duracao;
    Timestamp time;

} MessagePCP;

MessagePCP create_message(int nseq, int duracao, Timestamp time, int tipo);

MessagePCP generate_message_pcp(int& nseq);

void show_message(MessagePCP msg);

/* Definicao de meta mensagem */

typedef struct {
    MessagePCP pcp;
    MessageSupervisor supervisor;
    int type;
} Message;

#endif // MESSAGE_H_