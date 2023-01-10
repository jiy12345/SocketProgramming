#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<winsock2.h>

#include "Protocol.h"

void MakePacket(PACKET& packet, SOCKET client_socket, char* msg, short packet_type) {
	ZeroMemory(&packet, sizeof(PACKET));
	packet.ph.len = strlen(msg) + PACKET_HEADER_SIZE;
	packet.ph.type = packet_type;
	memcpy(packet.msg, msg, strlen(msg));
}

int SendMsg(SOCKET client_socket, char* msg, short packet_type) 
{
	PACKET packet;
	MakePacket(packet, client_socket, msg, packet_type);

	char* msgSend = (char*)&packet;

	int iSendBytes = send(client_socket, msgSend, packet.ph.len, 0);
	if (iSendBytes == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			int error_num = WSAGetLastError();
			std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
			closesocket(client_socket);
			return error_num;
		}
	}
	return 0;
}

// WINAPI �����带 ���� �Լ�
// ��������� ������ �Լ��� �Ǳ� ���� �ݵ�� WINAPI�� ����Ǿ��
DWORD WINAPI SendThread(LPVOID socket)
{
	SOCKET client_socket = (SOCKET)socket;
	while (true)
	{
		char msg_to_send[256] = { 0, };
		fgets(msg_to_send, 256, stdin);
		msg_to_send[strlen(msg_to_send) - 1] = 0;
		
		// ���� ��û
		if (strcmp(msg_to_send, "exit") == 0)
		{
			break;
		}
		// ���� �߻�
		if (SendMsg(client_socket, msg_to_send, PACKET_CHAR_MSG) != 0)
		{
			break;
		}
	}
	closesocket(client_socket);
};

int main() {
	WSAData wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
		return -1;
	}

	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);

	std::string server_ip = "192.168.0.174";
	int server_port = 10000;

	//std::cout << "������ ������ ip�� �Է����ּ���: ";
	//std::cin >> server_ip;
	//std::cout << "������ ��Ʈ ��ȣ�� �Է����ּ���: ";
	//std::cin >> server_port;

	// ������ ���� �ּ� ����
	sockaddr_in socket_address;
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
	socket_address.sin_port = htons(10000);
	int return_value = connect(client_socket, (sockaddr*)&socket_address, sizeof(socket_address));
	if (return_value == SOCKET_ERROR) {
		int error_num = WSAGetLastError();
		std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
		closesocket(client_socket);
		WSACleanup();
		return error_num;
	}

	// �� ���ŷ �������� ����
	u_long mode = TRUE;
	ioctlsocket(client_socket, FIONBIO, &mode);

	// WINAPI ������ ����
	DWORD dwThreadID;
	HANDLE hClient = CreateThread(0, 0, SendThread, (LPVOID)client_socket, 0, &dwThreadID);

	int cur_packet_header_bytes = 0;
	char packet_header[PACKET_HEADER_SIZE];
	while (true) {

		// ��Ŷ ��� �б�
		char msg_to_recv[256] = { 0, };
		int recved_bytes = recv(client_socket, msg_to_recv, PACKET_HEADER_SIZE - cur_packet_header_bytes, 0);
		cur_packet_header_bytes += recved_bytes;
		memcpy(packet_header + cur_packet_header_bytes, msg_to_recv, sizeof(char) * recved_bytes);
		if (recved_bytes == 0) {
			std::cout << "���� ���� ����\n";
			break;
		}
		if (recved_bytes == SOCKET_ERROR) {
			// ���� ���� ������ ���� �׻� ������ WSAEWOULDBLOCK ���̾�� �Ѵ�!
			// �׷��� ������ ����
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				int error_num = WSAGetLastError();
				std::cout << "���� �߻�! ���� �ڵ�: " << error_num << '\n';
				closesocket(client_socket);
				WSACleanup();
				return error_num;
			}
		}
		
		// ��Ŷ ����� ��� �о��� ��� => ���� ������ �о����
		if (cur_packet_header_bytes == PACKET_HEADER_SIZE)
		{
			// ��Ŷ ��� ���� �� ��� �Է��ϱ�
			PACKET packet;
			ZeroMemory(&packet, sizeof(PACKET));
			memcpy(&packet.ph, packet_header, PACKET_HEADER_SIZE);

			// ���� ������ �б�
			int total_recved_packet_data_bytes = 0;
			while ((packet.ph.len - PACKET_HEADER_SIZE) > total_recved_packet_data_bytes) {
				int recved_bytes = recv(client_socket,
					&packet.msg[total_recved_packet_data_bytes],
					packet.ph.len - PACKET_HEADER_SIZE - total_recved_packet_data_bytes, 0);

				if (recved_bytes == 0)
				{
					printf("���� ���� ����\n");
					break;
				}
				if (recved_bytes == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSAEWOULDBLOCK)
					{
						printf("���� ������ ����\n");
						closesocket(client_socket);
						WSACleanup();
						return 1;
					}
					continue;
				}
				total_recved_packet_data_bytes += recved_bytes;
			} 

			// ��Ŷ Ÿ�Կ� ���� �ٸ� ó�� �ϱ�
			switch (packet.ph.type)
			{
			case PACKET_CHAR_MSG:
			{
				printf("%s\n", packet.msg);
			} break;
			case PACKET_CHATNAME_REQ:
			{
				printf("�̸��� �Է��Ͻÿ� : ");
				char szName[256] = { 0, };
				fgets(szName, 256, stdin);
				szName[strlen(szName) - 1] = 0;
				SendMsg(client_socket, szName, PACKET_NAME_REQ);
				ResumeThread(hClient);
			} break;
			case PACKET_JOIN_USER:
			{
				printf("%s %s\n", packet.msg, "���� �����Ͽ����ϴ�.");
			}break;
			case PACKET_NAME_ACK:
			{
				printf("��ȭ�� ��� ����\n");
			}break;
			}

			cur_packet_header_bytes = 0;
		}
	}

	closesocket(client_socket);
	WSACleanup();
}