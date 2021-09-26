#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define BUFF_SIZE 1024
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[])
{
   char buff_send[BUFF_SIZE + 1] = { 0 };
   char buff_receive[BUFF_SIZE + 1] = { 0 };
   SOCKET client;

   // 윈속 초기화
   WSADATA wsa;
   if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)   return -1;

   // socket()
   if ((client = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
   {

      return 1;
   }

   // 소켓 주소 구조체 초기화
   SOCKADDR_IN serveraddr;
   memset((void*)&serveraddr, 0x00, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET;
   serveraddr.sin_addr.s_addr = inet_addr("220.69.240.120");
   serveraddr.sin_port = htons(4000);

   if (connect(client, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
   {
      WSACleanup();
      return 1;
   }

   printf("서버에 연결되었습니다.");

   // 서버와 데이터 통신
   while (1)
   {
      // 데이터 입력
      back:;
      printf("\n[보낼 데이터] : ");
      if (fgets(buff_send, BUFF_SIZE + 1, stdin) == NULL) break;

      // '\n' 문자 제거
      if (buff_send[strlen(buff_send) - 1] == '\n') buff_send[strlen(buff_send) - 1] = '\0';   //엔터 제거
      if (strlen(buff_send) == 0) goto back;                                                   //다시 입력 받기

      // 데이터 보내기
      sendto(client, buff_send, strlen(buff_send), 0, (SOCKADDR*)&serveraddr, sizeof(serveraddr));

      // 데이터 받기
      recv(client, buff_receive, BUFF_SIZE, 0);

      // 받은 데이터 출력
      printf("[받은 데이터] : %s\n", buff_receive);

      if (buff_receive[0] == 'c')
      {
         // closesocket()
         closesocket(client);
         WSACleanup();
         exit(1);
      }
      memset(buff_receive, '\0', sizeof(buff_receive));             //입력 버퍼 초기화
   }

   return 0;
}