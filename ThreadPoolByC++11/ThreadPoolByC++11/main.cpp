#include <iostream>
#include <string>
#include <initializer_list>
#include <mutex>
#include <future>
#include <queue>

using namespace std;

void OutPutStrings(std::initializer_list<std::string> list)
{
	for (auto beg = list.begin(); beg != list.end(); beg++)
	{
		std::cout << *beg << std::endl;
	}
}

int myThread()
{
	cout << "mythread() start thread id = " << this_thread::get_id() << endl;
	std::chrono::milliseconds dura(2000);
	std::this_thread::sleep_for(dura);
	cout << "mythread() end thread id = " << this_thread::get_id() << endl;
	return 5;
}

class A
{
public:
	int myAThread(int iPut)
	{
		cout << "mythread() start thread id = " << this_thread::get_id() << endl;
		std::chrono::milliseconds dura(2000);
		std::this_thread::sleep_for(dura);
		cout << "mythread() end thread id = " << this_thread::get_id() << endl;
		return iPut;
	}
};

int myThreadPacket(int nInpt)
{
	cout << "mythread() start thread id = " << this_thread::get_id() << endl;
	std::chrono::milliseconds dura(2000);
	std::this_thread::sleep_for(dura);
	cout << "mythread() end thread id = " << this_thread::get_id() << endl;
	return nInpt;
}

void myThreadPromise(std::promise<int> &tmp, int calc)
{
	//calc
	calc++;
	calc *= 10;
	std::chrono::milliseconds dura(3000);
	std::this_thread::sleep_for(dura);


	int ret = calc;
	tmp.set_value(ret);
}

void myThreadPromise2(std::future<int> &tmp)
{
	cout << "myThreadPromise2 = " << tmp.get() << endl;
}

std::atomic<bool> m_ifEnd = false; //原子操作
void myThreadBool()
{
	std::chrono::milliseconds dar(1000);
	while (m_ifEnd == false)
	{
		//...
		cout << "Thread id =" << std::this_thread::get_id() << endl;
		std::this_thread::sleep_for(dar);
	}

	cout << "Thread id =" << std::this_thread::get_id() << " end" << endl;
}

//class MyThreadPool
void  myThreadPoolFun()
{
	cout << "myThreadPoolFun start thread id = " << this_thread::get_id() << endl;
	std::chrono::milliseconds dura(200);
	std::this_thread::sleep_for(dura);
	cout << "myThreadPoolFun end thread id = " << this_thread::get_id() << endl;
}

class NoCopyable
{
private:
	NoCopyable(const NoCopyable  &n) = delete;
	NoCopyable& operator= (const NoCopyable  &n) = delete;

public:
	NoCopyable() = default;
	~NoCopyable() = default;
};

class MyThreadPool : public NoCopyable
{
	typedef std::function<void()> Task;

public:
	explicit MyThreadPool()
	{
	}

	~MyThreadPool()
	{
		if (m_runing)
		{
			Stop();
		}
	}

	void Start(int numThreads)
	{
		m_runing = true;
		m_works.reserve(numThreads);
		//初始化线程
		for (int i = 0; i < numThreads; i++)
		{
			m_works.emplace_back(thread(&MyThreadPool::RunInThread, this));
		}
	}

	void Stop()
	{
		{
			unique_lock<mutex> lock(m_mutex);
			m_runing = false;
			m_condition.notify_all();
		}

		for (std::size_t i = 0; i < m_works.size(); i++)
		{
			m_works[i].join();
		}
	}

	void Run(const Task& task)  //任务插入线程池
	{
		if (m_works.empty())
		{
			task();
		}
		else
		{
			unique_lock<mutex> lock(m_mutex);
			m_funs.emplace(task);
			m_condition.notify_one();
		}
	}

private:
	void RunInThread() //线程池中每个任务执行Function
	{
		while (m_runing)
		{
			//cout << "Begin Get One tack thread is = " << this_thread::get_id() << endl;
			Task task = GetTack();
			//cout << "End Get One tack thread is = " << this_thread::get_id() << endl;
			if (task)
			{
				task();
			}
		}
	}

	Task GetTack()     //从队列中取出一个任务
	{
		unique_lock<mutex> lock(m_mutex);
		while (m_funs.empty() && m_runing)
		{
			m_condition.wait(lock);
		}

		Task task;
		if (!m_funs.empty())
		{
			task = m_funs.front();
			m_funs.pop();
		}

		return task;
	}

private:
	std::vector<std::thread> m_works;        //工作线程队列
	std::queue<Task> m_funs;                 //任务队列
	std::condition_variable m_condition;
	std::mutex  m_mutex;
	bool m_runing;
};

