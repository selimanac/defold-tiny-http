#pragma once
#include <thread>
#include <httplib.h>

using namespace httplib;

class TinyServer
{

public:
    TinyServer(){};
    ~TinyServer(){};

    void startServ(const char *n_host, int n_port);
    void initClient(const char *n_host, int n_port);
    void sayHi();

private:
    const char *host = "localhost";
    int port = 8808;

    const char *chost = "localhost";
    int cport = 8808;

    Client *cli;
    const char *contentType = "application/json";
};