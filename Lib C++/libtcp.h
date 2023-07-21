#ifndef __LIBTCP_H__
#define __LIBTCP_H__

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "skel.h"
#include <chrono>
#include <algorithm>

#define NLISTEN		5		        //максимальное колличество подключений
#define BUF         37000    	    //размер буфера сообщения
#define ACK         0x6             //символ подтверждения ACK
//мы можем добавить символы для управления потоком например

#define MRSZ        128                     //Максимальное число неподтвержденных сообщений для переключения на другую сеть
#define T0          20                      //частота отправки сообщений в OUT
#define T1          100                     //ждать 100мс до первого ACK
#define TSTAT       5000                    //вывод статистики каждые 5 сек
#define T2          5000                    //5сек между пульсами по второму соединению
#define ACKSZ       (sizeof(u_int32_t)+1)   //размер ACK
#define COOKIESZ    sizeof(u_int32_t)       //размер области номера пакета

extern char* program_name;		    // для ошибок 

//Глобальные данные для tselect
#define NTIMERS 25                      
/*Сколько таймеров сосздаетья за раз
Если все они задействованы и есть новое обращение 
то снова выделяется такое же количество таймеров*/

typedef struct tevent_t tevent_t;
//каждыйй таймер составляет структуру
struct tevent_t                     
{
    tevent_t *next;                 //Структуры связаны полем next
    struct timeval tv;              //время срабатывания таймеров
    void (*func) (void *);          //указатель на функцию
    void *arg;                      //указатель на ее аргумент
    unsigned int id;                //Идентификатор активного таймера
};

//стуктура пакета сообщения
typedef struct 
{
    u_int32_t len;      //длина признака и данных - удаленный хост может использовать это поле для разбиения на отдельные записи
    u_int32_t cookie;   //признак сообщения - это 32бит порядковый номер сообщения
    char buf[BUF];      //сообщение
} packet_t;

//стуктура посылаемого сообщения содержит сам пакет и идентификатор таймера
typedef struct
{
    packet_t pkt;               //указатель на сохраненное сообщение
    int id;                     //идентификатор таймера - т.е. таймера ретрансмиссии
    struct timeval start;       //время для контроля
}msgrec_t;

//структура сокета UDP
struct client_sock_udp
{
	SOCKET sock_hb = (-1);
	socklen_t AddrSize = sizeof(addr.sin_addr);
	struct sockaddr_in addr;
};

//структура двойного сокетов UDP 
struct client_sock_udp_oi
{
	SOCKET out = (-1);
    SOCKET in = (-1);
	socklen_t SizeOut = sizeof(addr_out);
    socklen_t SizeIn = sizeof(addr_in);
	struct sockaddr_in addr_in;
    struct sockaddr_in addr_out;
};

typedef void (*tofunc_t)(void*);

//вывод и обработка ошибок
void error(int, int, const char*, ...);

//считывание данных ровно n байт
int readn(SOCKET, char*, size_t);
int readn(SOCKET, char*, size_t, struct sockaddr_in*);
//считываение данных переменной длины
int readvrec( SOCKET, char *, size_t );
int readvrec( SOCKET, char *, size_t ,struct sockaddr_in*);

//фукнция заполнение структуры адреса
void set_address(const char*,const char*, struct sockaddr_in*, char*);

//TCP 
int tcp_client(const char*,const  char*); //Клиент TCP
int tcp_client_while(const char* hname,const  char* sname); //Клиент TCP
int tcp_server(const char*,const  char*); //Сервер TCP
int tcp_server(const char* hname,const  char* sname, char* device);//Сервер TCP на конкретную сетевую карту

//UDP
int udp_server(const char* hname,const char* sname, char* device);//Сервер UDP на конкретную сетевую карту
int udp_server(const char* ,const  char* ); //Сервер UDP
int udp_client(const char*,const  char*, struct sockaddr_in*); //Клиент UDP

//функции тестирования
int test_spead(SOCKET,SOCKET);
const char* testping(client_sock_udp, client_sock_udp,const  char*,const  char*);

//функция select с множеством таймеров - мультиплексирование ввода/вывода
int tselect( int maxfd, fd_set *rdmask, fd_set *wrmask, fd_set *exmask );

//функция создания таймера
unsigned int timeout( void ( *handler ) ( void * ), void *arg, int ms );

//функция уничтожения таймеров
void unitimeout (unsigned int id);



#endif