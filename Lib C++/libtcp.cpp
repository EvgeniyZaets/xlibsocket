#include "libtcp.h"
static tevent_t *active=NULL;       //активные таймеры - указывает на первый таймер в списке
static tevent_t *free_list=NULL;    //свободные таймеры - указывает на первый свободный таймер в списке сбоводных

//фкнция обработки ошибок
void error(int status, int err,const char* fmt, ...) //передаем статус, номер ошибки и сообщение
{
	va_list ap; 		//создаем указатель на неопределенное колличество параметров
	va_start(ap, fmt); 	//связываем с первым необязательным параметром
	fprintf(stderr, "%s: ", program_name);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (err)
		fprintf(stderr, ": %s (%d)\n", strerror(err), err);
	if (status)
		exit(status);
}
/*Если status не равно 0, то error завершает программу после печати диагностического сообщения;
в противном случае она возвращает управление. Если err не равно 0, то считается,
что это значение системной переменной errno. При этом в конец сообщения дописывается
соответствующая этому значению строка и числовое значение кода ошибки.*/

SOCKET tcp_server(const char* hname,const char* sname) //ip адрес и порт
{
	struct sockaddr_in local; 		//переменная структуры адреса
	SOCKET s; 						//сокет сервера
	const int on = 1; 				//флаг

	set_address(hname, sname, &local, "tcp"); 		//заполняем структуру адреса
	s = socket(AF_INET, SOCK_STREAM, 0); 			//создаем сокет TCP
	if (!isvalidsock(s)) 							//проверка на ошику
		error(1, errno, "socket call failed"); 		//ошибка

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) 	//задаем параметры сокета 
		error(1, errno, "setsockopt failed"); 								//ошибка

	if (bind(s, (struct sockaddr*)&local, sizeof(local)))		//биндим сокет
		error(1, errno, "bind failed"); 						//ошибка

	if (listen(s, NLISTEN)) 					//слушаем 
		error(1, errno, "listen failed"); 		//ошибка

	return s; 		//возвращаем сокет
}

SOCKET tcp_server(const char* hname,const char* sname, char* device) //ip адрес, порт, название сетевой карты 
{
	struct sockaddr_in local; //переменная структуры адреса
	SOCKET s; 				  //сокет сервера
	const int on = 1; 		  //флаг

	set_address(hname, sname, &local, "tcp"); 		//заполняем структуру адреса
	s = socket(AF_INET, SOCK_STREAM, 0); 			//создаем сокет TCP
	if (!isvalidsock(s)) 							//проверка на ошику
		error(1, errno, "socket call failed"); 		//ошибка

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) 	//задаем параметры сокета 
		error(1, errno, "setsockopt failed"); 								//ошибка
	
	if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device)))	//задаем сетевую карту
		error(1, errno, "setsockopt failed device"); 						//ошибка

	if (bind(s, (struct sockaddr*)&local, sizeof(local)))	//биндим сокет
		error(1, errno, "bind failed"); 					//ошибка

	if (listen(s, NLISTEN)) 					//слушаем 
		error(1, errno, "listen failed"); 		//ошибка

	return s; 		//возвращаем сокет
}

SOCKET tcp_client(const char* hname,const char* sname) //ip адрес, порт
{
	struct sockaddr_in peer;
	SOCKET s;

	set_address(hname, sname, &peer, "tcp");
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (!isvalidsock(s))
		error(1, errno, "socket call failed");

	if (connect(s, (struct sockaddr*)&peer,
		sizeof(peer)))
		error(1, errno, "connect failed");

	return s;
}

SOCKET tcp_client_while(const char* hname,const char* sname) //ip адрес, порт
{
	struct sockaddr_in peer;
	SOCKET s;

	set_address(hname, sname, &peer, "tcp");
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (!isvalidsock(s))
		error(1, errno, "socket call failed");
	while (true)
	{
	if (connect(s, (struct sockaddr*)&peer, sizeof(peer))) 
	{
		error(0, errno, "connect failed");
		close(s);
		s = socket(AF_INET, SOCK_STREAM, 0);
		sleep(1);
		continue;
	}
	break;
	}
	return s;
}

