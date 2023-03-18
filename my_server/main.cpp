#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")

void error_die(const char* str) {
	perror(str);
	exit(1);
}

int startup(unsigned short *port) {
	//1.网络通信初始化
	WSADATA data;
	int ret = WSAStartup(
		MAKEWORD(1, 1), //使用1.1版本的协议
		&data);
	if (ret) {
		error_die("网络通信初始化失败");
	}

	//2.创建套接字
	int server_socket = socket(PF_INET, //套接字类型
		SOCK_STREAM,	//这里表示使用数据流通信
		IPPROTO_TCP);   //使用tcp通信协议
	if (server_socket) {
		error_die("套接字创建失败");
	}

}

int main() {

	unsigned short port = 80;
	int server_sock = startup(&port);
	printf("服务已启动，正在监听 %d 端口...", port);
	return 0;
}