#pragma once

#define LIB_NAME "TinyHttp"
#define MODULE_NAME "dhttp"
#define DLIB_LOG_DOMAIN "DHTTP"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <dmsdk/sdk.h>
#include <TinyServer.hpp>
#include <stdlib.h>

struct LuaCallbackInfo
{
    LuaCallbackInfo() : m_L(0), m_Callback(LUA_NOREF), m_Self(LUA_NOREF) {}
    lua_State *m_L;
    int m_Callback;
    int m_Self;
};

struct MessageCommand
{
    int m_id;
    int m_eventId;
    char *m_ws_message;
};

struct ConnectionState
{
    LuaCallbackInfo server_Callback;
    LuaCallbackInfo client_Callback;

    ConnectionState()
    {
        server_Callback.m_Callback = LUA_NOREF;
        server_Callback.m_Self = LUA_NOREF;

        client_Callback.m_Callback = LUA_NOREF;
        client_Callback.m_Self = LUA_NOREF;
    }

    dmArray<MessageCommand> m_CmdQueue;
};
ConnectionState *state = 0;

void QueueCommand(int id, int eventID, char *ws_message);

void QueueCommand(int id, int eventID, char *ws_message)
{
    MessageCommand cmd;
    cmd.m_id = id;
    cmd.m_eventId = eventID;
    cmd.m_ws_message = ws_message;

    if (state->m_CmdQueue.Full())
    {
        state->m_CmdQueue.OffsetCapacity(8);
    }
    state->m_CmdQueue.Push(cmd);
}

static void RegisterCallback(lua_State *L, int index, LuaCallbackInfo *cbk)
{
    if (cbk->m_Callback != LUA_NOREF)
    {
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
    }

    cbk->m_L = dmScript::GetMainThread(L);
    luaL_checktype(L, index, LUA_TFUNCTION);

    lua_pushvalue(L, index);
    cbk->m_Callback = dmScript::Ref(L, LUA_REGISTRYINDEX);

    dmScript::GetInstance(L);
    cbk->m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);
}

static void UnregisterCallback(LuaCallbackInfo *cbk)
{
    if (cbk->m_Callback != LUA_NOREF)
    {
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
        cbk->m_Callback = LUA_NOREF;
    }
}

static void InvokeCallback(LuaCallbackInfo *cbk, MessageCommand *cmd)
{
    if (cbk->m_Callback == LUA_NOREF)
    {
        return;
    }

    lua_State *L = cbk->m_L;
    DM_LUA_STACK_CHECK(L, 0);

    lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Callback);
    lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Self);
    lua_pushvalue(L, -1);

    dmScript::SetInstance(L);

    lua_createtable(L, 0, 0); // Main Table

    lua_pushnumber(L, cmd->m_eventId);
    lua_setfield(L, -2, "event_id");

    lua_pushstring(L, cmd->m_ws_message);
    lua_setfield(L, -2, "result");

    int number_of_arguments = 2; // instance + 1
    int ret = lua_pcall(L, number_of_arguments, 0, 0);
    if (ret != 0)
    {
        dmLogError("Error running callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

static void FlushCommandQueue()
{

    for (uint32_t i = 0; i != state->m_CmdQueue.Size(); ++i)
    {

        MessageCommand *cmd = &state->m_CmdQueue[i];
        if (cmd->m_id == 0) // From server
        {
            InvokeCallback(&state->server_Callback, cmd);
        }
        else // From client
        {
            InvokeCallback(&state->client_Callback, cmd);
        }

        state->m_CmdQueue.EraseSwap(i--);
    }
}

TinyServer *server = 0;
