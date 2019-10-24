#pragma once
#include <thread>
#include <httplib.h>

using namespace httplib;

class TinyServer
{

public:
    TinyServer(){};
    ~TinyServer();

    void startServ(const char *n_host, int n_port, bool enableLog = false, bool enableError = false);
    void initClient(const char *n_host, int n_port);
    void clientHi();
    void clientGet(const char *path);
    void setPostResponseContent(const char *str);
    void serverStop();
    void clientPost(const char *path);

    httplib::Params postParams;

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
    const char *PostResponseContent;
    Server svr;
};