SOCKET udp_server(const char* hname,const char* sname, char* device) //ip адрес, порт, название сетевой карты 
{
	SOCKET s;
	struct sockaddr_in local;
	const int on = 1; 		  		
	set_address(hname, sname, &local, "udp");
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!isvalidsock(s))
		error(1, errno, "socket call failed");

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) 	//задаем параметры сокета 
		error(1, errno, "setsockopt failed"); 								//ошибка

	if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device)))	//задаем название сетевой карты
		error(1, errno, "setsockopt failed device"); 						//ошибка

	if (bind(s, (struct sockaddr*)&local, sizeof(local)))	//биндим сокет
		error(1, errno, "bind failed"); 					//ошибка
	
	return s;		//возвращаем сокет
}

SOCKET udp_server(const char* hname,const char* sname)	//ip адрес, порт
{
	SOCKET s;
	struct sockaddr_in local;
	set_address(hname, sname, &local, "udp");
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!isvalidsock(s))
		error(1, errno, "socket call failed");
	if (bind(s, (struct sockaddr*)&local, sizeof(local)))	//биндим сокет
		error(1, errno, "bind failed"); 					//ошибка

	return s; 		//возвращаем сокет
}

SOCKET udp_client(const char* hname,const char* sname, struct sockaddr_in* sap)
{
	SOCKET s;
	set_address(hname, sname, sap, "udp");
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!isvalidsock(s))
		error(1, errno, "socket call failed");
	return s;
}
//функция тестирования UDP
const char* testping(client_sock_udp a, client_sock_udp b,const  char* addr_a,const  char* addr_b)
{
	const char* tmp;
	printf("Testing - new\n");
	char buffer[500]; //задаем датаграмму для проверки сети
	sockaddr_in client{};
	socklen_t clientSize = sizeof(client);

	int count_msg1 = 0;
	auto start1 = std::chrono::high_resolution_clock::now();//фиксируем первое вермя
	
	for (int i = 0; i < 5; i++) { //пять попыток на отправку и прием 

		if (sendto(a.sock_hb, buffer, strlen(buffer), MSG_CONFIRM, (sockaddr*)&a.addr, sizeof(a.addr)) == -1)
		{
			error(0, errno, "sendto failed addr1: ");

		} else	printf("addr1 sendto # %d\n", i);
		if (recvfrom(a.sock_hb, buffer, sizeof(buffer), MSG_DONTWAIT, (sockaddr*)&client, &clientSize) == -1)
		{
			error(0, errno, "recfrom failed addr1");
			count_msg1 = -1;
			sleep(1);
		}
		else 
		{
			count_msg1 = +1;
			printf("addr1 recvfrom # %d\n", i);
		}
	}
	auto end1 = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

	int count_msg2 = 0;

	auto start2 = std::chrono::high_resolution_clock::now();//фиксируем первое вермя
	for (int i = 0; i < 5; i++) { //пять попыток на отправку и прием

		if (sendto(b.sock_hb, buffer, strlen(buffer), MSG_CONFIRM, (sockaddr*)&b.addr, sizeof(b.addr)) == -1)
		{
			error(0, errno, "sendto failed addr2");
		}
		else printf("addr2 sendto # %d\n", i);
		if (recvfrom(b.sock_hb, buffer, sizeof(buffer), MSG_DONTWAIT, (sockaddr*)&client, &clientSize) == -1)
		{
			error(0, errno, "recfrom failed addr2");
			count_msg2 = -1;
			sleep(1);
		}
		else {
			count_msg2 = +1;
			printf("addr2 recvfrom # %d\n", i);
		}

	}
	auto end2 = std::chrono::high_resolution_clock::now();
	double duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
	
	printf("Testing - is OK\n");
	if ((duration1) <= (duration2)) { //сравниваем среднеарифметическое время

		//вроде первый быстрый
		if (count_msg2 <= count_msg1) // and count_msg1 > 2) 
		{
			//точно первый - cообщений доставил больше или столько же, а также доставил больше половины отправленных
			printf("Server 1 is faster\n");
			tmp = addr_a;
			return tmp;

		}
		else if (count_msg2 > count_msg1)// and count_msg2 > 2) 
		{

			//второй - сообщений доставил больше, а также доставил больше половины отправленных
			printf("Server 2 is stabiliti\n");
			tmp = addr_b;
			return tmp;
		}
		return "Fail";
	}
	else {
		//вроде второй быстрый
		if (count_msg1 <= count_msg2)// and count_msg2 > 2) 
		{
			//точно первый - cообщений доставил больше или столько же, а также доставил больше половины отправленных
			printf("Server 2 is faster\n");
			tmp = addr_b;
			return tmp;

		}
		else if (count_msg1 > count_msg2)// and count_msg1 > 2) 
		{
			//первый - сообщений доставил больше, а также доставил больше половины отправленных
			printf("Server 1 is stabiliti\n");
			tmp = addr_a;
			return tmp;
		}
		return "Fail";
	}
return "Fail";
}

