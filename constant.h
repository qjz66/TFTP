//
// Created by qjz on 2023/9/4.
//


//opcode操作码
#define READ_REQUEST 1
#define WRITE_REQUEST 2
#define DATA 3
#define ACK 4
#define ERR 5

//mode传输文件数据类型
#define NET_ASCII "netascii"
#define OCTET "octet"

#define PORT_NUMBER 69
#define IP_ADDRESS "127.0.0.1"
#define OP_LEN 2
#define FILE_BYTES 516
#define QUIT 6
#define ACK_BYTES 4
#define MAX_SEND_TIME 6
