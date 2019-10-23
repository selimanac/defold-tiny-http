#include <TinyServer.hpp>
#include <iostream>
extern void QueueCommand(int id, char *ws_message);

void TinyServer::initClient(const char *n_host, int n_port)
{

    chost = n_host;
    cport = n_port;

    cli = new Client(chost, cport);
}

void TinyServer::sayHi()
{
    auto res = cli->Get("/hi");
    printf("SAY HI %i", 1);
    if (res && res->status == 200)
    {
        std::cout << res->body << std::endl;

        char *abc = new char[res->body.length() + 1];
        strcpy(abc, res->body.c_str());

        QueueCommand(1, abc);
    }
}

void TinyServer::startServ(const char *n_host, int n_port)
{

    host = n_host;
    port = n_port;

    std::thread *th = new std::thread([&]() {
        Server svr;

        svr.Get("/hi", [this](const Request &req, Response &res) {
            res.set_content("{ \"test\": \"Defold says hi!\" }", contentType);
            QueueCommand(0, "{ \"result\": 1 }");
        });

        svr.Get("/stop", [&](const Request &req, Response &res) {
            svr.stop();
        });

        svr.listen(host, port);
    svr.is_running();
      /*   while (!svr.is_running()) {
           
        } */
    });
}