//Функция тестирования двух TCP соединений
int test_spead(SOCKET client1,SOCKET client2)
{
	printf("Testing - new\n");
	// флаг для записи и чтения не ждать (не блокировать)
	// и никаких сообщений о разрыве соединения.
	int flags = MSG_DONTWAIT | MSG_NOSIGNAL; 
	
	int count_msg1 = 0;
	char buffer[500];
	auto start1 = std::chrono::high_resolution_clock::now();//фиксируем первое вермя
	
	for (int i = 0; i < 5; i++) { //пять попыток на отправку и прием 

		if (send(client1, buffer, sizeof(buffer),0) < 0)
		{
			error(0, errno, "sendto failed addr1: ");

		} else	printf("addr1 sendto # %d\n", i);
		if (recv(client1, buffer, sizeof(buffer), 0) < 0)
		{
			error(0, errno, "recfrom failed addr1");
			count_msg1 = -1;
		}
		else 
		{
			count_msg1 = +1;
			printf("addr1 recvfrom # %d\n", i);
		}
	}
	auto end1 = std::chrono::high_resolution_clock::now();
	auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

	int count_msg2 = 0;
	auto start2 = std::chrono::high_resolution_clock::now();//фиксируем первое вермя
	for (int i = 0; i < 5; i++) { //пять попыток на отправку и прием

		if (send(client1, buffer, sizeof(buffer),0) < 0)
		{
			error(0, errno, "sendto failed addr1: ");

		} else	printf("addr1 sendto # %d\n", i);
		if (recv(client1, buffer, sizeof(buffer), 0) < 0)
		{
			error(0, errno, "recf#include failed addr1");
			count_msg2 = -1;
		}
		else {
			count_msg2 = +1;
			printf("addr2 recvfrom # %d\n", i);
		}
	}
	auto end2 = std::chrono::high_resolution_clock::now();
	double duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();
	
	printf("Testing - is OK\n");
	if ((duration1) <= (duration2)) { //сравниваем среднеарифметическое время
		//вроде первый быстрый
		if (count_msg2 <= count_msg1) // and count_msg1 > 2) 
		{
			//точно первый - cообщений доставил больше или столько же, а также доставил больше половины отправленных
			printf("Server 1 is faster\n");
			return 1;
		}
		else if (count_msg2 > count_msg1)// and count_msg2 > 2) 
		{
			//второй - сообщений доставил больше, а также доставил больше половины отправленных
			printf("Server 2 is stabiliti\n");
			return 2;
		}
		return -1;
	}
	else {
		//вроде второй быстрый
		if (count_msg1 <= count_msg2)// and count_msg2 > 2) 
		{
			//точно первый - cообщений доставил больше или столько же, а также доставил больше половины отправленных
			printf("Server 2 is faster\n");
			return 2;
		}
		else if (count_msg1 > count_msg2)// and count_msg1 > 2) 
		{
			//первый - сообщений доставил больше, а также доставил больше половины отправленных
			printf("Server 1 is stabiliti\n");
			return 1;
		}
		return -1;
	}
	return -1;
}


//функция заполнения структуры sockaddr_in 
void set_address(const char* hname,const char* sname,
	struct sockaddr_in* sap, char* protocol) 		//адрес хоста или его имя, порт, стркуктура адреса, протокол
{
	struct servent* sp; 							//стрктура для хранения имени и номера службы
	struct hostent* hp; 							//стрктура для имени узла
	char* endptr;  									//Ссылка на объект содержит адрес следующего символа в строке
	short port; 									//порт

