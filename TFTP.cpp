#include <iostream>
#include "winsock2.h"
#include "constant.h"
#include "util.h"

using namespace std;



int main(){
    //��ʼ��
    bool judge = init();
    if(judge){
        printf("WSA not initialised,%d\n",WSAGetLastError());
        WSACleanup();
    }

    //�����������׽���
    SOCKET connectSocket = get_socket();
    if(connectSocket == INVALID_SOCKET){
        printf("get socket failed:%d\n",WSAGetLastError());
    }else
        printf("get socket success\n");

    // ���ý��պͷ��ͳ�ʱʱ�䣨�Ժ���Ϊ��λ��
    int timeout = 3000; // ����Ϊ3��
    if (setsockopt(connectSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "�޷����ý��ճ�ʱ: " << strerror(WSAGetLastError()) << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    /*if (setsockopt(connectSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
        std::cerr << "�޷����÷��ͳ�ʱ: " << strerror(WSAGetLastError()) << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }*/


    //���ӷ����

    while(true){
        //����mode type
        short mode;
        string type;
        printf("��������:1\n��������:2\n�˳�����:6 ");
        cin >> mode;
        if(mode == QUIT){
            closesocket(connectSocket);
            WSACleanup();
            break;
        }
        printf("��ѡ�����ݷ��ͷ�ʽ:octet/netascii ");
        cin >> type;
        cout << type;

        if(mode == 1)
            RRQ(type, connectSocket);
        else
            WRQ(type, connectSocket);
    }
    return 0;
}
