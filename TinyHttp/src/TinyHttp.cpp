#include <TinyHttp.hpp>

void startServer(const char *host, int port, bool enableLog, bool enableError)
{
    server->startServ(host, port, enableLog, enableError);
}

void initClient(const char *host, int port)
{
    server->initClient(host, port);
}

static int serverServe(lua_State *L)
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

    startServer(host, port, enableLog, enableError);
    // server->serv("host", 8800);
    // Return 1 item
    return 0;
}

static int clientSayHi(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    server->clientHi();
    return 0;
}

static int clientGet(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    const char *path = luaL_checkstring(L, 1);
    server->clientGet(path);
    return 0;
}

static int clientPost(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *path = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    if (lua_istable(L, 2))
    {
        server->postParams.clear();
        
        lua_pushnil(L);
        while (lua_next(L, 2) != 0)
        {
            printf("ID: %s\n", lua_tostring(L, -2));
            printf("Val: %s\n", lua_tostring(L, -1));
            server->postParams.emplace(lua_tostring(L, -2), lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    server->clientPost(path);
    return 0;
}

static int clientServe(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    RegisterCallback(L, 3, &state->client_Callback);

    // initClient(host, port);
    server->initClient(host, port);
    // server->serv("host", 8800);
    // Return 1 item
    return 0;
}

static int serverSetPostContent(lua_State *L)
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

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
    {
        {"server_start", serverServe},
        {"server_post_content", serverSetPostContent},
        {"server_stop", _serverStop},
        {"client_start", clientServe},
        {"client_hi", clientSayHi},
        {"client_get", clientGet},
        {"client_post", clientPost},
        // {"post", post},
        {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

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

dmExtension::Result AppFinalizeTinyHttp(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeTinyHttp(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(TinyHttp, LIB_NAME, AppInitializeTinyHttp, AppFinalizeTinyHttp, InitializeTinyHttp, UpdateTinyHttp, 0, FinalizeTinyHttp)
