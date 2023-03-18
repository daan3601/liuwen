#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")

void error_die(const char* str) {
	perror(str);
	exit(1);
}

int startup(unsigned short *port) {
	//1.����ͨ�ų�ʼ��
	WSADATA data;
	int ret = WSAStartup(
		MAKEWORD(1, 1), //ʹ��1.1�汾��Э��
		&data);
	if (ret) {
		error_die("����ͨ�ų�ʼ��ʧ��");
	}

	//2.�����׽���
	int server_socket = socket(PF_INET, //�׽�������
		SOCK_STREAM,	//�����ʾʹ��������ͨ��
		IPPROTO_TCP);   //ʹ��tcpͨ��Э��
	if (server_socket == -1) {
		error_die("�׽��ִ���ʧ��");
		}

	//3.���ö˿ڿɸ���
	int opt = 1;
	ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&opt, sizeof(opt));
	if (ret == -1) {
		error_die("�˿ڿɸ�������ʧ��");
	}

	//4.���÷������������ַ
	struct sockaddr_in server_addr;
	memset(&server_addr,0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//5.���׽���
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		error_die("�׽��ְ�ʧ��");
	}

	//��̬����һ���˿�
	int nameLen = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &nameLen) < 0) {
			error_die("��ȡ�˿ں�ʧ��");
		}
		*port = server_addr.sin_port;
	}

	//6.������������
	if (listen(server_socket, 5) < 0) {
		error_die("�������д���ʧ��");
	}
	return server_socket;
}

//�����û�����ķ����߳�
DWORD WINAPI accept_request(LPVOID arg) {
	return 0;
}

int main() {

	unsigned short port = 0;
	int server_sock = startup(&port);
	printf("���������������ڼ��� %d �˿�...", port);
	
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	while (1) {
		//����ʽ�ȴ��û�ͨ�����������
		int client_sock = accept(server_sock,(struct sockaddr*)&client_addr, &client_addr_len);
		if (client_sock == -1) {
			error_die("����ʧ��");
		}
		//����һ���µ��߳�
		DWORD threadId = 0;
		CreateThread(0, 0, accept_request,(void*)client_sock,0,&threadId);

	}
	closesocket(server_sock);
	return 0;
}