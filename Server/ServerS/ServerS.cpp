#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string> 

#pragma warning(disable: 4996)

//пременные для соединений
SOCKET Connections[100];
int Counter = 0;



enum Packet {//константы отвечающие за тип пакета
	P_ChatMessage,
	P_PersonalMessage,
	P_Test,
	P_userId
};

bool ProcessPacket(int index, Packet packettype) {//функиця обработки пакетов
	switch(packettype) {
	case P_PersonalMessage:
	{
		
		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];

		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		
		
		int indexOfUser_size;
		recv(Connections[index], (char*)&indexOfUser_size, sizeof(int), NULL);
		char* indexOfUser = new char[indexOfUser_size + 1];
		recv(Connections[index], indexOfUser, indexOfUser_size, NULL);
		
		
		int i = atoi(indexOfUser);
		Packet msgtype = P_PersonalMessage;
		send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
		send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
		send(Connections[i], msg, msg_size, NULL);
		delete[] msg;
		break;
		
	}
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char *msg = new char[msg_size + 1];
		
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		for(int i = 0; i < Counter; i++) {
			if(i == index) {
				continue;
			}

			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
		}
		delete[] msg;
		break;
	}
	default:
		std::cout << "Unrecognized packet: " << packettype << std::endl;
		break;
	}

	return true;
}

void ClientHandler(int index) {	//функция принимающая соединение в сокет массиве
	Packet packettype;
	while(true) {
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);

		if(!ProcessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]);
}

int main(int argc, char* argv[]) {
	//загружаем библиотеки
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	//проверка на загрузку библиотеки
	if(WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}

	//заполняем информацию об адрессе сокета
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	//ip адресс
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//сокет
	addr.sin_port = htons(2252);
	addr.sin_family = AF_INET;

	//прослушиваем на прием данных с сокета
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	//привязка адресса сокету
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	//слушаем
	listen(sListen, SOMAXCONN);

	//сокет для клиента, теперь addr содержит ip клиента
	SOCKET newConnection;
	for(int i = 0; i < 100; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		//проверка на то подключен ли пользователь
		if(newConnection == 0) {
			std::cout << "Error #2\n";
		} else {
			std::cout << "Client Connected!\n Client number "<< i <<"\n";

			//Отправляем номер клиента
			Packet userId = P_userId;
			send(Connections[i], (char*)&userId, sizeof(Packet), NULL);
			std::string thisuserId = std::to_string(i);
			int userId_size = thisuserId.size();
			send(Connections[i], (char*)&userId_size, sizeof(int), NULL);
			send(Connections[i], thisuserId.c_str(), userId_size, NULL);

            std::string msg;
			int msg_size = msg.size();
			Packet msgtype = P_ChatMessage;
			
			//записываем соединение в массив
			Connections[i] = newConnection;
			Counter++;
			//включаем работу 2 потоков,чтобы в функции main принимались новые соеденения , а в процедуре ClientHandler будут ожидаться и отправлятся сообщения клиентам
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

			


			//отправляем тестовый пакет
			/*Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);*/
		}
	}


	system("pause");
	return 0;
}