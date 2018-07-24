#include <iostream>
#include <thread>
using namespace std;

thread_local int g_n = 1;

void f()
{
    g_n++;
    cout << "id=" << std::this_thread::get_id() << "  n=" << g_n << endl;
}

void foo()
{
    thread_local int i = 0;
    cout << "id=" << std::this_thread::get_id() << "  n=" << i << endl;
    i++;
}

void f2()
{
    foo();
    foo();
}

int main()
{
    g_n++;
    f();               // 3
    std::thread t1(f); // 2
    std::thread t2(f); // 2

    t1.join();
    t2.join();

    f2();               // 0 1
    std::thread t4(f2); // 0 1
    std::thread t5(f2); // 0 1

    t4.join();
    t5.join();
    return 0;
}