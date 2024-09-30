#pragma once

#include <map>

#include "v8.h"

using std::string;
using std::map;

using v8::Isolate;
using v8::Global;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::Value;
using v8::Global;
using v8::Function;

class ScriptRunner
{
public:
    ScriptRunner();
    ~ScriptRunner();

    void RunScript(string& scriptPath);
    void PushScriptHandler(int npcId, Global<Function>* fn)
    {
        _scriptHandlers[npcId] = fn;
    }
    Isolate* GetIsolate() const { return _isolate; }
    Global<Context>& GetContext() { return _context; }
    void Execute();

    static void Log(const FunctionCallbackInfo<Value>& args);
    static void RegisterCreateScript(const FunctionCallbackInfo<Value>& args);

private:
    Isolate::CreateParams _create_params; // TODO: kiprobalni hogy main-ben deklaraljuk
    Isolate* _isolate;
    Global<Context> _context;

    map<int, Global<Function>* > _scriptHandlers;
};
