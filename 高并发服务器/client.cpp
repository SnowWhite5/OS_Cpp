#define WIN32_LEAN_MEAN
#include <iostream>
#include<winsock2.h>
#include<windows.h>
#include<winbase.h>
#include<thread>
//#pragma comment(lib,"E:\CodeBlocks\workspace\ws2_32.lib")
using namespace std;

enum CMD{
    CMD_LOGIN,
    CMD_LOGINRESULT,
    CMD_LOGOUT,
    CMD_LOGOUTRESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};
//消息头
struct DataHeader{
    short cmd;
    short dataLength;
};
struct Login: public DataHeader
{
    Login()
    {
        dataLength=sizeof(Login);
        cmd=CMD_LOGIN;
    }
    char userName[32];
    char PassWord[32];
};
struct LoginResult:public DataHeader
{
    LoginResult()
    {
        dataLength=sizeof(LoginResult);
        cmd=CMD_LOGINRESULT;
        result=1;
    }
    int result;

};
struct Logout:public DataHeader
{
    Logout()
    {
        dataLength=sizeof(Logout);
        cmd=CMD_LOGOUT;
    }
    char userName[32];
};
struct LogoutResult:public DataHeader
{
    LogoutResult()
    {
        dataLength=sizeof(LogoutResult);
        cmd=CMD_LOGOUTRESULT;
        result=1;
    }
    int result;
};
struct NewUserJoin:public DataHeader
{
    NewUserJoin()
    {
        dataLength=sizeof(LogoutResult);
        cmd=CMD_NEW_USER_JOIN;
        sock=0;
    }
    int sock;
};

int process(SOCKET _csock)
{

            //5.接受客户端数据
    DataHeader header={};
    int nlen=recv(_csock,(char*)&header,sizeof(DataHeader),0);
    //DataHeader* header=(DataHeader*)szRecv;
    if(nlen<=0)
    {
        printf("server closed.\n");
        return -1;
    }
    switch (header.cmd)
        {
        case CMD_LOGINRESULT:
            {
                LoginResult login={};
                recv(_csock,(char*)&login+sizeof(DataHeader),sizeof(LoginResult)-sizeof(DataHeader),0);
                printf("收到服务器端消息为:%d\n",login.result);

            }
            break;
        case CMD_LOGOUTRESULT:
            {
                LogoutResult logout={};
                recv(_csock,(char*)&logout+sizeof(DataHeader),sizeof(LogoutResult)-sizeof(DataHeader),0);
                printf("收到服务器端消息为:%d\n",logout.result);
            }
            break;
        case CMD_NEW_USER_JOIN:
            {
                NewUserJoin userjoin={};
                recv(_csock,(char*)&userjoin+sizeof(DataHeader),sizeof(NewUserJoin)-sizeof(DataHeader),0);
                printf("新加入的伙伴是<sock=%d>\n",userjoin.sock);
            }
        }


}
bool f=true;
void CMDTread(int _sock)
{

    char clientBuf[256]={};
    while(true)
    {
            scanf("%s",clientBuf);

        //4.处理请求
        if(0==strcmp(clientBuf,"exit"))
        {
            f=false;
            printf("client exit\n");
            break;
        }

        else if(0==strcmp(clientBuf,"login"))
            {
                Login login;
                strcpy(login.userName,"aaa");
                strcpy(login.PassWord,"123");
                login.cmd=CMD_LOGIN;
                login.dataLength=sizeof(login)-sizeof(DataHeader);
                send(_sock,(char*)&login,sizeof(Login),0);
            }
            else if(0==strcmp(clientBuf,"logout"))
            {
                Logout logout;
                strcpy(logout.userName,"aaa");
                logout.cmd=CMD_LOGOUT;
                logout.dataLength=sizeof(Logout)-sizeof(DataHeader);
                send(_sock,(char*)&logout,sizeof(Logout),0);
                //接受服务器返回数据


            }
            else
                printf("不支持的命令\n");
    }
    //cout<<f<<endl;
    //return;
}
int main()
{
    WORD ver=MAKEWORD(2,2);
    WSADATA dat;
    WSAStartup(ver,&dat);
    //------------------
    SOCKET _sock=socket(AF_INET,SOCK_STREAM,0);
    if(INVALID_SOCKET==_sock)
        printf(" socket failed.\n");
    sockaddr_in _sin={};
    _sin.sin_family=AF_INET;
    _sin.sin_port=htons(4567);
    _sin.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");//inet_addr("192.168.58.129");//
    if(SOCKET_ERROR==connect(_sock+1,(sockaddr*)&_sin,sizeof(sockaddr_in)))
        printf("connect failed.\n");
    char clientBuf[128]={};
    thread t1(CMDTread,_sock);
    t1.detach();
    while(f)
    {
        //printf("hahaha\n");

        fd_set fdrd;
        FD_ZERO(&fdrd);
        FD_SET(_sock,&fdrd);
        timeval t={1,0};
        int ret=select(_sock,&fdrd,NULL,NULL,&t);
        if(ret<0)
            break;
        if(FD_ISSET(_sock,&fdrd))
        {
            FD_CLR(_sock,&fdrd);
            if(process(_sock)==-1) break;
        }
        //t1.join();
        //Sleep(1000);

    }
    closesocket(_sock);
    WSACleanup();
    printf("client exit.\n");
    //getchar();
    return 0;
}

