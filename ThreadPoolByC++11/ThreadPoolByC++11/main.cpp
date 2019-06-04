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

std::atomic<bool> m_ifEnd = false; //ԭ�Ӳ���
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
		//��ʼ���߳�
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

	void Run(const Task& task)  //��������̳߳�
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
	void RunInThread() //�̳߳���ÿ������ִ��Function
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

	Task GetTack()     //�Ӷ�����ȡ��һ������
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
	std::vector<std::thread> m_works;        //�����̶߳���
	std::queue<Task> m_funs;                 //�������
	std::condition_variable m_condition;
	std::mutex  m_mutex;
	bool m_runing;
};

int main()
{

	//�����̳߳�
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
	//std::async ����ģ�� ��������һ���첽����  std::future ��������˼���ṩһ�ַ����첽����Ļ��� 
	//-------***********-------------
	std::future<int> ret = std::async(myThread);  //�����߳� ����ʼִ��
	cout << "continue ...!" << endl;
	cout << ret.get() << endl;  // ����ȴ�ֱ�� myThread()�߳� �õ����
	//cout << ret.get() << endl;  // ֻ�ܵ���һ�Σ����ܵ��ö�Σ���Ȼ��������
	//ret.wait();  //ֻ�ǵȴ��̣߳�û�з��ؽ��
	//std::launch::deferred �ӳٵ��õȴ������߳�ִ�У����������߳�  std::launch::async ϵͳĬ�ϣ����ô���
	//-------***********-------------

	//���Ա������ʼ��
	//-------***********-------------
	cout << "Begin to other test" << endl;
	A a;
	int nInput = 12;
	std::future<int> reta = std::async(/*std::launch::deferred,*/ &A::myAThread, &a, nInput);  //�����߳� ����ʼִ��  �ڶ����������������ã����ܱ�֤�߳��õ���ͬһ������
	cout << "continue ...!" << endl;
	cout << reta.get() << endl;
	//-------***********-------------

	//std::packaged_task: ������񣬰�����������
	//�Ǹ���ģ�壬���԰Ѹ��ֵ��ö����װ���������㽫����Ϊ�̵߳�������
	//-------***********-------------
	cout << "Begin to other test" << endl;
	std::packaged_task<int(int)> mypt(myThreadPacket);  //���ǰѺ���myThreadPacket ͨ��packaged_task��װ�������Բ�ͨ���߳�ֱ�ӵ���
	//std::packaged_task<int(int)> mypt([]int);  //���ǰѺ���myThreadPacket ͨ��packaged_task��װ���� ��lambe���ʽ
	//thread t1(std::ref(mypt), 1); //�߳�ֱ�ӿ�ʼ���ڶ���������Ϊ�߳���ڲ���
	//t1.join();//�ȴ��߳�ִ�����
	mypt(10); //ֱ�ӵ���
	std::future<int> rstPack = mypt.get_future();
	rstPack.wait_for(10s);
	cout << "rstPack = " << rstPack.get() << endl;
	//-------***********-------------


	//std::promise  ��ȡ�߳��е�ֵ  ͨ��promise����һ��ֵ�����ǿ���ͨ��future�󶨵�promise������ȡ������һ��ֵ,��������߳��� ʵ�������߳�ֱ�����ݴ���
	//-------***********-------------
	cout << "Begin to other test" << endl;
	std::promise<int> myPro; //�������� ��������Ϊint
	thread t2(myThreadPromise, std::ref(myPro), 100);
	t2.join();

	std::future<int> rstPro = myPro.get_future();
	thread t3(myThreadPromise2, std::ref(rstPro));
	t3.join();

	//cout <<  "rstPro = " << rstPro.get() << endl;
	//-------***********-------------


	//-------***********-------------
	cout << "Begin to other test" << endl;
	//std::future_status  ״̬��־λ  ö��
	//std::shared_future  ����ֵ  ����get���
	std::shared_future<int> fu_s(rstPro.share());
	//std::atomic(��ģ��) ԭ�Ӳ���  ����Ҫ�����������������������Ķ��̲߳�����̡�  
	//���߳��в��ᱻ���  ԭ�Ӳ����Ȼ��������м���Ч�ʸ��� ���һ�������������д�����Ǵ����
	//һ����ָ"���ɷָ�Ĳ���"
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
	return 0; //����ʱ�򲻵���wait ���� get ��Ȼ��ȴ��߳�ִ�����
}