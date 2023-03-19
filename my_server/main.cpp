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
	memset(&server_addr, 0, sizeof(server_addr));
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

//从指定套接字中读取一行数据,保存到buff中，返回实际读取到的字节数
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
	//向指定套接字发送一个提示还没有实现的错误页面
}

void not_found(int client) {
	//
}

void headers(int clieant) {
	//发送响应包的头信息
}

void cat(int client, FILE* resource) {

}

void server_file(int client, const char* fileName) {
	char numchars = 1;
	char buff[1024];
	
	//把请求数据包的剩余数据行读完
	while (numchars > 0 && strcmp(buff, "\n")) {
		numchars = get_line(client, buff, sizeof(buff));
		PRINTF(buff);
	}

	FILE* resource = fopen(fileName, "r");
	if (resource == NULL) {
		not_found(client);
	}
	else {
		//正式发送资源给浏览器
		headers(client);

		//发送请求的资源信息
		cat(client, resource);

		printf("资源发送完毕！\n");

	}
	fclose(resource);
}

//处理用户请求的服务线程
DWORD WINAPI accept_request(LPVOID arg) {


	char buff[1024];
	int client = (SOCKET)arg; //客户端套接字

	//读取一行数据
	int numchars = get_line(client, buff, sizeof(buff));
	PRINTF(buff);

	char method[255];
	int j = 0, i = 0;
	while (!isspace(buff[j]) && i < sizeof(method) - 1) { //isspace为跳过空白字符
		method[i++] = buff[j++];
	}
	method[i] = 0;
	PRINTF(method);

	//检查请求的方法,本服务器是否支持
	if (stricmp(method, "GET") && stricmp(method, "POST")) {
		//像浏览器返回一个错误提示页面
		unimplement(client);
		return 0;
	}

	//解析资源文件的路径
	char url[255]; //存放请求资源的完成路径
	i = 0;
	//跳过资源路径前边的空格
	while (isspace(buff[j]) && j < sizeof(buff)) j++; //isspace为跳过空白字符

	while (!isspace(buff[j]) && i < sizeof(url) - 1 && j < sizeof(buff)) {
		url[i++] = buff[j++];
	}
	url[i] = 0;
	PRINTF(url);

	//资源目录
	char path[512] = "";
	sprintf(path, "htdocs%s", url);
	if (path[strlen(path) - 1] == '/') {
		strcat(path, "index.html");
	}
	
	PRINTF(path);

	//调用接口判断访问的是目录还是文件
	struct stat status;
	if (stat(path, &status) == -1) {
		//请求包的剩余数据全部读取完毕
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
	printf("服务已启动，正在监听 %d 端口...", port);

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	while (1) {
		//阻塞式等待用户通过浏览器访问
		int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
		if (client_sock == -1) {
			error_die("链接失败");
		}
		//创建一个新的线程
		DWORD threadId = 0;
		CreateThread(0, 0, accept_request, (void*)client_sock, 0, &threadId);

	}

	// "/" 网站服务器资源目录下的 index.html


	closesocket(server_sock);
	return 0;

}
