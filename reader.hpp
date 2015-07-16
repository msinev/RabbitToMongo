/* 
 * File:   reader.hpp
 * Author: max
 *
 * Created on December 27, 2014, 11:34 PM
 */

#ifndef READER_HPP
#define	READER_HPP

struct AMPQParam {
    AMPQParam();
    //AMPQParam( const AMPQParam & ) = default; 

    std::string hostname;
    int port;
    bool passive;
    bool durable;
    std::string vhost;
    std::string vhostuser;
    std::string vhostpass;
    std::string exchange;
    std::string bindingkey;
    std::string extype;
    std::string cert;
    std::string key;
    std::string keypass;

};

class messageAcceptor {
public:
    virtual void accept(char *message, int len) = 0;
};

class AMQPConnector : public AMPQParam {
    messageAcceptor *acceptor;
    int status;
    volatile bool quit;
    amqp_connection_state_t conn;
    amqp_socket_t *socket;
public:
    AMQPConnector(messageAcceptor *dst, const AMPQParam &);
    int init();
    void run();
    void close();
    void inter();
};



#endif	/* READER_HPP */

