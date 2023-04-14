#include <net.hpp>
#include <select.hpp>
#include <poll.hpp>

#include <signal.h>

using NNet::TSimpleTask;
using NNet::TSocket;
using NNet::TSelect;
using NNet::TAddress;
using TLoop = NNet::TLoop<TSelect>;

TSimpleTask client_handler(TSocket socket, TLoop* loop) {
    char buffer[128] = {0}; ssize_t size = 0;

    try {
        while ((size = co_await socket.ReadSome(buffer, sizeof(buffer))) > 0) {
            std::cerr << "Received: " << std::string_view(buffer, size) << "\n";
            co_await socket.WriteSome(buffer, size);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
    }
    if (size == 0) { 
        std::cerr << "Client disconnected\n";
    }
    co_return;
}

TSimpleTask server(TLoop* loop, TAddress address)
{
    TSocket socket(std::move(address), loop->Poller());
    socket.Bind();
    socket.Listen();

    while (true) {
        auto client = co_await socket.Accept();
        client_handler(std::move(client), loop);
    }
    co_return;
}

int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    int port = 0;
    if (argc > 1) { port = atoi(argv[1]); }
    if (port == 0) { port = 8888; }

    TAddress address{"0.0.0.0", port};
    TLoop loop;
    server(&loop, std::move(address));
    loop.Loop();
    return 0;
}
