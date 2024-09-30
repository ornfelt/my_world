#include "CreatureAI.h"

#include "Creature.h"
#include "Player.h"
#include "ScriptRunner.h"
#include "Logger.h"

using v8::Local;
using v8::HandleScope;
using v8::ObjectTemplate;
using v8::String;
using v8::External;
using v8::FunctionTemplate;

CreatureAI::CreatureAI(Global<Function>* constructor, Creature* creature, ScriptRunner* sr) : _sr(sr)
{
    ostringstream ss;
    ss << "[CreatureAI::CreatureAI]";
    Logger::Log(ss);

    Isolate* isolate = _sr->GetIsolate();
    Isolate::Scope isolate_scope(isolate);

    HandleScope handle_scope(isolate);

    Local<Context> context = _sr->GetContext().Get(isolate);
    Context::Scope context_scope(context);

    // TODO: ennek valahol 1 helyen kene lennie "globalisan" majd az egyes CreatureAI-k Object-eket csinalnak belole NewInstance-al
    Local<ObjectTemplate> t = ObjectTemplate::New();
    t->SetInternalFieldCount(1);
    t->Set(String::NewFromUtf8(isolate, "CastSpell"), FunctionTemplate::New(isolate, &CreatureJS::CastSpell));
    t->Set(String::NewFromUtf8(isolate, "GetId"), FunctionTemplate::New(isolate, &CreatureJS::GetId));
    t->Set(String::NewFromUtf8(isolate, "GetGuid"), FunctionTemplate::New(isolate, &CreatureJS::GetGuid));

    Local<Object> obj = t->NewInstance();
    obj->SetInternalField(0, External::New(isolate, creature));

    Local<Value> constructorArgs[1] = { obj };
    _scriptObj.Reset(isolate, constructor->Get(isolate)->NewInstance(context, 1, constructorArgs).ToLocalChecked());
}

void CreatureAI::EnterCombat(Player* player)
{
    ostringstream ss;
    ss << "[CreatureAI::EnterCombat]";
    Logger::Log(ss);

    Isolate* isolate = _sr->GetIsolate();
    Isolate::Scope isolate_scope(isolate);

    HandleScope handle_scope(isolate);

    Local<Context> context = _sr->GetContext().Get(isolate);
    Context::Scope context_scope(context);

    // TODO: ennek valahol 1 helyen kene lennie "globalisan" majd az egyes CreatureAI-k Object-eket csinalnak belole NewInstance-al
    Local<ObjectTemplate> t = ObjectTemplate::New();
    t->SetInternalFieldCount(1);
    t->Set(String::NewFromUtf8(isolate, "GetName"), FunctionTemplate::New(isolate, &PlayerJS::GetName));

    Local<Object> obj = t->NewInstance();
    obj->SetInternalField(0, External::New(isolate, player));

    Local<Value> args[1] = { obj };

    Local<Object> localScriptObj = _scriptObj.Get(isolate);
    Local<Function> fn = Local<Function>::Cast(localScriptObj->Get(context, String::NewFromUtf8(isolate, "EnterCombat")).ToLocalChecked());
    fn->Call(context, localScriptObj, 1, args);
}
