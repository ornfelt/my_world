#include "ScriptRunner.h"

#include <fstream>
#include <sstream>

#include "ScriptManager.h"
#include "Player.h"
#include "Creature.h"
#include "Logger.h"

using std::string;
using std::ifstream;
using std::stringstream;
using std::cout;
using std::endl;
using std::getline;

using v8::ArrayBuffer;
using v8::String;
using v8::HandleScope;
using v8::Script;
using v8::ObjectTemplate;
using v8::FunctionTemplate;
using v8::External;
using v8::Local;

ScriptRunner::ScriptRunner()
{
    ostringstream ss;
    ss << "[ScriptRunner::ScriptRunner]";
    Logger::Log(ss);

    _create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
    _isolate = Isolate::New(_create_params);

    HandleScope handle_scope(_isolate);

    Local<ObjectTemplate> global = ObjectTemplate::New(_isolate);

    Local<External> instance = External::New(_isolate, this);
    global->Set(String::NewFromUtf8(_isolate, "Log"), FunctionTemplate::New(_isolate, &Log, instance));
    global->Set(String::NewFromUtf8(_isolate, "RegisterCreateScript"), FunctionTemplate::New(_isolate, &RegisterCreateScript, instance));

    _context.Reset(_isolate, Context::New(_isolate, nullptr, global));
}

ScriptRunner::~ScriptRunner()
{
    ostringstream ss;
    ss << "[ScriptRunner::~ScriptRunner]";
    Logger::Log(ss);

    _context.Reset();
    _isolate->Dispose();
    delete _create_params.array_buffer_allocator;
}

void ScriptRunner::RunScript(string& scriptPath)
{
    ostringstream ss;
    ss << "[ScriptRunner::RunScript]";
    Logger::Log(ss);

    ifstream sourceFile(scriptPath);

    stringstream sourceStringStream;
    string line;
    while (getline(sourceFile, line))
    {
        sourceStringStream << line << endl;
    }
    sourceFile.close();
    sourceStringStream.flush();

    HandleScope handle_scope(_isolate);

    Local<Context> context = _context.Get(_isolate);
    Context::Scope context_scope(context);

    Local<String> source = String::NewFromUtf8(_isolate, sourceStringStream.str().c_str());
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    script->Run();
}

void ScriptRunner::Execute()
{
    ostringstream ss;
    ss << "[ScriptRunner::Execute]";
    Logger::Log(ss);

    ScriptManager* sm = new ScriptManager(_scriptHandlers, this);

    Player* anuka = new Player("anuka", this);
    Player* lajoska = new Player("lajoska", this);
    Player* geza = new Player("geza", this);

    Creature* illidariBattleMage1 = new Creature(23402, sm);
    illidariBattleMage1->EnterCombat(lajoska);

    Creature* illidariBattleMage2 = new Creature(23402, sm);
    Creature* illidan = new Creature(22917, sm);

    illidariBattleMage1->EnterCombat(lajoska);
    illidariBattleMage2->EnterCombat(geza);

    illidan->EnterCombat(anuka);

    illidariBattleMage1->EnterCombat(geza);
}

void ScriptRunner::Log(const FunctionCallbackInfo<Value>& args)
{
    ostringstream ss;
    ss << "[ScriptRunner::Log] message: " << *String::Utf8Value(args[0]);
    Logger::Log(ss);
}

void ScriptRunner::RegisterCreateScript(const FunctionCallbackInfo<Value>& args)
{
    Local<External> e = Local<External>::Cast(args.Data());
    ScriptRunner* instance = static_cast<ScriptRunner*>(e->Value());

    Isolate* isolate = instance->GetIsolate();
    Isolate::Scope isolate_scope(isolate);

    int npcId = args[0]->ToInt32()->Value();
    Local<Function> fn = Local<Function>::Cast(args[1]);

    ostringstream ss;
    ss << "[ScriptRunner::RegisterCreateScript] functionName: " << *String::Utf8Value(fn->GetName());
    Logger::Log(ss);

    instance->PushScriptHandler(npcId, new Global<Function>(isolate, fn));
}
