#include<iostream>
#include<algorithm>
#include<thread>
#include<atomic>
using namespace std;
atomic<int> key(0);
void mythread(){
    for(int i=0;i<1000000;++i)
    {
        key++;
        //key+=1;  //֧��
        //key =key + 1; //���󣬲�֧��ԭ�Ӳ���
    }
}

int main()
{
    thread t1(mythread);
    thread t2(mythread);
    t1.join();
    t2.join();
    cout<<"key:"<<key<<endl;
    return 0;
}

