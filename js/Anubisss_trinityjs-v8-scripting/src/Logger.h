#pragma once

#include <iostream>
#include <sstream>
#include <mutex>
#include <thread>

using std::cout;
using std::endl;
using std::ostringstream;
using std::mutex;
using std::lock_guard;

class Logger
{
public:
    static void Log(ostringstream& s)
    {
        lock_guard<mutex> guard(_mutex);
        cout << s.str() << " thread ID: " << std::this_thread::get_id() << endl;
    }
private:
    static mutex _mutex;
};
