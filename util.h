//
// Created by qjz on 2023/9/4.
//
#include "fstream"
#include "winsock2.h"
#include "constant.h"
#include "windows.h"

using namespace std;


bool init(){
    WSADATA a;
    int nRc = WSAStartup(0x0101, &a);
    if(nRc){
        WSACleanup();
        return true;
    }
    else
        return false;
}

bool isError(char buf[]){
    if(buf[1] == ERR){
        return true;
    }
    return false;
}

SOCKET get_socket(){
    SOCKET socket1 = INVALID_SOCKET;
    socket1 = socket(AF_INET,SOCK_DGRAM,0);
    if(socket1 == INVALID_SOCKET){
        printf("Error at socket: %d\n", WSAGetLastError());
        WSACleanup();
        return socket1;
    }

    // 定义要绑定的地址信息
    sockaddr_in serverAddress, clientAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // 替换为您的目标IP地址
    serverAddress.sin_port = htons(PORT_NUMBER); // 替换为您的目标端口号
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // 替换为您的目标IP地址
    clientAddress.sin_port = htons(0); // 替换为您的目标端口号

    // 使用bind函数将套接字与指定的IP地址和端口绑定
    if (bind(socket1, (struct sockaddr*)&clientAddress, sizeof(clientAddress)) == SOCKET_ERROR) {
        printf("Error at bind: %d\n", WSAGetLastError());
        closesocket(socket1);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return socket1;
}

void RRQ(string type, SOCKET connectSocket){
    sockaddr_in serverAddress;
    int size = sizeof(serverAddress), bytesnum, num = 1, byteRead;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // 替换为您的目标IP地址
    serverAddress.sin_port = htons(PORT_NUMBER); // 替换为您的目标端口号

    //构造请求包
    char buf[516], buf1[516];
    string filename, filename1;
    cout << "请输入要下载的文件名:";
    cin >> filename;
    cout << "下载至:";
    cin >> filename1;
    ZeroMemory(buf1, sizeof(buf1));
    buf1[1] = READ_REQUEST;
    strcpy(buf1 + OP_LEN, &filename[0]);
    if(type == OCTET || type == NET_ASCII) strcpy(buf1 + OP_LEN + filename.size() + 1, &type[0]);
    else {
        printf("illegal type\n");
        return ;
    }

    int sendNum = 1;
    //发送请求包
    sendto(connectSocket, buf1, 2 + filename.size() + type.size() + 2, 0, (struct sockaddr*)&serverAddress, size);

    //打开文件
    FILE *in;
    in = fopen(&filename1[0],"a+");


    while(true){
        if(sendNum > MAX_SEND_TIME){
            printf("超过最大重传数\n");
            break;
        }
        ZeroMemory(buf, sizeof(buf));
        bytesnum = recvfrom(connectSocket, buf, FILE_BYTES, 0, (struct sockaddr*)&serverAddress, &size);
        if(isError(buf)){
            printf("Error at : %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            exit(1);
        }
        //错误处理
        if(bytesnum == SOCKET_ERROR && num == 1){
            printf("Error at : %d\n", WSAGetLastError());
            sendto(connectSocket, buf1, 2 + filename.size() + type.size() + 2, 0, (struct sockaddr*)&serverAddress, size);
            sendNum++;
            continue;
        }else if(bytesnum == SOCKET_ERROR && num != 1){
            printf("Error at : %d\n", WSAGetLastError());
            buf[1] = ACK; buf[3] = num;
            sendto(connectSocket, buf, ACK_BYTES, 0, (struct sockaddr*)&serverAddress, size);
            sendNum++;
            continue;
        }else {
            fprintf(in, "%s", buf + 4);
            sendNum = 1;
            //发送ACK
            if(buf[3] == num){
                buf[1] = ACK; buf[3] = num;
            }else{
                num++;
                buf[1] = ACK; buf[3] = num;
            }
            do{
                byteRead = sendto(connectSocket, buf, ACK_BYTES, 0, (struct sockaddr*)&serverAddress, size);
            }while(byteRead == SOCKET_ERROR);
        }

        if(bytesnum < FILE_BYTES){
            //关闭文件
            fclose(in);
            //退出循环
            break;
        }
    }
    printf("receive succeed!\n");
}

void WRQ(string type, SOCKET connectSocket){
    int bytenum;
    sockaddr_in serverAddress;
    int size = sizeof(serverAddress), byteRead, num = 1;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // 替换为您的目标IP地址
    serverAddress.sin_port = htons(PORT_NUMBER); // 替换为您的目标端口号
    //发送请求包
    char buf[FILE_BYTES], buf1[FILE_BYTES];
    string filename, filename1;
    cout << "请输入要上传到服务器的文件名:\n";
    cin >> filename;
    cout << "请输入要上传的本地文件名:\n";
    cin >> filename1;
    ZeroMemory(buf1, sizeof(buf1));
    buf1[1] = WRITE_REQUEST;
    strcpy(buf1 + OP_LEN, &filename[0]);
    if(type == OCTET || type == NET_ASCII) strcpy(buf1 + OP_LEN + filename.size() + 1, &type[0]);
    else {
        printf("illegal type\n");
        return ;
    }

    sendto(connectSocket, buf1, 2 + filename.size() + type.size() + 2, 0, (struct sockaddr*)&serverAddress, size);


    //清零
    ZeroMemory(buf, sizeof(buf));

    int sendNum = 1;

    //确认ACK
    while(true){
        if(sendNum > MAX_SEND_TIME){
            printf("超过最大重传数\n");
            return;
        }
        bytenum = recvfrom(connectSocket, buf, ACK_BYTES, 0, (struct sockaddr*)&serverAddress, &size);
        if(isError(buf)){
            printf("Error at : %d\n", WSAGetLastError());
            closesocket(connectSocket);
            WSACleanup();
            exit(1);
        }
        if(bytenum == SOCKET_ERROR){
            sendNum++;
            sendto(connectSocket, buf1, 2 + filename.size() + type.size() + 2, 0, (struct sockaddr*)&serverAddress, size);
        }else{
            sendNum = 1;
            break;
        }
    }


    //清零
    ZeroMemory(buf, sizeof(buf));

    //打开文件
    ifstream in;
    in.open(filename1, ios::in);
    if(!in.is_open()){
        printf("open file failed\n");
        return;
    }

    //将文件数据循环写入缓冲区
    while(true){
        //发包
        buf[1] = DATA; buf[3] = num;
        in.read(buf + 4, 512);
        streamsize bytesRead = in.gcount();
        if(bytesRead > 0){
            //构造数据包
            //发送数据包
            sendto(connectSocket, buf, bytesRead + 4, 0, (struct sockaddr*)&serverAddress, size);
        }else{
            printf("read file failed\n");
            return;
        }


        //接收ACK
        while(true){
            if(sendNum > MAX_SEND_TIME){
                printf("超过最大重传数\n");
                return;
            }
            byteRead = recvfrom(connectSocket, buf, 30, 0, (struct sockaddr*)&serverAddress, &size);
            if(buf[3] == num) num++;
            if(byteRead == SOCKET_ERROR){
                sendto(connectSocket, buf, bytesRead + 4, 0, (struct sockaddr*)&serverAddress, size);
                sendNum++;
            }else break;
        }

        //退出循环
        if(bytesRead < 512){
            break;
        }
    }
    //关闭文件
    in.close();
    printf("send succeed!\n");
}



