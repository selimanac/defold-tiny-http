#include <TinyHttp.hpp>

void startServer(const char *host, int port)
{
    
    server->startServ(host, port);
}

void initClient(const char *host, int port)
{
    server->initClient(host, port);
}

void shi()
{
    server->sayHi();
}

static int serve(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    RegisterCallback(L, 3, &state->server_Callback);

    startServer(host, port);
    // server->serv("host", 8800);
    // Return 1 item
    return 0;
}

static int sayHi(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

   shi();
    return 0;
}

static int client(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char *host = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    RegisterCallback(L, 3, &state->client_Callback);

    initClient(host, port);
    // server->serv("host", 8800);
    // Return 1 item
    return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
    {
        {"serve", serve},
        {"client", client},
        {"hi", sayHi},
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
