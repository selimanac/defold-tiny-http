#include <TinyServer.hpp>
#include <iostream>
extern void QueueCommand(int id, int eventID, char *ws_message);

TinyServer::~TinyServer()
{
    if (svr.is_running())
    {
        serverStop();
    }
};

std::string TinyServer::dump_headers(const Headers &headers)
{
    std::string s;
    char buf[BUFSIZ];

    for (auto it = headers.begin(); it != headers.end(); ++it)
    {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }

    return s;
}

std::string TinyServer::log(const Request &req, const Response &res)
{
    std::string s;
    char buf[BUFSIZ];

    s += "================================\n";

    snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
             req.version.c_str(), req.path.c_str());
    s += buf;

    std::string query;
    for (auto it = req.params.begin(); it != req.params.end(); ++it)
    {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "%c%s=%s",
                 (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
                 x.second.c_str());
        query += buf;
    }
    snprintf(buf, sizeof(buf), "%s\n", query.c_str());
    s += buf;

    s += dump_headers(req.headers);

    s += "--------------------------------\n";

    snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
    s += buf;
    s += dump_headers(res.headers);
    s += "\n";

    if (!res.body.empty())
    {
        s += res.body;
    }

    s += "\n";

    return s;
}

void TinyServer::serverStop()
{
    if (svr.is_running())
    {
        svr.stop();
        serverThread->join();
    }
}

bool TinyServer::isServerRunning()
{
    return svr.is_running();
}

void TinyServer::initClient(const char *n_host, int n_port)
{
    chost = n_host;
    cport = n_port;
    cli = new Client(chost, cport, 5);
}

void TinyServer::clientHi()
{
    auto res = cli->Get("/hi");
    char *result = new char[res->body.length() + 1];
    strcpy(result, res->body.c_str());
    QueueCommand(1, 0, result);
}

void TinyServer::clientGet(const char *path, int eventID)
{
    if (!cli)
    {
        dmLogError("Client is not initialized!");
        return;
    }

    httplib::Headers headers = {{"Event-ID", std::to_string(eventID)}};
    auto res = cli->Get(path, headers);

    if (!res)
    {
        QueueCommand(1, 0, "{ \"error\": 503 }");
        return;
    }

    if (res && res->status == 200)
    {
        char *result = new char[res->body.length() + 1];
        strcpy(result, res->body.c_str());
        QueueCommand(1, eventID, result);
    }
    else if (res)
    {
        std::string error_result = "{ \"error\": " + std::to_string(res->status) + " }";
        char *result = new char[error_result.length() + 1];
        strcpy(result, error_result.c_str());
        QueueCommand(1, eventID, result);
    }
}

void TinyServer::clientPost(const char *path, int eventID)
{
    if (!cli)
    {
        dmLogError("Client is not initialized!");
        return;
    }

    httplib::Headers headers = {{"Event-ID", std::to_string(eventID)}};
    auto res = cli->Post(path, headers, postParams);

    if (!res)
    {
        QueueCommand(1, 0, "{ \"error\": 503 }");
        return;
    }

    if (res && res->status == 200)
    {
        char *result = new char[res->body.length() + 1];
        strcpy(result, res->body.c_str());
        QueueCommand(1, eventID, result);
    }
    else if (res)
    {
        std::string error_result = "{ \"error\": " + std::to_string(res->status) + " }";
        char *result = new char[error_result.length() + 1];
        strcpy(result, error_result.c_str());
        QueueCommand(1, eventID, result);
    }
}

char *TinyServer::serverRegex(const Request &req, bool isString)
{
    std::string value = req.matches[1];
    if (isString)
    {
        value = "\"" + value + "\"";
    }
    std::string result = "{ \"result\": " + value + " }";

    char *cstr = new char[result.length() + 1];
    strcpy(cstr, result.c_str());

    return cstr;
}

void TinyServer::setPostResponseContent(const char *str)
{
    PostResponseContent = str;
}

