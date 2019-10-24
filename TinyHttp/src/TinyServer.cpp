#include <TinyServer.hpp>
#include <iostream>
extern void QueueCommand(int id, char *ws_message);

TinyServer::~TinyServer()
{
    if (svr.is_running())
    {
        serverStop();
    }
};

void TinyServer::serverStop()
{
    if (svr.is_running())
    {
        svr.stop();
        serverThread->join();
    }
}

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

void TinyServer::initClient(const char *n_host, int n_port)
{

    chost = n_host;
    cport = n_port;

    cli = new Client(chost, cport);
}

void TinyServer::clientHi()
{
    auto res = cli->Get("/hi");
    if (res && res->status == 200)
    {
        char *result = new char[res->body.length() + 1];
        strcpy(result, res->body.c_str());

        QueueCommand(1, result);
    }
}

void TinyServer::clientGet(const char *path)
{

    auto res = cli->Get(path);
    if (res && res->status == 200)
    {
        char *result = new char[res->body.length() + 1];
        strcpy(result, res->body.c_str());

        QueueCommand(1, result);
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

void TinyServer::clientPost(const char *path )
{
   
    auto res = cli->Post(path,postParams);
    if (res && res->status == 200)
    {
        char *result = new char[res->body.length() + 1];
        strcpy(result, res->body.c_str());

        QueueCommand(1, result);
    }
}


void TinyServer::startServ(const char *n_host, int n_port, bool enableLog, bool enableError)
{

    host = n_host;
    port = n_port;

    isServerLogEnabled = enableLog;
    isServerErrorEnabled = enableError;

    serverThread = new std::thread([&]() {
        //   Server svr;

        svr.Get("/hi", [this](const Request &req, Response &res) {
            res.set_content("{ \"result\": \"Defold says hi!\" }", contentType);
            QueueCommand(0, "{ \"result\": \"Defold says hi!\" }");
        });

        svr.Get(R"(/num/(\d+))", [&](const Request &req, Response &res) {
            char *cstr = serverRegex(req);
            res.set_content(cstr, contentType);
            QueueCommand(0, cstr);
        });

        svr.Get(R"(/str/(\w+))", [&](const Request &req, Response &res) {
            char *cstr = serverRegex(req, true);
            res.set_content(cstr, contentType);
            QueueCommand(0, cstr);
        });

        svr.Post("/post", [&](const Request &req, Response &res) {
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

            res.status = 200;
            res.set_content(PostResponseContent, contentType);
            char *data_cstr = new char[s.length() + 1];
            strcpy(data_cstr, s.c_str());

            QueueCommand(0, data_cstr);
        });

        svr.Get("/stop", [&](const Request &req, Response &res) {
            svr.stop();
        });

        if (isServerErrorEnabled)
        {
            svr.set_error_handler([this](const Request & /*req*/, Response &res) {
                const char *fmt = "{ \"error\": %d }";
                ;
                char buf[BUFSIZ];
                snprintf(buf, sizeof(buf), fmt, res.status);
                res.set_content(buf, contentType);
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