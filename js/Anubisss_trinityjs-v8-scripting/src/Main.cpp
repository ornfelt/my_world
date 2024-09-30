#include <string>
#include <vector>
#include <thread>

#include "v8.h"
#include "libplatform/libplatform.h"

#include "ScriptRunner.h"
#include "ScriptManager.h"
#include "Creature.h"
#include "Player.h"
#include "Logger.h"

using std::string;
using std::vector;
using std::thread;

using v8::V8;
using v8::Platform;
using v8::platform::CreateDefaultPlatform;

mutex Logger::_mutex;

string getWorkingDirPath(string&& executablePath)
{
    return executablePath.substr(0, executablePath.find_last_of("/") + 1);
}

void Worker(string (&scriptPaths)[2])
{
    ostringstream ss;
    ss << "[Worker]";
    Logger::Log(ss);

    ScriptRunner* sr = new ScriptRunner();

    for (auto& scriptPath : scriptPaths)
    {
        sr->RunScript(scriptPath);
    }

    sr->Execute();

    delete sr;
}

int main(int argc, char* argv[])
{
    V8::InitializeExternalStartupData(argv[0]);
    Platform* platform = CreateDefaultPlatform();
    V8::InitializePlatform(platform);
    V8::Initialize();

    cout << "[main] thread_id: " << std::this_thread::get_id() << endl;

    string workingDirPath = getWorkingDirPath(string(argv[0]));
    string scriptPaths[2] = { workingDirPath + "/scripts/BossIllidanStormrage.js", workingDirPath + "scripts/MobIllidariBattleMage.js" };

    int threadNum = 4;
    vector<thread> threads;
    for (int i = 0; i < threadNum; ++i)
    {
        threads.push_back(thread(Worker, std::ref(scriptPaths)));
    }

    for (auto& tr : threads)
    {
        tr.join();
    }

    cout << "[main] SHUTDOWN thread_id: " << std::this_thread::get_id() << endl;

    V8::Dispose();
    V8::ShutdownPlatform();
    delete platform;

    return 0;
}