	//bzero(sap, sizeof(*sap)); 
	memset(sap, 0, sizeof(sap));					//обнуляет структуру
	sap->sin_family = AF_INET; 						//задаем простратсво адресных имен
	if (hname != NULL) 								//если имя не пусото, полагаем что это числовой адрес 
	{
		if (!inet_aton(hname, &sap->sin_addr)) 		//пытаемя преобразовать обычный вид IP-адреса 
		{
			hp = gethostbyname(hname); 				//если не получилось разрешаем IP-адрес сервера в соответствии с доменным именем
			if (hp == NULL) 										//если все равно ошибка то выходим
				error(1, 0, "unknown host: %s\n", hname);
			sap->sin_addr = *(struct in_addr*)hp->h_addr_list[0]; 	//удалсь то заполняем структуру
		}
	}
	else
		sap->sin_addr.s_addr = htonl( INADDR_ANY );
	port = strtol( sname, &endptr, 0 );
	if ( *endptr == '\0' )
		sap->sin_port = htons( port );
	else 											//полагаем что это символическое название сервиса
	{
		sp = getservbyname(sname, protocol); 				//возвращаем соответсвующего номера порта
		if (sp == NULL) 									//если не удалось выходим 
			error(1, 0, "unknown service: %s\n", sname);
		sap->sin_port = sp->s_port; 						// заполняем порт в структуру адреса
	}
}

// Функция для чтения записи переменной длины
/*Функция возвращает 0 (конец файла) если число байтов 
прочитанных readn не точно совпадает с размером целого или -1 если ошибка*/
int readvrec(SOCKET fd, char *bp, size_t len)
{
	u_int32_t reclen;
	int byte;
	/*прочитать длину записи из первого поля*/
	byte = readn(fd,(char*)&reclen, sizeof(u_int32_t));
	if(byte!=sizeof(u_int32_t))
		return byte<0? -1 : 0;
	reclen = htonl(reclen);		//Размер записи преобразуется из сетевого порядка в машинный
	if(reclen > len){
		while (reclen>0)
		{
			/*Проверяется, достаточна ли длина буфера, предоставленного вызываю­
			щей программой, для размещения в нем всей записи. Если места не 
			хватит, то данные считываются в буфер частями по len байт, то есть, 
			по сути, отбрасываются.*/
			byte = readn(fd, bp, len);
			if(byte != len)
				return byte < 0 ? -1 : 0;
			reclen -=len;
			if(reclen < len)
				len = reclen;
		}
		set_errno(EMSGSIZE);
		return -1;
	}
	/*прочитать саму запись*/
	byte = readn(fd,  bp, reclen);
	if(byte != reclen)
		return byte < 0 ? -1 : 0;
	return byte;
}

//для UDP
int readvrec(SOCKET fd, char *bp, size_t len,struct sockaddr_in* sap)
{
	u_int32_t reclen;
	int byte;
	/*прочитать длину записи*/
	byte = readn(fd,(char*)&reclen, sizeof(u_int32_t),sap);
	if(byte!=sizeof(u_int32_t))
		return byte<0? -1 : 0;
	reclen = htonl(reclen);		//Размер записи преобразуется из сетевого порядка в машинный
	if(reclen > len){
		while (reclen>0)
		{
			/*Проверяется, достаточна ли длина буфера, предоставленного вызываю­
			щей программой, для размещения в нем всей записи. Если места не 
			хватит, то данные считываются в буфер частями по len байт, то есть, 
			по сути, отбрасываются.*/
			byte = readn(fd, bp, len, sap);
			if(byte != len)
				return byte < 0 ? -1 : 0;
			reclen -=len;
			if(reclen < len)
				len = reclen;
		}
		set_errno(EMSGSIZE);
		return -1;
	}
	/*прочитать саму запись*/
	byte = readn(fd,  bp, reclen, sap);
	if(byte != reclen)
		return byte < 0 ? -1 : 0;
	return byte;
}

