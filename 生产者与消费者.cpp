#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <condition_variable>
#include<windows.h>
using namespace std;

mutex mtx;
condition_variable produce, consume;  // 条件变量是一种同步机制，要和mutex以及lock一起使用

queue<int> q;     // shared value by producers and consumers, which is the critical section
int maxSize = 20;

void consumer()
{
    while (true)
    {
        
        chrono::milliseconds dura(1000);
        this_thread::sleep_for(dura);
        unique_lock<mutex> lck(mtx);
        while(q.size()==0)
        {
            consume.wait(lck);             //condition_variable.wait()锁至满足while条件不满足
        }
        //consume.wait(lck, [] {return q.size() != 0; });     // 跟上一句等价，wait(block) consumer until q.size() != 0 is true

        cout << "consumer " << this_thread::get_id() << ": ";
        q.pop();
        cout << q.size() << '\n';

        produce.notify_all();                               // nodity(wake up) producer when q.size() != maxSize is true
       lck.unlock();
    }
}

void producer(int id)
{
    while (true)
    {
        //this_thread::sleep_for(chrono::milliseconds(900));      // producer is a little faster than consumer
        //sleep(1);//
        chrono::milliseconds dura(1000);
        this_thread::sleep_for(dura);
        unique_lock<mutex> lck(mtx);
        while(q.size() == maxSize)
        {
            produce.wait(lck);
        }
       // produce.wait(lck, [] {return q.size() != maxSize; });   // wait(block) producer until q.size() != maxSize is true

        cout << "-> producer " << this_thread::get_id() << ": ";
        q.push(id);
        cout << q.size() << '\n';

        consume.notify_all();                                   // notify(wake up) consumer when q.size() != 0 is true
        lck.unlock();
    }
}

int main()
{
    thread consumers[2], producers[2];

    // spawn 2 consumers and 2 producers:
    for (int i = 0; i < 2; ++i)
    {
        consumers[i] = thread(consumer);
        producers[i] = thread(producer, i + 1);  //thread：第一个参数是task任务，第二个参数是task函数的参数
    }

    // join them back: (in this program, never join...)
    for (int i = 0; i < 2; ++i)
    {
        producers[i].join();
        consumers[i].join();
    }

    system("pause");
    return 0;
}
