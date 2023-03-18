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
	if (server_socket) {
		error_die("�׽��ִ���ʧ��");
	}

}

int main() {

	unsigned short port = 80;
	int server_sock = startup(&port);
	printf("���������������ڼ��� %d �˿�...", port);
	return 0;
}