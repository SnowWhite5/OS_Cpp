//转自https://blog.csdn.net/hhypractise/article/details/107150434
//写者优先
//reader:
//p(mutex);    异步信号量1，保证多读者时写者优先
//p(book);     同步信号量1，保证读写申请的顺序
//p(rc_mutex); 异步信号量2，保证写计数变量的互斥访问
//if(readcnt==0) p(wrt); 同步信号量3，保证读写的顺序
//readcnt++;
//v(rc_mutex); 异步信号量2
//v(book);  同步信号量2
//v(mutex);  异步信号量1
//读....
//p(rc_mutex);异步信号量3
//readcnt--;
//if(readcnt==0) v(wrt); 同步信号量4
//v(rc_mutex);异步信号量3

//writer:
//p(wc_mutex);异步信号量4
//if(writecnt==0) p(book);同步信号量2
//writecnt++;
//v(wc_mutex);异步信号量4
//p(wrt);同步信号量4
//写....
//v(wrt);同步信号量3
//p(wc_mutex);异步信号量5
//writecnt--;
//if(writecnt==0) v(book);同步信号量1
//v(wc_mutex);异步信号量5

#include<iostream>
#include<string>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <io.h>
#include <string.h>
#include<algorithm>
#include<Windows.h> //多线程编程
#include<process.h>
using namespace std;

#define READER 'R'                   //读者
#define WRITER 'W'                   //写者
#define INTE_PER_SEC 1000            //每秒时钟中断的数目
#define MAX_THREAD_NUM 64            //最大线程数目

//变量声明初始化
int readercount = 0;//记录等待的读者数目
int writercount = 0;//记录等待的写者数目

HANDLE rc_mutex;//因为读者数量而添加的互斥信号量，用于读者优先

HANDLE rc2_mutex;//因为读者数量而添加的互斥信号量，用于写者优先
HANDLE wc_mutex;//因为写者数量而添加的互斥信号量
HANDLE book;//互斥访问信号量
HANDLE wrt;//保证每次只有一个写者进行写操作，当写者的数量writercount等于0的时候，则证明此时没有没有读者了,释放信号量book
HANDLE mutex;//避免写者同时与多个读者进行竞争，读者中信号量RWMutex比mutex3先释放，则一旦有写者，写者可马上获得资源