int main()
{

	//测试线程池
	cout << "---------------------" << "Begin to Thread Pool" << "--------------------" << endl;
	MyThreadPool myThreadPool;
	myThreadPool.Start(3);
	for (int i = 0; i < 20; i++)
	{
		auto task = std::bind(myThreadPoolFun);
		myThreadPool.Run(task);
	}
	cout << "---------------------" << "End to Thread Pool" << "--------------------" << endl;
	std::chrono::milliseconds darThreadPool(1000);
	std::this_thread::sleep_for(darThreadPool);

	//OutPutStrings({ "hello", "every", "one!" });
	cout << "main thread start thread id = " << this_thread::get_id() << endl;
	//std::async 函数模板 用来启动一个异步任务  std::future 将来的意思，提供一种访问异步结果的机制 
	//-------***********-------------
	std::future<int> ret = std::async(myThread);  //创建线程 并开始执行
	cout << "continue ...!" << endl;
	cout << ret.get() << endl;  // 卡这等待直到 myThread()线程 拿到结果
	//cout << ret.get() << endl;  // 只能调用一次，不能调用多次，不然会有问题
	//ret.wait();  //只是等待线程，没有返回结果
	//std::launch::deferred 延迟调用等待在主线程执行，不创建新线程  std::launch::async 系统默认，不用传参
	//-------***********-------------

	//类成员函数初始化
	//-------***********-------------
	cout << "Begin to other test" << endl;
	A a;
	int nInput = 12;
	std::future<int> reta = std::async(/*std::launch::deferred,*/ &A::myAThread, &a, nInput);  //创建线程 并开始执行  第二个参数是类型引用，才能保证线程用的是同一个对象
	cout << "continue ...!" << endl;
	cout << reta.get() << endl;
	//-------***********-------------

	//std::packaged_task: 打包任务，把任务打包起来
	//是个类模板，可以把各种调用对象包装起来，方便将来作为线程调用起来
	//-------***********-------------
	cout << "Begin to other test" << endl;
	std::packaged_task<int(int)> mypt(myThreadPacket);  //我们把函数myThreadPacket 通过packaged_task包装起来可以不通过线程直接调用
	//std::packaged_task<int(int)> mypt([]int);  //我们把函数myThreadPacket 通过packaged_task包装起来 用lambe表达式
	//thread t1(std::ref(mypt), 1); //线程直接开始，第二个参数作为线程入口参数
	//t1.join();//等待线程执行完毕
	mypt(10); //直接调用
	std::future<int> rstPack = mypt.get_future();
	rstPack.wait_for(10s);
	cout << "rstPack = " << rstPack.get() << endl;
	//-------***********-------------


	//std::promise  获取线程中的值  通过promise保存一个值，我们可以通过future绑定到promise上来获取将来的一个值,传到别的线程中 实现两个线程直接数据传递
	//-------***********-------------
	cout << "Begin to other test" << endl;
	std::promise<int> myPro; //声明对象 保存类型为int
	thread t2(myThreadPromise, std::ref(myPro), 100);
	t2.join();

	std::future<int> rstPro = myPro.get_future();
	thread t3(myThreadPromise2, std::ref(rstPro));
	t3.join();

	//cout <<  "rstPro = " << rstPro.get() << endl;
	//-------***********-------------


	//-------***********-------------
	cout << "Begin to other test" << endl;
	//std::future_status  状态标志位  枚举
	//std::shared_future  分享值  可以get多次
	std::shared_future<int> fu_s(rstPro.share());
	//std::atomic(类模板) 原子操作  不需要互斥量加锁（无锁）技术的多线程并发编程。  
	//多线程中不会被打断  原子操作比互斥量更有技术效率更好 针对一个变量的运算读写，而非代码块
	//一般是指"不可分割的操作"
	thread tdMy1(myThreadBool);
	thread tdMy2(myThreadBool);

	std::chrono::milliseconds dar(5000);
	std::this_thread::sleep_for(dar);
	m_ifEnd = true;
	tdMy1.join();
	tdMy2.join();
	//-------***********-------------

	cout << "I Love China" << endl;
	getchar();
	return 0; //结束时候不调用wait 或者 get 依然会等待线程执行完毕
}