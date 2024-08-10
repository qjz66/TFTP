#include <iostream>
#include "winsock2.h"
#include "constant.h"
#include "util.h"

using namespace std;



int main(){
    //初始化
    bool judge = init();
    if(judge){
        printf("WSA not initialised,%d\n",WSAGetLastError());
        WSACleanup();
    }

    //创建并连接套接字
    SOCKET connectSocket = get_socket();
    if(connectSocket == INVALID_SOCKET){
        printf("get socket failed:%d\n",WSAGetLastError());
    }else
        printf("get socket success\n");

    // 设置接收和发送超时时间（以毫秒为单位）
    int timeout = 3000; // 设置为3秒
    if (setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "无法设置接收超时: " << strerror(WSAGetLastError()) << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    /*if (setsockopt(connectSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "无法设置发送超时: " << strerror(WSAGetLastError()) << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }*/


    //连接服务端

    while(true){
        //输入mode type
        short mode;
        string type;
        printf("请求数据:1\n发送数据:2\n退出程序:6 ");
        cin >> mode;
        if(mode == QUIT){
            closesocket(connectSocket);
            WSACleanup();
            break;
        }
        printf("请选择数据发送方式:octet/netascii ");
        cin >> type;
        cout << type;

        if(mode == 1)
            RRQ(type, connectSocket);
        else
            WRQ(type, connectSocket);
    }
    return 0;
}
