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

    // ����Ҫ�󶨵ĵ�ַ��Ϣ
    sockaddr_in serverAddress, clientAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // �滻Ϊ����Ŀ��IP��ַ
    serverAddress.sin_port = htons(PORT_NUMBER); // �滻Ϊ����Ŀ��˿ں�
    clientAddress.sin_family = AF_INET;
    clientAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // �滻Ϊ����Ŀ��IP��ַ
    clientAddress.sin_port = htons(0); // �滻Ϊ����Ŀ��˿ں�

    // ʹ��bind�������׽�����ָ����IP��ַ�Ͷ˿ڰ�
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
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // �滻Ϊ����Ŀ��IP��ַ
    serverAddress.sin_port = htons(PORT_NUMBER); // �滻Ϊ����Ŀ��˿ں�

    //���������
    char buf[516], buf1[516];
    string filename, filename1;
    cout << "������Ҫ���ص��ļ���:";
    cin >> filename;
    cout << "������:";
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
    //���������
    sendto(connectSocket, buf1, 2 + filename.size() + type.size() + 2, 0, (struct sockaddr*)&serverAddress, size);

    //���ļ�
    FILE *in;
    in = fopen(&filename1[0],"a+");


    while(true){
        if(sendNum > MAX_SEND_TIME){
            printf("��������ش���\n");
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
        //������
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
            //����ACK
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
            //�ر��ļ�
            fclose(in);
            //�˳�ѭ��
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
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDRESS); // �滻Ϊ����Ŀ��IP��ַ
    serverAddress.sin_port = htons(PORT_NUMBER); // �滻Ϊ����Ŀ��˿ں�
    //���������
    char buf[FILE_BYTES], buf1[FILE_BYTES];
    string filename, filename1;
    cout << "������Ҫ�ϴ������������ļ���:\n";
    cin >> filename;
    cout << "������Ҫ�ϴ��ı����ļ���:\n";
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


    //����
    ZeroMemory(buf, sizeof(buf));

    int sendNum = 1;

    //ȷ��ACK
    while(true){
        if(sendNum > MAX_SEND_TIME){
            printf("��������ش���\n");
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


    //����
    ZeroMemory(buf, sizeof(buf));

    //���ļ�
    ifstream in;
    in.open(filename1, ios::in);
    if(!in.is_open()){
        printf("open file failed\n");
        return;
    }

    //���ļ�����ѭ��д�뻺����
    while(true){
        //����
        buf[1] = DATA; buf[3] = num;
        in.read(buf + 4, 512);
        streamsize bytesRead = in.gcount();
        if(bytesRead > 0){
            //�������ݰ�
            //�������ݰ�
            sendto(connectSocket, buf, bytesRead + 4, 0, (struct sockaddr*)&serverAddress, size);
        }else{
            printf("read file failed\n");
            return;
        }


        //����ACK
        while(true){
            if(sendNum > MAX_SEND_TIME){
                printf("��������ش���\n");
                return;
            }
            byteRead = recvfrom(connectSocket, buf, 30, 0, (struct sockaddr*)&serverAddress, &size);
            if(buf[3] == num) num++;
            if(byteRead == SOCKET_ERROR){
                sendto(connectSocket, buf, bytesRead + 4, 0, (struct sockaddr*)&serverAddress, size);
                sendNum++;
            }else break;
        }

        //�˳�ѭ��
        if(bytesRead < 512){
            break;
        }
    }
    //�ر��ļ�
    in.close();
    printf("send succeed!\n");
}



