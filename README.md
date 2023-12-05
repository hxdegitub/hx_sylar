## 
#sylar
目前完成的部分，视频教程中模块都已完成

## 日志
log4j 格式的日志类。
主要类：
* Logger 日志器 进行日志输出的类。内部组合了若干日志输出地集合，日志的格式器。
* LogAppender:日志输出地，主要有两种文件输出，标准输出。还有日志等级，日志格式（可以设置为false表示不用自己的日志）。
* LogFormatter ：日志格式器，可以对输出日志格式进行定制。输出日志级别，线程号，时间等。
内部定义了许多FormatItem ，每个item就对应一种日志输出的内容。比如一个时间就有一个DateTimeForamtItem,然后当解析到日志格式有一个%d ,就将其添加到后面。类似于工厂模式。解析了字符串之后就会将其存入到m_items,
* LogEvent: 日志事件，在打日志的时候，会有一些环境信息，在哪个文件，哪一行，哪个协程哪个线程等信息。LoggEvent就是包含了这些信息。
* LogEventWrap包装日志事件。
采用单例模式对Logger进行管理。

## 配置模块
采用YAML文件来进行服务器的配置。使用了yaml-cpp库来进行yaml文件的解析。
主要类：
* ConfigVar :代表了某个配置的变量，如服务器的端口，日志的等级。
* Config: 一组配置变量的集合，用map来记录，比如{"port",ConfigVar(80)}  
  用Lookup来读取配置中的变量。
  
## 线程模块
封装了Linux下pthread 库对于线程的使用。
主要类：Thread,Semaphore ,Spinlock,RWMute,ScopedLockImpl类。
* Thread :线程：内部主体是一个pthread_t 和回调函数cb。提供以下接口：
  * join即回收线程。
  * 构造函数，封装了pthread_create函数，将run函数作为回调注册。
  * run函数,执行内部的cb。
  * 析构函数，如果没用被join 就进行回收。
* Semaphore： 信号，封装了sem_t 。
* Mutex: 封装了底层的pthread_mutex  pthread_rwlock_t pthread_mutex_t 
  
## 协程模块
基于ucontext 实现，协程可以理解为更强大的函数，普通函数智只能进行调用，然后从头执行到尾，即使调用了其他或者自己，对应的寄存器环境等信息还是存在，并不能挂起，或者从某点继续执行。这里是一个非对称的有栈协程，而cpp20开始就对无栈协程进行编译器层次支持。
主要类：fiber，内部主要就是一个ucontet_t 和回调函数  等信息。
一个协程存在以下的状态，INIT,HOLD,EXEC,TERM,READY,EXCEPT依次对应：
* init:初始化阶段，刚刚被创建，或者在进行复用设置了内部回调函数。
* HOLD: 挂起，当当前协程被让出的时候就会将当前的ucontext替换成其他协程，这里只有两种一种是主协程替换成子协程，或者反过来，但是不能从子协程切换成子协程，
* EXEC: 执行中。
* TERM: 结束。
* EXCEPTT： 异常状态。
* READY ： 可执行状态
这里只是提供了协程的操作，没用真正的进行调度。
操作包括：
* reset 重新设置回调。
* swapIn() 执行该线程。
* swapOut() 暂停协程到hold
* call() 执行协程，语义同swapIn，但是这个是给调度器使用的。
* back() 暂停协程，给调度器使用。
在调度模块中会有调度。
为了支持调度，这里提供了一些接口：
* Fiber() 无参构造函数，用于线程创建的时候对其进行线程主协程创建。
* static void SetThis(Fiber* f);设置线程中运行的线程，用于切换线程，里面就一个动作，将线程局部变量t_fiber设置为参数f，真正的执行切换寄存器逻辑不在setThis()里面。
* static Fiber::ptr GetThis() ; 获取当前正在执行的线程，如果线程没有在执行的线程说明线程没启用协程，那么就为其创建并设置主协程，再返回主协程。
* static void YieldToReady(); 将当前正在执行的协程切换到后台并将其设置为Ready 状态，这里设置为Ready 是有其他用处，
* static void YieldToHold(); 和上面相同只是默认挂起。
* static uint64_t TotalFibers(); 返回总协程数，实际获取这个意义不大，可以用来判断是否结束了。
* static void MainFunc() ; 获取到正在执行的协程然后执行。就是真正意义是的执行。
* static void CallerMainFunc(); 同上