char *TinyServer::parseParams(const Request &req)
{
    std::string s;
    char buf[BUFSIZ];

    std::string query;
    for (auto it = req.params.begin(); it != req.params.end(); ++it)
    {
        const auto &x = *it;
        snprintf(buf, sizeof(buf), "\"%s\": %s,", x.first.c_str(), x.second.c_str());
        query += buf;
    }

    query = query.substr(0, query.size() - 1);

    snprintf(buf, sizeof(buf), "{%s}", query.c_str());
    s += buf;
    char *data_cstr = new char[s.length() + 1];
    strcpy(data_cstr, s.c_str());
    return data_cstr;
}

int TinyServer::getEventID(const Request &req)
{
    auto search = req.headers.find("Event-ID");
    int event_id = 0;
    if (search != req.headers.end())
    {
        event_id = std::stoi(search->second);
    }
    return event_id;
}

void TinyServer::startServ(const char *n_host, int n_port, bool enableLog, bool enableError)
{
    host = n_host;
    port = n_port;

    isServerLogEnabled = enableLog;
    isServerErrorEnabled = enableError;

    serverThread = new std::thread([&]() {
        //   Server svr;

        for (std::multimap<int, const char *>::iterator it = endPoints.begin(); it != endPoints.end(); ++it)
        {
            if ((*it).first == METHOD_GET)
            {
                svr.Get((*it).second, [&](const Request &req, Response &res) {
                    char *cstr = serverRegex(req, true);
                    res.set_content(cstr, contentType);
                    QueueCommand(0, getEventID(req), cstr);
                });
            }
            else if ((*it).first == METHOD_POST)
            {
                svr.Post((*it).second, [&](const Request &req, Response &res) {
                    res.status = 200;
                    if (PostResponseContent == "{}")
                    {
                        res.set_content(parseParams(req), contentType);
                    }
                    else
                    {
                        res.set_content(PostResponseContent, contentType);
                    }

                    QueueCommand(0, getEventID(req), parseParams(req));
                });
            }
        }

        svr.Get("/hi", [this](const Request &req, Response &res) {
            res.set_content("{ \"result\": \"Defold says hi!\" }", contentType);
            QueueCommand(0, getEventID(req), "{ \"result\": \"Defold says hi!\" }");
        });

        svr.Get(R"(/num/(\d+))", [&](const Request &req, Response &res) {
            char *cstr = serverRegex(req);
            res.set_content(cstr, contentType);
            QueueCommand(0, getEventID(req), cstr);
        });

        svr.Get(R"(/str/(\w+))", [&](const Request &req, Response &res) {
            char *cstr = serverRegex(req, true);
            res.set_content(cstr, contentType);
            QueueCommand(0, getEventID(req), cstr);
        });

        svr.Post("/post", [&](const Request &req, Response &res) {
            res.status = 200;
            if (PostResponseContent == "{}")
            {
                res.set_content(parseParams(req), contentType);
            }
            else
            {
                res.set_content(PostResponseContent, contentType);
            }
            QueueCommand(0, getEventID(req), parseParams(req));
        });

        svr.Get("/stop", [&](const Request &req, Response &res) {
            //-svr.stop();
            serverStop();
        });

        if (isServerErrorEnabled)
        {
            svr.set_error_handler([this](const Request & /*req*/, Response &res) {
                std::string error_result = "{ \"error\": " + std::to_string(res.status) + " }";
                res.set_content(error_result.c_str(), contentType);
                char *result = new char[error_result.length() + 1];
                strcpy(result, error_result.c_str());
                QueueCommand(0, 0, result);
                QueueCommand(1, 0, result);
            });
        }

        if (isServerLogEnabled)
        {
            svr.set_logger([this](const Request &req, const Response &res) {
                printf("%s", log(req, res).c_str());
            });
        }

        svr.listen(host, port);
    });
}