// Функция для чтения записи ровно len байт
//для TCP
int readn(SOCKET fd, char* buffer, size_t len) //сокет, буфер, длина
{
	int cnt; 			//переменная остатка байт
	int rc; 			//переменная байт что читаем в итерации (шаге)

	cnt = len; 			//задаем начлаьное значение для остатка байт
	while (cnt > 0) 	//если остаток больше нуля 
	{
		rc = recv(fd, buffer, cnt, 0); 	//читаем из сокета
		if (rc < 0)						// ошибка прочтения
		{
			if (errno == EINTR)			// если прервано
				continue;				// перезапуск прочтения 
			return -1;					// вывод ошибки 
		}
		if (rc == 0)					// конец передачи 
			return (len - cnt);			// возвращаем количество прочтенных байт 
		buffer += rc; 					//добавляем к буферу байты что прочли
		cnt -= rc; 						//вычитаем их того что нужно было прочесь (получаем остаток байт)
	}
	return len; 						//возвращаем длину байт
}

//для UDP
int readn(SOCKET fd, char* buffer, size_t len,struct sockaddr_in* sap) //сокет, буфер, длина, структура адреса
{
	int cnt; 			//переменная остатка байт
	int rc; 			//переменная байт что читаем в итерации (шаге)

	cnt = len; 			//задаем начлаьное значение для остатка байт
	while (cnt > 0) 	//если остаток больше нуля 
	{
		rc = recvfrom(fd, buffer, cnt, MSG_DONTWAIT, (sockaddr*)&sap, (socklen_t*)sizeof(sap)); //читаем из сокета
		if (rc < 0)					// ошибка прочтения
		{
			if (errno == EINTR)		// если прервано 
				continue;			// перезапуск прочтения 
			return -1;				// вывод ошибки 
		}
		if (rc == 0)				// конец передачи 
			return (len - cnt);		// возвращаем количество прочтенных байт 
		buffer += rc; 				//добавляем к буферу байты что прочли
		cnt -= rc; 					//вычитаем их того что нужно было прочесь 
									//(получаем остаток байт)
	}
	return len; 					//возвращаем длину байт
}

//функция получения свободного таймера
static tevent_t *allocate_timer(void)  
{
    tevent_t *tp;
    if(free_list == NULL)    	//нужен новый блок таймеров ?
    {   
        free_list = (tevent_t*)malloc(NTIMERS*sizeof(tevent_t));   //выделение из кучи структур tevent_t
        if(free_list == NULL)   
            error(1,0,"не удалось получить таймеры\n");
        for (tp = free_list;                            //Структуры связываются в список
            tp < free_list + NTIMERS - 1; 
            tp++) tp->next = tp+1;
        tp->next = NULL;
    }
    tp=free_list;           //выделить первый
    free_list = tp->next;   //убрать из списка
    return tp;              //передаем свободный таймер
}

//функция таймера
unsigned int timeout(void (*func)(void *), void *arg, int ms) //
{
    tevent_t *tp;
    tevent_t *tcur;
    tevent_t **tprev;
    static unsigned int id = 1;     	//идентификатор таймера
    tp = allocate_timer();          	//получаем таймер
    tp->func = func;                	//передаем в поля название фукнции
    tp->arg = arg;                  	//и ее аргументы
    if(gettimeofday(&tp->tv,NULL)<0)
        error(1,errno,"timeout: ошибка вызова gettimeofday");
    tp->tv.tv_usec += ms * 1000;        //вычисляем момент срабатывания таймера
    if(tp->tv.tv_usec > 1000000)        
    {
        tp->tv.tv_sec +=tp->tv.tv_usec/1000000;
        tp->tv.tv_usec %=1000000;
    }
    for (tprev = &active, tcur = active;            //ищем место в списке для вставки нового таймера
        tcur && !timercmp(&tp->tv,&tcur->tv,<);     //вставить нужно так что бы моменты срабатывания всех предыдущих таймеров были меньше или равны
        tprev = &tcur->next, tcur = tcur->next)     //а момент срабатывания всех последующих больше момента срабатывания нового
    {;}
    *tprev = tp;            //вставляем таймер в нужное место
    tp->next = tcur;        
    tp->id = id++;          //присваиваем значение идентификатору таймера
    return tp->id;          //возвращаем этот идентификатор программе

    /*возвращаем идентификатор а не адрес структуры что бы избежать ситуации 
    когда таймер после сработки возвращается в начало списка свободных
    При выделении нового таймера будет использована именно эта структура 
    Если приложение теперь попытается отменить первый таймер то при условии 
    что возвращается адрес структуры а не индекс будет отменен второй таймер.*/
}

