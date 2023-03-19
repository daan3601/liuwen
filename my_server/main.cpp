#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#pragma comment(lib,"WS2_32.lib")

#define PRINTF(str) printf("[%s - %d]"#str "=%s\n", __func__, __LINE__, str);

void error_die(const char* str) {
	perror(str);
	exit(1);
}

int startup(unsigned short* port) {
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
	memset(&server_addr, 0, sizeof(server_addr));
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

//��ָ���׽����ж�ȡһ������,���浽buff�У�����ʵ�ʶ�ȡ�����ֽ���
int get_line(int sock, char* buff, int size) {
	char c = 0; //'\0'
	int i = 0;
	while (i < size - 1 && c != '\n') {
		int n = recv(sock, &c, 1, 0);
		if (n > 0) {
			if (c == '\r') {
				n = recv(sock, &c, 1, MSG_PEEK);
				if (n > 0 && c == '\n') {
					recv(sock, &c, 1, 0);
				}
				else {
					c = '\n';
				}
			}
			buff[i++] = c;
		}
		else {
			// 
			c = '\n';
		}
	}
	buff[i] = 0; //'\0'
	return i;
}

void unimplement(int client) {
	//��ָ���׽��ַ���һ����ʾ��û��ʵ�ֵĴ���ҳ��
}

void not_found(int client) {
	//
}

void headers(int clieant) {
	//������Ӧ����ͷ��Ϣ
}

void cat(int client, FILE* resource) {

}

void server_file(int client, const char* fileName) {
	char numchars = 1;
	char buff[1024];
	
	//���������ݰ���ʣ�������ж���
	while (numchars > 0 && strcmp(buff, "\n")) {
		numchars = get_line(client, buff, sizeof(buff));
		PRINTF(buff);
	}

	FILE* resource = fopen(fileName, "r");
	if (resource == NULL) {
		not_found(client);
	}
	else {
		//��ʽ������Դ�������
		headers(client);

		//�����������Դ��Ϣ
		cat(client, resource);

		printf("��Դ������ϣ�\n");

	}
	fclose(resource);
}

//�����û�����ķ����߳�
DWORD WINAPI accept_request(LPVOID arg) {


	char buff[1024];
	int client = (SOCKET)arg; //�ͻ����׽���

	//��ȡһ������
	int numchars = get_line(client, buff, sizeof(buff));
	PRINTF(buff);

	char method[255];
	int j = 0, i = 0;
	while (!isspace(buff[j]) && i < sizeof(method) - 1) { //isspaceΪ�����հ��ַ�
		method[i++] = buff[j++];
	}
	method[i] = 0;
	PRINTF(method);

	//�������ķ���,���������Ƿ�֧��
	if (stricmp(method, "GET") && stricmp(method, "POST")) {
		//�����������һ��������ʾҳ��
		unimplement(client);
		return 0;
	}

	//������Դ�ļ���·��
	char url[255]; //���������Դ�����·��
	i = 0;
	//������Դ·��ǰ�ߵĿո�
	while (isspace(buff[j]) && j < sizeof(buff)) j++; //isspaceΪ�����հ��ַ�

	while (!isspace(buff[j]) && i < sizeof(url) - 1 && j < sizeof(buff)) {
		url[i++] = buff[j++];
	}
	url[i] = 0;
	PRINTF(url);

	//��ԴĿ¼
	char path[512] = "";
	sprintf(path, "htdocs%s", url);
	if (path[strlen(path) - 1] == '/') {
		strcat(path, "index.html");
	}
	
	PRINTF(path);

	//���ýӿ��жϷ��ʵ���Ŀ¼�����ļ�
	struct stat status;
	if (stat(path, &status) == -1) {
		//�������ʣ������ȫ����ȡ���
		while (numchars > 0 && strcmp(buff, "\n")) {
			numchars = get_line(client, buff, sizeof(buff));
		}
		not_found(client);
	}
	else {
		if ((status.st_mode & S_IFMT) == S_IFDIR) {
			strcat(path, "index.html");
		}

		server_file(client, path);
	}

	closesocket(client);
	return 0;
}

int main() {

	unsigned short port = 80;
	int server_sock = startup(&port);
	printf("���������������ڼ��� %d �˿�...", port);

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	while (1) {
		//����ʽ�ȴ��û�ͨ�����������
		int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_sock == -1) {
			error_die("����ʧ��");
		}
		//����һ���µ��߳�
		DWORD threadId = 0;
		CreateThread(0, 0, accept_request, (void*)client_sock, 0, &threadId);

	}

	// "/" ��վ��������ԴĿ¼�µ� index.html


	closesocket(server_sock);
	return 0;

}
