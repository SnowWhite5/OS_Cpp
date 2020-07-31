#define WIN32_LEAN_MEAN
#include <iostream>
#include<winsock2.h>
#include<windows.h>
#include<vector>
#include<algorithm>
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
vector<SOCKET> g_clients;
int process(SOCKET _csock)
{
    DataHeader header={};
    int nlen=recv(_csock,(char*)&header,sizeof(DataHeader),0);
    //DataHeader* header=(DataHeader*)szRecv;
    if(nlen<=0)
    {
        printf("client closed.\n");
        return -1;
    }
    switch (header.cmd)
    {
        case CMD_LOGIN:
        {
            Login login={};
            recv(_csock,(char*)&login+sizeof(DataHeader),sizeof(Login)-sizeof(DataHeader),0);
            printf("get from <sock=%d> cmd：%d length:%d,username=%s,password=%s\n",_csock,header.cmd,header.dataLength,
                login.userName,login.PassWord);
            LoginResult ret;
            ret.result=0;
            send(_csock,(char*)&ret,sizeof(LoginResult),0);
            }
        break;
        case CMD_LOGOUT:
        {
            Logout logout={};
            recv(_csock,(char*)&logout+sizeof(DataHeader),sizeof(Logout)-sizeof(DataHeader),0);
            printf("get from <sock=%d> cmd：%d length:%d\n",_csock,header.cmd,header.dataLength);
            LoginResult ret;
            ret.result=0;
            send(_csock,(char*)&ret,sizeof(LoginResult),0);
        }
        break;
        default:
            header.cmd=CMD_ERROR;
            header.dataLength=0;
            send(_csock,(char*)&header,sizeof(header),0);
        }
}
int main()
{
    WORD ver=MAKEWORD(2,2);
    WSADATA dat;
    WSAStartup(ver,&dat);
    //------------------
    //1.socket
    SOCKET _sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    //2.bind
    sockaddr_in _sin={};
    _sin.sin_family=AF_INET;
    _sin.sin_port=htons(4567);//host to net unsigned shprt
    //_sin.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");//连本机的
    //内网访问用127这个，所有都可以访问用INADDR_ANY
    //S_un是个联合体
    _sin.sin_addr.S_un.S_addr=inet_addr("127.0.0.1");//inet_addr("192.168.58.1");//连虚拟机的
    if(SOCKET_ERROR==bind(_sock,(sockaddr*)&_sin,(int)(sizeof(_sin))))//ockaddr不方便我们设置
        printf("ERROR,bind failed\n");
    //3.listen
    if(SOCKET_ERROR==listen(_sock,5))
        printf("ERROR,listen failed\n");

    char _recvBuf[128]={};
    while(true)
    {
        fd_set fdrd;
        FD_ZERO(&fdrd);
        FD_SET(_sock,&fdrd);
        for(auto c:g_clients) FD_SET(c,&fdrd);
        timeval t={1,0};//这样这儿就不会阻塞等待了
        int ret=select(_sock+1,&fdrd,NULL,NULL,&t);//第一个值是范围，不是数量，linux里很重要
        if(ret<0) break;
        if(FD_ISSET(_sock,&fdrd))
        {
            FD_CLR(_sock,&fdrd);
            //4.accept
            sockaddr_in clientaddr={};
            int nAddrlen=sizeof(sockaddr_in);
            SOCKET _csock=INVALID_SOCKET;
            _csock=accept(_sock,(sockaddr*)&clientaddr,&nAddrlen);
            if(INVALID_SOCKET==_csock)
                printf("error,accpet failed.\n");
            printf("new client %s.\n",inet_ntoa(clientaddr.sin_addr));
            for(auto c:g_clients)
            {

                NewUserJoin userjoin;
                userjoin.sock=_csock;
                if(send(c,(char*)&userjoin,sizeof(NewUserJoin),0)==-1) printf("aaa\n");
            }
            g_clients.push_back(_csock);
        }
        //5.处理请求
        for(int i=0;i<fdrd.fd_count;++i)
            if(process(fdrd.fd_array[i])==-1)
            {
                auto iter=find(g_clients.begin(),g_clients.end(),fdrd.fd_array[i]);
                if(iter!=g_clients.end())
                    g_clients.erase(iter);
            }
    }
    for(auto c:g_clients) closesocket(c);
    closesocket(_sock);
    WSACleanup();

    printf("client exit.\n");
    return 0;
}