/*
 active - первый в списке активных
 tprev - таймер в списке активных что должен сработать раньше нового
 tcur - таймер в списке активных что должен сработать позже нового

изначально задаем как tprev =  active и потом спускаемся вниз по ссылкам
а tcur задаем как следующий после active т.е. active->next

+-------+     +--------+     +--------+   
| tprev | --> | active | --> | next   |
+-------+     +--------+     +--------+
                                ^
+-------+                       |
| tcur  |-----------------------* 
+-------+
*/

//функция отмены таймера
void unitimeout (unsigned int id)   
{
    tevent_t **tprev;
    tevent_t *tcur;
    for (tprev = &active, tcur = active;       //ищем в списке активных таймер с идентификатором id
    tcur &&id !=tcur->id;                       
    tprev=&tcur->next, tcur = tcur->next)
    {;}
    if(tcur == NULL)       //если в списке нет таймера который пытаемся отменить то выводим сообщение и выходим
    {
        error(0,0,"при выполнении unitimeout указан несуществующий таймер (%d)\n",id);
        return;
    }
    *tprev = tcur->next;        //для отмены таймера исключаем стркутуру из списка активных
    tcur->next = free_list;     //и возвращаем в список свободных
    free_list = tcur;
}

int tselect (int maxpl, fd_set *re, fd_set *we, fd_set *ee)
{
    fd_set rmask;
    fd_set wmask;
    fd_set emask;
    struct timeval now;        //таймер новый
    struct timeval tv;         //таймер первый из очереди значения
    struct timeval *tvp;       //указатель таймера что передаем в select
    tevent_t *tp;              //временная переменная для
    
    int n;
    /*Сохраняем маски событий*/
    if(re) rmask = *re; //на чтение
    if(we) wmask = *we; //на запись
    if(ee) emask = *ee; //исключения
    for(;;)
    {
        if(gettimeofday(&now,NULL)<0) 
            error(1,errno,"tselect: ошибка вызова gettimeofday");
            while(active && !timercmp(&now, &active->tv, <))
            {
                active->func(active->arg);
                tp = active;
                active = active->next;
                tp->next = free_list;
                free_list = tp;
            }
            if(active)  //если список активных таймеров не пуст
            {
                //вычисляем разность между текущим моментом времени 
				//и временем срабатывания таймера в начале списка активных
				//это значение и передаем select
				tv.tv_sec = active->tv.tv_sec - now.tv_sec;; tv.tv_usec = active->tv.tv_usec - now.tv_usec; 
                if(tv.tv_usec < 0)
                {
                    tv.tv_usec +=1000000;
                    tv.tv_sec--;
                }
                tvp = &tv;
        }else if(re == NULL && we == NULL && ee == NULL) return 0; 	//если нет таймеров и событий возвращаем 0
        else tvp=NULL; 						//если нет события таймера но есть ввода то передаем NULL в select 
        n = select(maxpl,re,we,ee,tvp); 
        if(n<0) return -1; 					//возвращаем код ошибки
        if(n>0) return n; 					//возвращаем число событий
        //если select вернул 0 возвращаем начальное значение масок
        if(re) *re = rmask;
        if(we) *we = wmask;
        if(ee) *ee = emask;
    }
}

/*tselect - Возвращаемое значение: число готовых событий, 0 - если событий нет, -1 -ошибка.

timeout - события задаются с его помощью указывая длительность таймера и действие
Когда срабатывает таймер ассоциированный с вызовом timeout вызывается функция handler.
Величина тайм-аута ms задается в миллисекундах. 
Возвращаемое значение: идентификатор таймера для передачи untimeout

untimeout - отменяет таймер до срабатывания*/
