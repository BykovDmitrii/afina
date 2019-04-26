#ifndef AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <memory>
#include <string>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <spdlog/logger.h>

#include <afina/Storage.h>
#include <afina/execute/Command.h>
#include <afina/logging/Service.h>

#include "protocol/Parser.h"

namespace spdlog {
class logger;
}

namespace Afina {
namespace Network {
namespace STnonblock {

// Forward declaration, see ServerImpl.h
class ServerImpl;

class Connection {
public:
    Connection(int s, std::shared_ptr<Afina::Storage> ps, std::shared_ptr<spdlog::logger> plogger)
        : pStorage(ps), _socket(s), _logger(plogger) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    inline bool isAlive() const { return is_alive; }

    void Start();

protected:
    /**
     * Instance of backing storeage on which current server should execute
     * each command
     */
    std::shared_ptr<Afina::Storage> pStorage;

    /**
     * Logging service to be used in order to report application progress
     */

    void OnError();
    void OnClose();
    void OnStop(); // shutdown RD but don't declare "not alive"
    void DoRead();
    void DoWrite();

private:
    friend class ServerImpl;
    bool is_alive = true;
    int _socket;
    struct epoll_event _event;
    std::size_t arg_remains;
    std::string argument_for_command;
    std::shared_ptr<spdlog::logger> _logger;
    std::unique_ptr<Execute::Command> command_to_execute;
    char client_buffer[4096];
    int readed_bytes;
    Protocol::Parser parser;
    int written_bytes;
    char wbuf[4096];
    std::size_t wbuf_len;
};

} // namespace STnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_NONBLOCKING_CONNECTION_H
