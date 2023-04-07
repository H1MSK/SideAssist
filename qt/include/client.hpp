#pragma once

#include <qmqtt.h>
#include <QByteArray>
#include <QHostAddress>
#include <QObject>
#include <QReadWriteLock>
#include <map>
#include <memory>
#include "global.hpp"
#include "named_value.hpp"

#ifndef QT_NO_SSL
#include <QSslConfiguration>
#endif  // QT_NO_SSL
#ifdef QT_WEBSOCKETS_LIB
#include <QWebSocketProtocol>
#endif  // QT_WEBSOCKETS_LIB

namespace SideAssist::Qt {

class Q_SIDEASSIST_EXPORT Client : public QObject {
  Q_OBJECT
 public:
  Client(const QHostAddress& host = QHostAddress::LocalHost,
         const quint16 port = 1883,
         QObject* parent = nullptr);
#ifndef QT_NO_SSL
  Client(const QString& hostName,
         const quint16 port,
         const QSslConfiguration& config,
         const bool ignoreSelfSigned = false,
         QObject* parent = nullptr);
#endif  // QT_NO_SSL

#ifdef QT_WEBSOCKETS_LIB
  // Create a connection over websockets
  Client(const QString& url,
         const QString& origin,
         QWebSocketProtocol::Version version,
         bool ignoreSelfSigned = false,
         QObject* parent = nullptr);

#ifndef QT_NO_SSL
  Client(const QString& url,
         const QString& origin,
         QWebSocketProtocol::Version version,
         const QSslConfiguration& config,
         const bool ignoreSelfSigned = false,
         QObject* parent = nullptr);
#endif  // QT_NO_SSL
#endif  // QT_WEBSOCKETS_LIB

  void connectToHost();

  std::shared_ptr<NamedValue> addOption(const QString& name);
  std::shared_ptr<NamedValue> option(const QString& name,
                                     bool createIfNotFound = false);
  std::shared_ptr<NamedValue> addParameter(const QString& name);
  std::shared_ptr<NamedValue> parameter(const QString& name,
                                        bool createIfNotFound = false);

  bool installDefaultMessageHandler();

 public slots:
  void setClientId(const QString& clientId);
  void setUsername(const QString& username);
  void setPassword(const QByteArray& password);
  void uploadChangedOptionValue();
  void uploadChangedParameterValue();
  void uploadChangedOptionValidator();
  void uploadChangedParameterValidator();

 signals:
  void connected();
  void disconnected();

 private slots:
  void setupSideAssistConnection();
  void handleMessage(const QMQTT::Message& message);
  void uploadOptionValue(const NamedValue* option);
  void uploadParameterValue(const NamedValue* parameter);
  void uploadOptionValidator(const NamedValue* option);
  void uploadParameterValidator(const NamedValue* parameter);
  void uploadAll();

  void logConnected();
  void logDisconnected();
  void logPublished(const QMQTT::Message& message, quint16 id);
  void logSubscribed(const QString& topic, const quint8 qos);
  void handleMqttError(const QMQTT::ClientError error);

 private:
  void connectSignals();

 private:
  std::unique_ptr<QMQTT::Client> mqtt_client_;

  std::map<QString, std::shared_ptr<NamedValue> > options_;
  QReadWriteLock options_lock_;
  std::map<QString, std::shared_ptr<NamedValue> > parameters_;
  QReadWriteLock parameters_lock_;
};

}  // namespace SideAssist::Qt