struct thread_info {
	int id;		      //线程序号
	char entity;      //线程类别(判断是读者线程还是写者线程)
	double delay;		 //线程延迟时间
	double lastTime;	 //线程读写操作时间
};
/*****************/
//读者优先
//进程管理-读者线程
void rp_threadReader(void *p)
{
	DWORD m_delay;                   //延迟时间
	DWORD m_persist;                 //读文件持续时间
	int m_serial;                    //线程序号
									 //从参数中获得信息
	m_serial = ((thread_info*)(p))->id;
	m_delay = (DWORD)(((thread_info*)(p))->delay *INTE_PER_SEC);
	m_persist = (DWORD)(((thread_info*)(p))->lastTime *INTE_PER_SEC);
	Sleep(m_delay);                  //延迟等待

	printf("读者进程%d申请读文件.\n", m_serial);
	//cout << "读者进程"<< m_serial<<"申请读文件." << endl;

	WaitForSingleObject(rc_mutex, -1);//对readercount互斥访问
	if (readercount == 0)WaitForSingleObject(book, -1);//第一位读者申请书
	readercount++;
	ReleaseSemaphore(rc_mutex, 1, NULL);//释放互斥信号量rc_mutex

	printf("读者进程%d开始读文件.\n", m_serial);
	Sleep(m_persist);
	printf("读者进程%d完成读文件.\n", m_serial);

	WaitForSingleObject(rc_mutex, -1);//修改readercount
	readercount--;//读者读完
	if (readercount == 0)ReleaseSemaphore(book, 1, NULL);//释放书籍，写者可写
	ReleaseSemaphore(rc_mutex, 1, NULL);//释放互斥信号量rc_mutex
}
/*****************/
//读者优先
//进程管理-写者线程
void rp_threadWriter(void *p)
{
	DWORD m_delay;                   //延迟时间
	DWORD m_persist;                 //读文件持续时间
	int m_serial;                    //线程序号
									 //从参数中获得信息
	m_serial = ((thread_info*)(p))->id;
	m_delay = (DWORD)(((thread_info*)(p))->delay *INTE_PER_SEC);
	m_persist = (DWORD)(((thread_info*)(p))->lastTime *INTE_PER_SEC);
	Sleep(m_delay);                  //延迟等待
	printf("写者进程%d申请写文件.\n", m_serial);
	WaitForSingleObject(book, INFINITE);//申请资源
	/*write is performed*/
	printf("写者进程%d开始读文件.\n", m_serial);
	Sleep(m_persist);
	printf("写者进程%d完成读文件.\n", m_serial);
	ReleaseSemaphore(book, 1, NULL);//释放资源
}
//读者优先
void ReaderPriority(char *file)
{
	DWORD n_thread = 0;           //线程数目
	DWORD thread_ID;            //线程ID
	DWORD wait_for_all;         //等待所有线程结束

								//创建信号量
	rc_mutex = CreateSemaphore(NULL, 1, 1, "mutex_for_readcount");//读者对count修改互斥信号量，初值为1,最大为1
	book = CreateSemaphore(NULL, 1, 1, NULL);//书籍互斥访问信号量，初值为1,最大值为1

	HANDLE h_Thread[MAX_THREAD_NUM];//线程句柄,线程对象的数组
	thread_info thread_info[MAX_THREAD_NUM];

	int id = 0;
	readercount = 0;               //初始化readcount
	ifstream inFile;
	inFile.open(file);
	cout << "读者优先:" << endl;
	while (inFile)
	{
		//读入每一个读者,写者的信息
		inFile >> thread_info[n_thread].id;
		inFile >> thread_info[n_thread].entity;
		inFile >> thread_info[n_thread].delay;
		inFile >> thread_info[n_thread++].lastTime;
		inFile.get();
	}
	for (int i = 0; i<(int)(n_thread); i++)
	{
		if (thread_info[i].entity == READER || thread_info[i].entity == 'r')
		{
			//创建读者进程
			h_Thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(rp_threadReader), &thread_info[i], 0, &thread_ID);
		}
		else
		{
			//创建写线程
			h_Thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(rp_threadWriter), &thread_info[i], 0, &thread_ID);
		}
	}
	//等待子线程结束

	//关闭句柄
	wait_for_all = WaitForMultipleObjects(n_thread, h_Thread, TRUE, -1);
	cout << endl;
	cout << "所有读者写者已经完成操作！！" << endl;
	for(int i = 0; i<(int)(n_thread); i++)
		CloseHandle(h_Thread[i]);
	CloseHandle(rc_mutex);
	CloseHandle(book);
}
/*****************/
//写者优先
//进程管理-读者线程
void wp_threadReader(void *p) {
	DWORD m_delay;                   //延迟时间
	DWORD m_persist;                 //读文件持续时间
	int m_serial;                    //线程序号
									 //从参数中获得信息
	m_serial = ((thread_info*)(p))->id;
	m_delay = (DWORD)(((thread_info*)(p))->delay *INTE_PER_SEC);
	m_persist = (DWORD)(((thread_info*)(p))->lastTime *INTE_PER_SEC);
	Sleep(m_delay);                  //延迟等待

	printf("读者进程%d申请读文件.\n", m_serial);
	WaitForSingleObject(mutex, -1);
	WaitForSingleObject(book, -1);
	WaitForSingleObject(rc2_mutex, -1);//对readercount互斥访问
	if (readercount == 0)WaitForSingleObject(wrt, -1);//第一位读者申请书,同时防止写者进行写操作
	readercount++;

	ReleaseSemaphore(rc2_mutex, 1, NULL);//释放互斥信号量rc_mutex
	ReleaseSemaphore(book, 1, NULL);//释放互斥信号量book
	ReleaseSemaphore(mutex, 1, NULL);//释放互斥信号量mutex
									 /* reading is performed */
	printf("读者进程%d开始读文件.\n", m_serial);
	Sleep(m_persist);
	printf("读者进程%d完成读文件.\n", m_serial);
	WaitForSingleObject(rc2_mutex, -1);//修改readercount
	readercount--;//读者读完
	if (readercount == 0)ReleaseSemaphore(wrt, 1, NULL);//释放资源，写者可写
	ReleaseSemaphore(rc2_mutex, 1, NULL);//释放互斥信号量rc_mutex
}
/*****************/
//写者优先
//进程管理-写者线程
void wp_threadWriter(void *p) {
	DWORD m_delay;                   //延迟时间
	DWORD m_persist;                 //读文件持续时间
	int m_serial;                    //线程序号
									 //从参数中获得信息
	m_serial = ((thread_info*)(p))->id;
	m_delay = (DWORD)(((thread_info*)(p))->delay *INTE_PER_SEC);
	m_persist = (DWORD)(((thread_info*)(p))->lastTime *INTE_PER_SEC);
	Sleep(m_delay);                  //延迟等待

	printf("写者进程%d申请写文件.\n", m_serial);
	WaitForSingleObject(wc_mutex, -1);//对writercount互斥访问
	if (writercount == 0)WaitForSingleObject(book, -1);//第一位写者申请资源
	writercount++;
	ReleaseSemaphore(wc_mutex, 1, NULL);//释放资源

	WaitForSingleObject(wrt, -1);
	/*write is performed*/
	printf("写者进程%d开始写文件.\n", m_serial);
	Sleep(m_persist);
	printf("写者进程%d完成写文件.\n", m_serial);
	ReleaseSemaphore(wrt, 1, NULL);//释放资源

	WaitForSingleObject(wc_mutex, -1);//对writercount互斥访问
	writercount--;
	if (writercount == 0)ReleaseSemaphore(book, 1, NULL);//释放资源
	ReleaseSemaphore(wc_mutex, 1, NULL);//释放资源
}
//写者优先
void WriterPriority(char *file) {
	DWORD n_thread = 0;           //线程数目
	DWORD thread_ID;            //线程ID
	DWORD wait_for_all;         //等待所有线程结束

								//创建信号量
	rc2_mutex = CreateSemaphore(NULL, 1, 1, "mutex_for_readercount");//读者对count修改互斥信号量，初值为1,最大为1
	wc_mutex = CreateSemaphore(NULL, 1, 1, "mutex_for_writercount");//写者对count修改互斥信号量，初值为1,最大为1
	wrt = CreateSemaphore(NULL, 1, 1, NULL);//
	mutex = CreateSemaphore(NULL, 1, 1, NULL);//
	book = CreateSemaphore(NULL, 1, 1, NULL);//书籍互斥访问信号量，初值为1,最大值为1

	HANDLE h_Thread[MAX_THREAD_NUM];//线程句柄,线程对象的数组
	thread_info thread_info[MAX_THREAD_NUM];

	int id = 0;
	readercount = 0;               //初始化readcount
	writercount = 0;
	ifstream inFile;
	inFile.open(file);
	cout << "写者优先:" << endl;
	while (inFile)
	{
		//读入每一个读者,写者的信息
		inFile >> thread_info[n_thread].id;
		inFile >> thread_info[n_thread].entity;
		inFile >> thread_info[n_thread].delay;
		inFile >> thread_info[n_thread++].lastTime;
		inFile.get();
	}
	for (int i = 0; i<(int)(n_thread); i++)
	{
		if (thread_info[i].entity == READER || thread_info[i].entity == 'r')
		{
			//创建读者进程
			h_Thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(wp_threadReader), &thread_info[i], 0, &thread_ID);
		}
		else
		{
			//创建写线程
			h_Thread[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)(wp_threadWriter), &thread_info[i], 0, &thread_ID);
		}
	}
	//等待子线程结束

	//关闭句柄
	wait_for_all = WaitForMultipleObjects(n_thread, h_Thread, TRUE, -1);
	cout << endl;
	cout << "所有读者写者已经完成操作！！" << endl;
	for (int i = 0; i<(int)(n_thread); i++)
		CloseHandle(h_Thread[i]);
	CloseHandle(wc_mutex);
	CloseHandle(rc2_mutex);
	CloseHandle(book);
}
//主函数

int main()
{
	char choice;
	cout << "    欢迎进入读者写者模拟程序    " << endl;
	while (true)
	{
		//打印提示信息
		cout << "     请输入你的选择     " << endl;
		cout << "     1、读者优先" << endl;
		cout << "     2、写者优先" << endl;
		cout << "     3、退出程序" << endl;
		cout << endl;
		//如果输入信息不正确，继续输入
		do {
			choice = (char)_getch();
		} while (choice != '1'&&choice != '2'&&choice != '3');

		system("cls");
		//选择1，读者优先
		if (choice == '1')
			//ReaderPriority("thread.txt");
			ReaderPriority(const_cast<char *>("thread.txt"));
		//选择2，写者优先
		else if (choice == '2')
			WriterPriority(const_cast<char *>("thread.txt"));
		//选择3，退出
		else
			return 0;
		//结束
		printf("\nPress Any Key to Coutinue");
		_getch();
		system("cls");
	}
	//return 0;
}
