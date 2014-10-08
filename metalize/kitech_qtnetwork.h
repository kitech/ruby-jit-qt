#ifndef KITECH_QTNETWORK_H
#define KITECH_QTNETWORK_H

#include <QtCore>
#include <QtNetwork>

/*
class yQHostAddress : public QObject, QHostAddress
{
    Q_OBJECT;
    Q_ENUMS(SpecialAddress)
public:
    enum SpecialAddress {
        Null = QHostAddress::Null,
        LocalHost = QHostAddress::LocalHost,
        LocalHostIPv6 = QHostAddress::LocalHostIPv6,
        Broadcast = QHostAddress::Broadcast,
        AnyIPv4 = QHostAddress::AnyIPv4,
        AnyIPv6 = QHostAddress::AnyIPv6,
        Any = QHostAddress::Any,
    };
public slots:
    void 	clear() { QHostAddress::clear();}
    bool 	isInSubnet(const QHostAddress & subnet, int netmask) const
    {return QHostAddress::isInSubnet(subnet, netmask);}
    bool 	isInSubnet(const QPair<QHostAddress, int> & subnet) const
    {return QHostAddress::isInSubnet(subnet);}
    bool 	isLoopback() const { return QHostAddress::isLoopback();}
    bool 	isNull() const {return QHostAddress::isNull();}
    QAbstractSocket::NetworkLayerProtocol 	protocol() const
    {return QHostAddress::protocol();}
    QString 	scopeId() const {return QHostAddress::scopeId();}
    void 	setAddress(quint32 ip4Addr) {return QHostAddress::setAddress(ip4Addr);}
    void 	setAddress(quint8 * ip6Addr) {return QHostAddress::setAddress(ip6Addr);}
    void 	setAddress(const Q_IPV6ADDR & ip6Addr)
    {QHostAddress::setAddress(ip6Addr);}
    bool 	setAddress(const QString & address)
    {return QHostAddress::setAddress(address);}
    void 	setAddress(const sockaddr * sockaddr)
    {QHostAddress::setAddress(sockaddr);}
    void 	setScopeId(const QString & id) {QHostAddress::setScopeId(id);}
    quint32 	toIPv4Address() const{ return QHostAddress::toIPv4Address();}
    Q_IPV6ADDR 	toIPv6Address() const{ return QHostAddress::toIPv6Address();}
    QString 	toString() const    {return QHostAddress::toString();}
};
*/
/*
class yQTcpSocket : public QTcpSocket
{
    Q_OBJECT;

public slots:
    
    void 	abort() { QTcpSocket::abort(); }
    bool 	bind(const QHostAddress & address, quint16 port = 0, BindMode mode = DefaultForPlatform)
    { return QTcpSocket::bind(address, port, mode); }
    bool 	bind(quint16 port = 0, BindMode mode = DefaultForPlatform)
    { return QTcpSocket::bind(port, mode); }
    virtual void 	connectToHost(const QString & hostName, quint16 port, OpenMode openMode = ReadWrite, NetworkLayerProtocol protocol = AnyIPProtocol)
    { QTcpSocket::connectToHost(hostName, port, openMode, protocol); }
    virtual void 	connectToHost(const QHostAddress & address, quint16 port, OpenMode openMode = ReadWrite)
    { QTcpSocket::connectToHost(address, port, openMode); }
    virtual void 	disconnectFromHost() { QTcpSocket::disconnectFromHost(); }
    SocketError 	error() const { return QTcpSocket::error(); }
    bool 	flush() {return QTcpSocket::flush(); }
    bool 	isValid() const    { return QTcpSocket::isValid(); }
};
*/

/*
class yQTcpServer : public QTcpServer
{
    Q_OBJECT;
public:
    Q_INVOKABLE yQTcpServer(QObject * parent = 0) : QTcpServer(parent) {}
    Q_INVOKABLE virtual 	~yQTcpServer() {}

public slots:
    void 	close() { QTcpServer::close();}
    QString 	errorString() const{ return QTcpServer::errorString();}
    virtual bool 	hasPendingConnections() const
    {return QTcpServer::hasPendingConnections();}
    bool 	isListening() const {return QTcpServer::isListening(); }
    bool 	listen(const QHostAddress & address = QHostAddress::Any, quint16 port = 0)
    {return QTcpServer::listen(address, port); }
    int 	maxPendingConnections() const {return QTcpServer::maxPendingConnections();}
    virtual QTcpSocket * 	nextPendingConnection()
    {return QTcpServer::nextPendingConnection();}
    void 	pauseAccepting() {QTcpServer::pauseAccepting();}
    QNetworkProxy 	proxy() const {return QTcpServer::proxy();}
    void 	resumeAccepting() {QTcpServer::resumeAccepting();}
    QHostAddress 	serverAddress() const {return QTcpServer::serverAddress();}
    QAbstractSocket::SocketError 	serverError() const
    {return QTcpServer::serverError();}
    quint16 	serverPort() const {return QTcpServer::serverPort();}
    void 	setMaxPendingConnections(int numConnections)
    {QTcpServer::setMaxPendingConnections(numConnections);}
    void 	setProxy(const QNetworkProxy & networkProxy)
    {QTcpServer::setProxy(networkProxy);}
    bool 	setSocketDescriptor(qintptr socketDescriptor)
    {return QTcpServer::setSocketDescriptor(socketDescriptor);}
    qintptr 	socketDescriptor() const
    {return QTcpServer::socketDescriptor();}
    bool 	waitForNewConnection(int msec = 0, bool * timedOut = 0)
    {return QTcpServer::waitForNewConnection(msec, timedOut);}

};
*/

#include <ruby.hpp>

extern "C" {
    int register_qtnetwork_methods(VALUE module);
};


#endif /* KITECH_QTNETWORK_H */










