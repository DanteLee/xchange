#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>

#include <iostream>

#include <xchange/base/ThreadPool.h>
#include <xchange/io/Channel.h>
#include <xchange/io/Poller.h>
#include <xchange/io/poller/EpollPoller.h>
#include <xchange/io/channel/PTCPChannel.h>
#include <xchange/io/channel/TCPChannel.h>
#include <xchange/io/channel/FSChannel.h>
#include <xchange/io/EventLoop.h>
#include <xchange/io/Buffer.h>
#include <xchange/io/utils.h>
#include <xchange/net/Address.h>

using xchange::io::Buffer;
using xchange::io::EventLoop;
using xchange::io::channel::Channel;
using xchange::io::channel::ChannelEvent;
using xchange::io::channel::ChannelType;
using xchange::io::channel::TCPChannel;
using xchange::io::channel::PTCPChannel;
using xchange::io::channel::FSChannel;
using xchange::io::poller::Poller;
using xchange::io::poller::EpollPoller;
using xchange::io::utils::setNonblockingChannel;
using xchange::net::socket::SocketAddress;
using xchange::threadPool::ThreadPool;
#define LOG_FILE_NAME "log.txt"
#define LOG_FILE_FLAG (O_RDWR | O_CREAT)
#define LOG_FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
using std::cout;
using std::endl;

int main() {
    ThreadPool::ptr pool(new ThreadPool);
    Poller::ptr poller(new EpollPoller);
    EventLoop loop(pool, poller);
    SocketAddress host("127.0.0.1", 12345, SocketAddress::ipv4);
    PTCPChannel *acceptor = PTCPChannel::createPassiveTCPChannel(AF_INET, host.getStruct(), sizeof(struct sockaddr_storage));
    FSChannel *logFile = FSChannel::open(pool, LOG_FILE_NAME, LOG_FILE_FLAG, LOG_FILE_MODE);

    if (logFile == NULL) {
        cout << "create log file failed" << endl;
        return errno;
    } else {
        cout << "fd: " << logFile->fd() << endl;
    }

    if (acceptor == NULL) {
        cout << "create acceptor failed" << endl;
        return errno;
    }

    struct sigaction act;
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, NULL);

    // neccessary
    setNonblockingChannel(acceptor);

    acceptor->on(ChannelEvent::IN, [&loop, logFile](ChannelEvent, void *arg) {
                PTCPChannel *channel = static_cast<PTCPChannel *>(arg);

                while (channel->readable()) {
                    TCPChannel *conn = channel->accept();

                    if (conn != NULL) {
                        cout << "new connection incoming" << endl;
                    } else {
                        continue;
                    }

                    conn->on(ChannelEvent::IN, [&loop, logFile](ChannelEvent, void *arg) {
                                TCPChannel *channel = static_cast<TCPChannel *>(arg);
                                Buffer result;

                                cout << "IN event triggered on " << channel->fd() << endl;

                                while (channel->readable()) {
                                    Buffer buff = channel->read(1024);

                                    if (buff.empty() && channel->eof()) {
                                        cout << "connection closed by peer" << endl;
                                        loop.removeChannel(channel);

                                        return;
                                    } else {
                                        cout << "receive " << buff.size() << " bytes data" << endl;
                                    }

                                    result += buff;

                                    if (buff.size() < 1024) {
                                        channel->write(result);

                                        int64_t nwrite = logFile->write(buff, [](int error, void*){
                                                    cout << "write log complete: " << strerror(error) << endl;
                                                 });
                                        cout << "start writing " << nwrite << " bytes to " << LOG_FILE_NAME << endl;
                                        return;
                                    }
                                }
                            });
                    conn->on(ChannelEvent::ERROR, [&loop](ChannelEvent, void *arg) {
                                TCPChannel *channel = static_cast<TCPChannel *>(arg);

                                cout << "Channel closed due to an error" << endl;

                                loop.removeChannel(channel);
                            });

                    // neccessary
                    setNonblockingChannel(conn);

                    if (conn != NULL) {
                        if (loop.addChannel(conn)) {
                            cout << "add channel failed" << endl;
                        };
                    } else {
                        break;
                    }
                }
            });

    loop.addChannel(acceptor);

    loop.loop();

    return 0;
}
