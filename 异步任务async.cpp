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

    //future与shared_future
    future<int> t1=async(launch::async,mythread1); //立即创建新线程并启动
    //future<int> t1=async(launch::deferred,mythread1); //直到get,wait调用时，在main里执行，get_id时线程与main一样；不调用get,wait就不会执行
    //async第一个参数默认为launch::deferred | launch::async，本机默认用deferred
    //与thread不同的是，当资源紧张时，thread的创建会导致程序崩溃，但async会用deferred方式创建，即不创建新线程
    //cout<<"1 out in main:"<<t1.get()<<endl; //future的get函数为移动语义，调用完t1就空了

    //为了多线程中都能取到返回值，提出了shared_future
    shared_future<int> t2(move(t1)); //方法一
    //shared_future<int> t2(t1.share()); //方法二
    //shared_future<int> t2=async(launch::async,mythread1);  //方法三
    thread t3(mythread2,ref(t2));
    t3.join();
    cout<<"1 out in main:"<<t2.get()<<endl;  //shared_future的get函数为复制语义，可多次调用

    //promise,可在一个线程给其赋值，再在另一个线程中取出
    promise<int> myprom;
    thread t4(mythread3,ref(myprom));
    t4.join();
    future<int> ful=myprom.get_future();
    cout<<"myprom:"<<ful.get()<<endl;

    //packaged_task 把各种可调用对象打包起来，作为一个入口函数
    packaged_task<int(int,int)> pa(sum);
    future<int> ful2=pa.get_future();
    thread t5(move(pa),1,2);
    t5.join();
    cout<<"sum out:"<<ful2.get()<<endl;


    //其他
    //future_status 枚举类型，有ready,timeout,deferred等

    return 0;
}

