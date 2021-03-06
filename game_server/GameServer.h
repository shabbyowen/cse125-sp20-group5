#pragma once
#ifndef _GAME_SERVER_H_
#define _GAME_SERVER_H_
#include <string>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include "GameState.hpp"
#include "ServerParams.h"

using boost::asio::ip::tcp;

class ClientConnection;
typedef boost::shared_ptr<ClientConnection> PtrClientConnection;

struct IGameServer {
    virtual void onDataRead(PtrClientConnection pConn, const char *pData, size_t bytes_transferred) = 0;
    virtual void onClientConnected(PtrClientConnection pConn) = 0;
    virtual void onClientDisconnected(PtrClientConnection pConn, const boost::system::error_code& error) = 0;
};


class ClientConnection : public boost::enable_shared_from_this<ClientConnection> {
public:

    static PtrClientConnection create(boost::asio::io_context& io_context, IGameServer* p_server);
    tcp::socket& socket();
    void start();
    void deliver(const std::string& msg);
    void deliverSerialization(char* buf);

    bool getStartMessageSent();
    void setStartMessageSent(bool sent);

private:
    ClientConnection(boost::asio::io_context& io_context, IGameServer* p_server);

    void stop(const boost::system::error_code& error);
    void doRead();
    void handleRead(const boost::system::error_code& error, size_t bytes_transferred);
    void handleWrite(const boost::system::error_code& error, size_t bytes_transferred);

    tcp::socket socket_;
    IGameServer* server;
    boost::asio::streambuf readStreamBuf;
    bool startMessageSent;
};

class GameServer : public boost::enable_shared_from_this<GameServer>,
                   public IGameServer {
public:
    GameServer(ServerParams& config, boost::asio::io_context& io_context);
    void startAccept();

private:

    void handleAccept(PtrClientConnection newConnection, const boost::system::error_code& error);
    void sendToAll();

    void onClientConnected(PtrClientConnection pConn);
    void onClientDisconnected(PtrClientConnection pConn, const boost::system::error_code& error);
    void onDataRead(PtrClientConnection pConn, const char* pData, size_t bytes_transferred);

    boost::asio::io_context& ioContext;
    ServerParams& config;
    tcp::acceptor tcpAcceptor;

    std::vector<PtrClientConnection> clients;
    boost::unordered::unordered_map<PtrClientConnection, Player*> pConn2Player;
    unsigned int playerIdCounter;
    bool gameStarted;

    boost::asio::steady_timer tickTimer;
    int deltaTimeMicrosec;
    std::vector<std::string> messages;
    GameState gameState;
    boost::recursive_mutex m_guard;
    enum { MAX_BUFFER_SIZE = 65536 };
};
#endif