## 协程调度模块
对于线程多协程模型进行一个调度。对于上述协程模块，存在的最大缺陷就是子协程不能再创建子协程，必须回到主协程主协程才有资格进行新协程创建，这就给程序员带来了很多负担，所以我们可以使用调度器，进行调度，然后将所有协程都放到协程调度器里面去。
调度器：内部实现了多线程管理多协程。这里协程调度器有一个比较特殊的地方，是否允许调度调度器所在线程（caller线程）。怎么理解这个概念呢，
如果允许，那么创建调度器的线程，一般是主线程，就会被加入到协程调度器里面，供协程调度器作为一个worker线程使用，
反之只能调度新创建的线程，比如我是要使用一个m个线程的调度器，允许调度caller线程，那么我还需要创建m-1个线程，不允许的话，我就需要创建m个线程。
可以先看看构造函数
构造函数：   Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = ""); / 第一个参数即内部线程池线程数量，第二个参数就是是否允许使用caller线程，第三个就是调度器名字。
与调度相关接口：
* 构造函数Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
  如果将当前线程当作调度线程，需要为当前线程设置调度器和调度协程为this 和新协程。

*   static Scheduler* GetThis(); 返回当前线程的调度器。
*   static Fiber* GetMainFiber(); 返回当前线程的调度器所在协程。
*   start() 启动调度器， 创建线程池，都绑定run函数。
*   stop() 停止调度器。  
*   void schedule(FiberOrCb fc, int thread = -1) ： 加入协程或者函数，进行调度。
*   void switchTo(int thread = -1); 协程切换到哪个线程
*   std::ostream& dump(std::ostream& os);
*   virtual void tickle();  通知调度器进行调度。
*   run() 协程调度函数，这个是每个线程都进行的一个函数，也是调度的核心函数。
  设置线程的本地变量包括 当前线程的协程调度器和调度协程。如果不是caller线程就创建或者设置其主协程。
  然后就去里面寻找空置线程，与需要执行的回调，然后执行，如果被终止了就挂起进入hold状态，如果只是暂停到ready 就继续加入调度器中，需要继续执行。一种情况就结束了。
*   void setThis(); 设置当前进行的协程调度器
*   bool hasIdleThreads() { return m_idleThreadCount > 0;} 是否有空置线程
*   idel 闲置了
    
## 定时器模块
基于epoll_wait来实现ms级别的定时器，采取最小堆管理定时任务。
主要类：
Timer:
内部记录了到时的实际，即需要执行回调的时间点，比如现在是今天的第5ms ，你给定时器安排一个10ms之后的定时任务，那么在内部记录的就是15ms，这里的绝对时间指的应该是相对对1970年的时间。还有就是定时的任务即一个回调函数。然后是否循环执行。这个就是只是一个定时器容器，用来作为调度的基础设施。就像协程和协程调度关系。
TimerManager:
内部记录了上次执行任务的时间。
对所有Timer进行管理，利用epoll_wait的特性来进行时间更新。
每次epoll_wait返回的时候，都获取到当前时间，然后将时间小于等于该时间点的Timer全部取出，拿出所有任务进行执行。
里面的接口有：
*   virtual void onTimerInsertedAtFront() = 0; 给子类使用，因为插入首部可能是需要执行的Timer
*   void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock); 添加计数器
*   bool detectClockRollover(uint64_t now_ms); 检测服务器时间是否调后，即是否需要调度
*   void listExpiredCb(std::vector<std::function<void()> >& cbs); 列出所有需要执行的timer
*   uint64_t getNextTimer() ; 获取最近的定时器的执行时间。
*   Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb,std::weak_ptr<void> weak_cond,bool recurring = false);
 添加条件定时任务。
真正运用了定时器的示例在下一个模块IOManager
## IO管理模块
继承了协程管理模块与定时器管理器
这个类真正的实现了定时器。
内部就是一个epoll的句柄，可以理解为一个epoll树的钥匙，通过这个可以使用epoll进行文件描述符的管理。
加上一个pipe 句柄。
内部还有一个FdContext，封装了fd包括其关注的事件其回调的调度。
除了对epoll树添加和删除其关注的事件，就是实现两个基类的接口。
*    void tickle() override; 在onTimerInsertedAtFront,被调用，作用主要是触发管道的读写数据，使得epoll_wait返回。
*    bool stopping() override; 停止没啥好说的。
*    void idle() override;   epoll等待，实现定时器时间设置。检测Timer的函数。
*    void onTimerInsertedAtFront() override; 插入新的timer在前面就执行。
*    void contextResize(size_t size); 重新设置context的大小。

##