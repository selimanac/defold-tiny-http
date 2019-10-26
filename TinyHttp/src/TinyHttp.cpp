#include <TinyHttp.hpp>

void _startServer(const char *host, int port, bool enableLog, bool enableError)
{
    server->startServ(host, port, enableLog, enableError);
}

static int _serverServe(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);

    RegisterCallback(L, 3, &state->server_Callback);

    bool enableLog = false;
    if (lua_isboolean(L, 4))
    {
        enableLog = lua_toboolean(L, 4);
    }

    bool enableError = false;
    if (lua_isboolean(L, 5))
    {
        enableError = lua_toboolean(L, 5);
    }

    if (lua_istable(L, 6))
    {
        luaL_checktype(L, 6, LUA_TTABLE);
        if (lua_istable(L, 6))
        {
            server->endPoints.clear();

            int end_point_type;
            const char *end_point;
            std::string field;

            lua_pushnil(L);
            while (lua_next(L, 6) != 0)
            {
                if (lua_istable(L, -1))
                {
                    lua_pushnil(L);
                    while (lua_next(L, -2) != 0)
                    {

                        field = lua_tostring(L, -2);
                        if (field == "end_point")
                        {
                            end_point = lua_tostring(L, -1);
                        }
                        else if (field == "end_point_type")
                        {
                            end_point_type = lua_tointeger(L, -1);
                        }

                        lua_pop(L, 1);
                    }

                    server->endPoints.emplace(end_point_type, end_point);
                }

                lua_pop(L, 1);
            }
        }
    }

    _startServer(host, port, enableLog, enableError);
    return 0;
}

static int _clientSayHi(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    server->clientHi();
    return 0;
}

static int _clientGet(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    const char *path = luaL_checkstring(L, 1);
    int eventID = luaL_checkint(L, 2);
    server->clientGet(path, eventID);
    return 0;
}

static int _clientPost(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *path = luaL_checkstring(L, 1);
    int eventID = luaL_checkint(L, 3);
    luaL_checktype(L, 2, LUA_TTABLE);
    if (lua_istable(L, 2))
    {
        server->postParams.clear();

        lua_pushnil(L);
        while (lua_next(L, 2) != 0)
        {
            server->postParams.emplace(lua_tostring(L, -2), lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    server->clientPost(path, eventID);
    return 0;
}

static int _clientServe(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    RegisterCallback(L, 3, &state->client_Callback);

    server->initClient(host, port);

    return 0;
}

static int _serverSetPostContent(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    const char *host = luaL_checkstring(L, 1);
    server->setPostResponseContent(host);
    return 0;
}

static int _serverStop(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    server->serverStop();
    return 0;
}

static int _serverRunning(lua_State *L)
{
    int top = lua_gettop(L);
    bool isRunning = server->isServerRunning();
    lua_pushboolean(L, isRunning);
    assert(top + 1 == lua_gettop(L));
    return 1;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
    {
        {"server_start", _serverServe},
        {"server_post_content", _serverSetPostContent},
        {"server_stop", _serverStop},
        {"is_server_running", _serverRunning},

        {"client_start", _clientServe},
        {"client_hi", _clientSayHi},
        {"client_get", _clientGet},
        {"client_post", _clientPost},
        {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name)                \
    lua_pushnumber(L, (lua_Number)name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(METHOD_GET);
    SETCONSTANT(METHOD_POST);
#undef SETCONSTANT

#define SETCONSTANT(name)                \
    lua_pushnumber(L, (lua_Number)name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(SERVER_START);
    SETCONSTANT(SERVER_STOP);
#undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeTinyHttp(dmExtension::AppParams *params)
{
    server = new TinyServer;
    state = new ConnectionState;
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeTinyHttp(dmExtension::Params *params)
{
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result UpdateTinyHttp(dmExtension::Params *params)
{
    if (state)
    {
        FlushCommandQueue();
    }

    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeTinyHttp(dmExtension::Params *params)
{
    server->serverStop();
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(TinyHttp, LIB_NAME, AppInitializeTinyHttp, 0, InitializeTinyHttp, UpdateTinyHttp, 0, FinalizeTinyHttp)
