#pragma once
#include <thread>
#include <httplib.h>
#include <Enums.hpp>
#include <dmsdk/dlib/log.h>

using namespace httplib;

class TinyServer
{
public:
    TinyServer(){};
    ~TinyServer();

    // Server
    void startServ(const char *n_host, int n_port, bool enableLog = false, bool enableError = true);
    void serverStop();
    void setPostResponseContent(const char *str);
    bool isServerRunning();

    //Client
    void initClient(const char *n_host, int n_port);
    void clientHi();
    void clientGet(const char *path, int eventID);
    void clientPost(const char *path, int eventID);

    httplib::Params postParams;
    std::multimap<int, const char *> endPoints;

private:
    // Server
    Server svr;
    const char *host = "localhost";
    int port = 8808;

    bool isServerLogEnabled = false;
    bool isServerErrorEnabled = false;

    char *serverRegex(const Request &req, bool isString = false);
    const char *PostResponseContent = "{}";

    std::thread *serverThread;

    std::string dump_headers(const Headers &headers);
    std::string log(const Request &req, const Response &res);

    char *parseParams(const Request &req);
    int getEventID(const Request &req);

    //Client
    const char *chost = "localhost";
    int cport = 8808;

    Client *cli = nullptr;
    const char *contentType = "application/json";
};