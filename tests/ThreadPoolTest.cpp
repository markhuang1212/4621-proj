#include <bits/stdc++.h>
#include "src/ThreadPool.cpp"
#include "unistd.h"

using namespace std;

int main()
{
    ThreadPool tp(4);

    for (int i = 0; i < 20; i++)
    {
        tp.addTask([=]() {
            cout << "Hello World from " << this_thread::get_id() << " " << i << endl;
        });
    }

    while (tp.numOfWaitingTasks() || tp.numOfRunningTasks())
    {
        this_thread::yield();
    }

    tp.shutDown();
}