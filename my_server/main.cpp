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
	if (server_socket == -1) {
		error_die("套接字创建失败");
		}

	//3.设置端口可复用
	int opt = 1;
	ret = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&opt, sizeof(opt));
	if (ret == -1) {
		error_die("端口可复用设置失败");
	}

	//4.配置服务器的网络地址
	struct sockaddr_in server_addr;
	memset(&server_addr,0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//5.绑定套接字
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		error_die("套接字绑定失败");
	}

	//动态分配一个端口
	int nameLen = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &nameLen) < 0) {
			error_die("获取端口号失败");
		}
		*port = server_addr.sin_port;
	}

	//6.创建监听队列
	if (listen(server_socket, 5) < 0) {
		error_die("监听队列创建失败");
	}
	return server_socket;
}

//处理用户请求的服务线程
DWORD WINAPI accept_request(LPVOID arg) {
	return 0;
}

int main() {

	unsigned short port = 0;
	int server_sock = startup(&port);
	printf("服务已启动，正在监听 %d 端口...", port);
	
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	while (1) {
		//阻塞式等待用户通过浏览器访问
		int client_sock = accept(server_sock,(struct sockaddr*)&client_addr, &client_addr_len);
		if (client_sock == -1) {
			error_die("链接失败");
		}
		//创建一个新的线程
		DWORD threadId = 0;
		CreateThread(0, 0, accept_request,(void*)client_sock,0,&threadId);

	}
	closesocket(server_sock);
	return 0;
}