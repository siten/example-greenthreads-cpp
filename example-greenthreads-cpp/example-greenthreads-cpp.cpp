// example-greenthreads-cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

const int DEFAULT_STACK_SIZE = 1024 * 1024 * 2;
const int MAX_THREADS = 4;

struct MyThreadContext {
    unsigned long long rsp_ = 0;
    unsigned long long r15_ = 0;
    unsigned long long r14_ = 0;
    unsigned long long r13_ = 0;
    unsigned long long r12_ = 0;
    unsigned long long rbx_ = 0;
    unsigned long long rbp_ = 0;
};

enum MyState {
    Available,
    Running,
    Ready,
};

struct MyThread {
    int id_;
    std::vector<char> stack_;
    MyThreadContext ctx_;
    MyState state_;

    MyThread(int id) {
        id_ = id;
        stack_.resize(DEFAULT_STACK_SIZE);
        state_ = MyState::Available;
    }
};

extern "C" void Myswitch(MyThreadContext *poldctx, MyThreadContext *pnewctx);
void guard();

struct MyRuntime {
    std::vector<MyThread> threads_;
    int current;

    MyRuntime() {
        MyThread base_thread(0);
        base_thread.state_ = MyState::Running;

        threads_.push_back(std::move(base_thread));

        for (int i = 1; i <= MAX_THREADS; i++)
        {
            MyThread Onethread(i);
            threads_.push_back(std::move(Onethread));
        }
        current = 0;    //base_thread
    }

    void run() {
        while (true)
        {
            if (!this->t_yield()) {
                ::exit(0);
            }
        }
    }

    void t_return() {
        if (this->current != 0)
        {
            this->threads_[current].state_ = MyState::Available;
            this->t_yield();
        }
    }

    bool t_yield() {
        //查找处于Ready状态的threads
        int pos = this->current;
        while (this->threads_[pos].state_ != MyState::Ready)
        {
            pos++;
            if (pos == this->threads_.size()) {
                pos = 0;
            }
            if (pos == this->current)
            {
                return false;
            }
        }

        //当前Mythread状态为Running要切换出去，状态改为Ready
        //当前Mythread 是guard调t_return过来的表示状态完成退出 在t_return已经把状态改为Available 表示下次切换不会选中
        if (this->threads_[this->current].state_ != MyState::Available)
        {
            this->threads_[this->current].state_ = MyState::Ready;
        }

        this->threads_[pos].state_ = MyState::Running;
        int old_pos = this->current;
        this->current = pos;
        printf("--- Mythread %d run ---\n", pos);
        //切换ctx
        Myswitch(&(this->threads_[old_pos].ctx_), &(this->threads_[pos].ctx_));

        return this->threads_.size() > 0;
    }

    void spawn(void *fun) {
        for (auto &one : this->threads_)
        {
            if (one.state_ == MyState::Available)
            {
                char *pend = &one.stack_[0] + one.stack_.size() - 1;
                //16字节对齐
                unsigned long long ullend = (unsigned long long)pend;
                ullend &= ~15;
                pend = (char *)ullend;

                *((unsigned long long*)(pend - 24)) = (unsigned long long)guard;
                *((unsigned long long*)(pend - 32)) = (unsigned long long)fun;

                one.ctx_.rsp_ = (unsigned long long)(pend - 32);

                one.state_ = MyState::Ready;
                break;
            }
        }
    }

};

static MyRuntime* pRuntime = nullptr;

void guard() {
    pRuntime->t_return();
}

void yield_thread() {
    pRuntime->t_yield();
}

void fun1() {
    printf("THREAD 1 STARTING\n");
    int id = 1;
    for (int i = 0; i < 10; i++)
    {
        printf("thread: %d counter: %d\n", id, i);
        yield_thread();
    }
    printf("THREAD 1 FINISHED\n");
}
void fun2() {
    printf("THREAD 2 STARTING\n");
    int id = 2;
    for (int i = 0; i < 15; i++)
    {
        printf("thread: %d counter: %d\n", id, i);
        yield_thread();
    }
    printf("THREAD 2 FINISHED\n");
}

int main()
{
    MyRuntime myrun;
    pRuntime = &myrun;

    myrun.spawn(fun1);
    myrun.spawn(fun2);
    myrun.run();

    std::cout << "Hello World!\n";
}
