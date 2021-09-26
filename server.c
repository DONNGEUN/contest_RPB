#define BUFF_SIZE 1024 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iconv.h>

int fre = 400;
int t = 1;

int server_socket;              //서버 디스크립터
int client_socket;              //클라이언트 디스크립터
int client_addr_size,option;
struct sockaddr_in server_addr; //소켓에 주소와 포트를 할당하기 위해 struct 구조체 이용
struct sockaddr_in client_addr;
char buff_receive[BUFF_SIZE+1];
char buff_send[BUFF_SIZE+1];
const char *aw[24] = {"을/를 받았습니다."};

void data_setting()
{
	switch(buff_send[0])
	{
		case 'w':                     // 전진
			digitalWrite(24, 0);
			digitalWrite(25, 1);
			digitalWrite(27, 0);
			digitalWrite(28 ,0);
			pwmWrite(1, 800);           //HARDWARE PWM 출력
			pwmWrite(26, 800);
			printf("%d\n", fre);
		break;
		
		case 's':                     // 후진
			digitalWrite(24, 1);
			digitalWrite(25, 0);
			digitalWrite(27, 0);
			digitalWrite(28 ,0);
			pwmWrite(1, 800);           //HARDWARE PWM 출력
			pwmWrite(26, 800);
			printf("%d\n", fre);

		break;
		
		case 'x':                     // 정지 / 정지해제
			if (t == 1)
			{
			   pwmWrite(1, 0);           //HARDWARE PWM 출력
			   digitalWrite(27, 1);
			   digitalWrite(28, 1);
			}
			else 	
			{
			   pwmWrite(1, 800);           //HARDWARE PWM 출력
			   digitalWrite(27, 0);	
			   digitalWrite(28, 0);

			}
			t = ~t;
		break;

		case 'o':                     // 속도 감소
			fre += 50;
			pwmSetClock(fre);
		     
			printf("%d\n", fre);
		break;

		case 'p':                     // 속도 증가
			fre -= 50;
			pwmSetClock(fre);
			printf("%d\n", fre);
		break;

		default :
		break;
	}
}

void client_connecting()
{
   //클라이언트 접속 요청 확인
   if( -1 == listen(server_socket,5)) 
   {
      printf("listen error\n");
      exit(1);
   }

   client_addr_size = sizeof(client_addr); //client 주소 크기 대입
   client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
   //accept()함수는 클라이언트의 접속 요청을 받아드리고 클라이언트와 통신하는 전용 소켓을 생성합니다
   //accept()로 접속요청을 허락시, 클라이언트와 통신을 하기 위해 커널이 자동으로 소켓 생성= client_socket

   if(-1 == client_socket) 
   {   
      printf("클라이언트 연결 수락 실패\n");
      exit(1);
   }
   else printf("클라이언트가 연결되었습니다.\n");
}

void tran(char mod) 
{
   iconv_t it;
   size_t in_size, out_size;
   char *input_buf_ptr = buff_receive;
   char *output_buf_ptr = buff_send;
      
   in_size = strlen(buff_receive);
   out_size = sizeof(buff_send);
   
   switch(mod)
   {
      case 1:  // EUC-KR을 UTF-8로 변환
      {
         it = iconv_open("UTF-8", "EUC-KR");
         iconv(it,&input_buf_ptr, &in_size, &output_buf_ptr, &out_size);
    
         iconv_close(it);
         break;
      }
      
      case 2:  // UTF-8을 EUC-KR로 변환
      {
         it = iconv_open("EUC-KR", "UTF-8");
         iconv(it,&output_buf_ptr, &out_size, &output_buf_ptr, &out_size);
                
         iconv_close(it);
         break;
      }
   }
}

int main(void)
{
    if(wiringPiSetup()==-1) return -1;
   
      pwmSetClock(fre);
      pinMode(1, PWM_OUTPUT); //HARDWARE PWM
      pinMode(24, OUTPUT);     //Dir1
      pinMode(25, OUTPUT);     //Dir2
      pinMode(6, OUTPUT);     //magnetic
      pinMode(26, PWM_OUTPUT); //HARDWARE PWM
      pinMode(27, OUTPUT); //EN_MOTOR_1
      pinMode(28, OUTPUT); //EN_MOTOR_2
	
   
	pwmSetRange(1024);         //19.2MHz / PWM클럭 / 분해능 = 주기, 19.2M / fre / 1024 = 9.76Hz
	pwmWrite(1, 0);           //HARDWARE PWM off
	pwmWrite(26, 0);
	
   //소켓 생성
   server_socket = socket(PF_INET, SOCK_STREAM, 0);    //TCP/IP에서는 SOCK_STREAM 을 UDP/IP에서는 SOCK_DGRAM을 사용
   if(-1 == server_socket) 
   {
      printf("socket() error\n"); 
      exit(1); //오류 확인
   }
   else printf("서버 오픈\n");
   
   memset(&server_addr, 0, sizeof(server_addr)); //메모리의 크기를 변경할 포인터, 초기화 값, 초기화 길이
   server_addr.sin_family = AF_INET;   //IPv4 인터넷 프로토콜
   server_addr.sin_port = htons(4000); //사용할 port 번호 4000
   server_addr.sin_addr.s_addr= inet_addr("192.168.0.35"); //32bit IPv4주소
   
   //소켓의 바인드 오류를 해결해주기위한 함수
   char option = 1;
   setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

   //bind()함수를 이용하여 소켓에 server socket에 필요한 정보를 할당하고 커널에 등록
   if(-1 == bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
   {
      printf("bind error\n");
      exit(1);
   }
   
   client_connecting();

   while(1)
   {
		if(-1 == client_socket)
        {   
			printf("클라이언트와의 연결이 끊어졌습니다.\n 재연결을 시도합니다.\n");
			client_connecting();
        }
      
      else
      {
        read(client_socket, buff_receive, BUFF_SIZE);                 //컴퓨터의 지시 받기
        tran(1);                                                      //EUC-KR을 UTF-8로 변환(buff_send에 저장)
		
		printf("[client] : %s\n", buff_send);                         //받은 지시 수행 확인
		data_setting();                                               //지시 수행
        
        strcat(buff_send, *aw);                                       //확인 답장 만들기
        tran(2);                                                      //UTF-8을 EUC-KR로 변환
        
        write(client_socket, buff_send, strlen(buff_send));           //받은 문자 확인 답장 보내기
        
        if (buff_receive[0] == 'c')                                   //c문자를 받으면 소켓 닫기
        {
           // closesocket()
           close(client_socket);
           client_socket = -1;
        }
        
        memset(buff_receive, '\0', sizeof(buff_receive));             //버퍼 초기화
        memset(buff_send, '\0', sizeof(buff_send));                   //버퍼 초기화
      }
   }
   
   return 0;
}
