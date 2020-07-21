#include<iostream>
#include<algorithm>
#include<thread>
#include<future>
using namespace std;

int mythread1(){
    cout<<"thread1 id "<<this_thread::get_id()<<endl;
    return 5;
}
void mythread2(shared_future<int> &t){
    cout<<"thread2 id "<<this_thread::get_id()<<endl;
    cout<<"1 out:"<<t.get()<<endl;
}

void mythread3(promise<int> &t){
    cout<<"thread3 id "<<this_thread::get_id()<<endl;
    t.set_value(10);
}
int sum(int a,int b)
{
    cout<<"sum id "<<this_thread::get_id()<<endl;
    return a+b;
}
int main()
{

    cout<<"main id "<<this_thread::get_id()<<endl;

    //future��shared_future
    future<int> t1=async(launch::async,mythread1); //�����������̲߳�����
    //future<int> t1=async(launch::deferred,mythread1); //ֱ��get,wait����ʱ����main��ִ�У�get_idʱ�߳���mainһ����������get,wait�Ͳ���ִ��
    //async��һ������Ĭ��Ϊlaunch::deferred | launch::async������Ĭ����deferred
    //��thread��ͬ���ǣ�����Դ����ʱ��thread�Ĵ����ᵼ�³����������async����deferred��ʽ�����������������߳�
    //cout<<"1 out in main:"<<t1.get()<<endl; //future��get����Ϊ�ƶ����壬������t1�Ϳ���

    //Ϊ�˶��߳��ж���ȡ������ֵ�������shared_future
    shared_future<int> t2(move(t1)); //����һ
    //shared_future<int> t2(t1.share()); //������
    //shared_future<int> t2=async(launch::async,mythread1);  //������
    thread t3(mythread2,ref(t2));
    t3.join();
    cout<<"1 out in main:"<<t2.get()<<endl;  //shared_future��get����Ϊ�������壬�ɶ�ε���

    //promise,����һ���̸߳��丳ֵ��������һ���߳���ȡ��
    promise<int> myprom;
    thread t4(mythread3,ref(myprom));
    t4.join();
    future<int> ful=myprom.get_future();
    cout<<"myprom:"<<ful.get()<<endl;

    //packaged_task �Ѹ��ֿɵ��ö�������������Ϊһ����ں���
    packaged_task<int(int,int)> pa(sum);
    future<int> ful2=pa.get_future();
    thread t5(move(pa),1,2);
    t5.join();
    cout<<"sum out:"<<ful2.get()<<endl;


    //����
    //future_status ö�����ͣ���ready,timeout,deferred��

    return 0;
}

