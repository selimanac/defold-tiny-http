#pragma once
#include <thread>
#include <httplib.h>
#include <Enums.hpp>

using namespace httplib;

class TinyServer
{

public:
    TinyServer(){};
    ~TinyServer();

    void startServ(const char *n_host, int n_port, bool enableLog = false, bool enableError = false);
    void serverStop();
    void setPostResponseContent(const char *str);
    
    void initClient(const char *n_host, int n_port);
    void clientHi();
    void clientGet(const char *path, int eventID);
    void clientPost(const char *path, int eventID);
    
    httplib::Params postParams;
    std::multimap<int, const char *> endPoints;

    bool isServerRunning();

private:
    const char *host = "localhost";
    int port = 8808;

    const char *chost = "localhost";
    int cport = 8808;

    Client *cli;
    const char *contentType = "application/json";

    std::thread *serverThread;

    bool isServerLogEnabled = false;
    bool isServerErrorEnabled = false;

    std::string dump_headers(const Headers &headers);
    std::string log(const Request &req, const Response &res);

    char *serverRegex(const Request &req, bool isString = false);
    const char *PostResponseContent ="{}";
    Server svr;

    char * parseParams(const Request &req);
};