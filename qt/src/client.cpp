#include "client.hpp"
#include <qmqtt.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageLogger>
#include <QReadLocker>
#include <QWriteLocker>

namespace SideAssist {
namespace Qt {

Client::Client(const QHostAddress& host, const quint16 port, QObject* parent)
    : mqtt_client_(std::make_unique<QMQTT::Client>(host, port, parent)) {
  connectSignals();
}

Client::Client(const QString& hostName,
               const quint16 port,
               const QSslConfiguration& config,
               const bool ignoreSelfSigned,
               QObject* parent)
    : mqtt_client_(std::make_unique<QMQTT::Client>(hostName,
                                                   port,
                                                   config,
                                                   ignoreSelfSigned,
                                                   parent)) {
  connectSignals();
}

Client::Client(const QString& url,
               const QString& origin,
               QWebSocketProtocol::Version version,
               bool ignoreSelfSigned,
               QObject* parent)
    : mqtt_client_(std::make_unique<QMQTT::Client>(url,
                                                   origin,
                                                   version,
                                                   ignoreSelfSigned,
                                                   parent)) {
  connectSignals();
}

Client::Client(const QString& url,
               const QString& origin,
               QWebSocketProtocol::Version version,
               const QSslConfiguration& config,
               const bool ignoreSelfSigned,
               QObject* parent)
    : mqtt_client_(std::make_unique<QMQTT::Client>(url,
                                                   origin,
                                                   version,
                                                   config,
                                                   ignoreSelfSigned,
                                                   parent)) {
  connectSignals();
}

void Client::connectSignals() {
  connect(mqtt_client_.get(), &QMQTT::Client::connected, this,
          &Client::connected);
  connect(this, &Client::connected, this, &Client::logConnected);
  connect(mqtt_client_.get(), &QMQTT::Client::disconnected, this,
          &Client::disconnected);
  connect(this, &Client::disconnected, this, &Client::logDisconnected);

  connect(mqtt_client_.get(), &QMQTT::Client::published, this,
          &Client::logPublished);

  connect(mqtt_client_.get(), &QMQTT::Client::error, this, &Client::logError);

  connect(mqtt_client_.get(), &QMQTT::Client::connected, this,
          &Client::setupSideAssistConnection);
  connect(this, &Client::connected, this, &Client::uploadAll);
  connect(mqtt_client_.get(), &QMQTT::Client::received, this,
          &Client::handleMessage);
}

void Client::connectToHost() {
  mqtt_client_->connectToHost();
}

std::shared_ptr<NamedValue> Client::addOption(const QString& name) {
  QWriteLocker lock(&options_lock_);
  auto itr = options_.emplace(
      name,
      std::make_shared<NamedValue>(name, QJsonValue(QJsonValue::Undefined)));

  if (itr.second) {
    qInfo("Created option %s", qPrintable(name));
    connect(itr.first->second.get(), &NamedValue::valueChanged, this,
            &Client::uploadChangedOptionValue);
  } else {
    qWarning("Trying to create existed parameter %s", qPrintable(name));
  }
  // uploadOptionValue(itr.first->second.get());
  return itr.first->second;
}

std::shared_ptr<NamedValue> Client::option(const QString& name,
                                           bool createIfNotFound) {
  QReadLocker lock(&options_lock_);
  auto itr = options_.find(name);
  if (itr != options_.end())
    return itr->second;

  lock.unlock();

  if (createIfNotFound)
    return addOption(name);
  return nullptr;
}

std::shared_ptr<NamedValue> Client::addParameter(const QString& name) {
  QWriteLocker lock(&parameters_lock_);
  auto itr = parameters_.emplace(
      name,
      std::make_shared<NamedValue>(name, QJsonValue(QJsonValue::Undefined)));

  if (itr.second) {
    qInfo("Created parameter %s", qPrintable(name));
    connect(itr.first->second.get(), &NamedValue::valueChanged, this,
            &Client::uploadChangedParameterValue);
  } else {
    qWarning("Trying to create existed parameter %s", qPrintable(name));
  }
  // uploadParameterValue(itr.first->second.get());
  return itr.first->second;
}

std::shared_ptr<NamedValue> Client::parameter(const QString& name,
                                              bool createIfNotFound) {
  QReadLocker lock(&parameters_lock_);
  auto itr = parameters_.find(name);
  if (itr != parameters_.end())
    return itr->second;

  lock.unlock();

  if (createIfNotFound)
    return addParameter(name);
  return nullptr;
}

void Client::setClientId(const QString& clientId) {
  mqtt_client_->setClientId(clientId);
}

void Client::setUsername(const QString& username) {
  mqtt_client_->setUsername(username);
}

void Client::setPassword(const QByteArray& password) {
  mqtt_client_->setPassword(password);
}

void Client::uploadChangedOptionValue() {
  const auto* opt = dynamic_cast<const NamedValue*>(sender());
  assert(opt != nullptr);
  assert(options_.find(opt->name()) != options_.end());
  uploadOptionValue(opt);
}

void Client::uploadChangedParameterValue() {
  const auto* param = dynamic_cast<const NamedValue*>(sender());
  assert(param != nullptr);
  assert(parameters_.find(param->name()) != parameters_.end());
  uploadParameterValue(param);
}

void Client::handleMessage(const QMQTT::Message& message) {
  const QString& topic = message.topic();
  QString start = "side_assist/" + mqtt_client_->clientId() + "/";
  if (!topic.startsWith(start)) {
    qCritical("Illegal topic prefix: %s", qPrintable(topic));
    return;
  }
  QStringList seg = topic.right(topic.length() - start.length()).split('/');

  if (seg.length() == 0) {
    qCritical("Illegal topic: %s", qPrintable(topic));
    return;
  }

  if (seg[0] == "option") {
    bool remoteSavedLocalValue = seg.length() == 2;
    if (!(seg.length() == 3 && seg[2] != "set" || remoteSavedLocalValue)) {
      qCritical("Illegal option operation: %s", qPrintable(topic));
      return;
    }
    auto itr = options_.find(seg[1]);
    if (itr == options_.end()) {
      qCritical("Illegal option name: %s", qPrintable(topic));
      return;
    }

    // TODO: check if value should be covered by remote

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.payload(), &error);
    auto payload_formatter = [&]() {
      return QString(message.payload())
          .replace(QRegularExpression("[ \t\n][ \t\n]+"), " ")
          .left(100);
    };
    if (error.error != QJsonParseError::NoError) {
      qCritical("Invalid json from payload(payload=\"%s\", topic=\"%s\"): %s",
                qPrintable(payload_formatter()), qPrintable(message.topic()),
                qPrintable(error.errorString()));
      return;
    }
    auto& value = doc["value"];
    if (value.isUndefined()) {
      qCritical("Invalid value from json(topic=\"%s\"): %s",
                qPrintable(message.topic()), qPrintable(payload_formatter()));
      return;
    }

    bool ret = itr->second->setValue(value);
    if (!ret) {
      qCritical("Validation failed on json(topic=\"%s\"): %s",
                qPrintable(message.topic()), qPrintable(payload_formatter()));
      return;
    }
    qInfo("Remote changed option %s: %s", qPrintable(seg[1]),
          qPrintable(payload_formatter()));
  }
}

void Client::uploadOptionValue(const NamedValue* option) {
  if (option->value().isUndefined()) {
    qCritical("Ignore uploading undefined option %s",
              qPrintable(option->name()));
    return;
  }
  QMQTT::Message message(
      0,
      "side_assist/" + mqtt_client_->clientId() + "/option/" + option->name(),
      QJsonDocument(QJsonObject({qMakePair("value", option->value())}))
          .toJson(QJsonDocument::Compact),
      2, true);
  qInfo("Uploading option %s...", qPrintable(option->name()));
  mqtt_client_->publish(message);
}

void Client::uploadParameterValue(const NamedValue* parameter) {
  if (parameter->value().isUndefined()) {
    qCritical("Ignore uploading undefined parameter %s",
              qPrintable(parameter->name()));
    return;
  }
  QMQTT::Message message(
      0,
      "side_assist/" + mqtt_client_->clientId() + "/param/" + parameter->name(),
      QJsonDocument(QJsonObject({qMakePair("value", parameter->value())}))
          .toJson(QJsonDocument::Compact),
      2, true);
  qInfo("Uploading parameter %s...", qPrintable(parameter->name()));
  mqtt_client_->publish(message);
}

void Client::logConnected() {
  qInfo("Connected to %s", qPrintable(mqtt_client_->host().toString()));
}

void Client::logDisconnected() {
  qCritical("Disconnected from %s",
            qPrintable(mqtt_client_->host().toString()));
}

void Client::logPublished(const QMQTT::Message& message, quint16 id) {
  qInfo("Message on topic %s published, id = %d", qPrintable(message.topic()),
        id);
}

void Client::logSubscribed(const QString& topic, const quint8 qos) {
  qInfo("Subscribed to topic %s, qos = %d", qPrintable(topic), qos);
}

void Client::logError(const QMQTT::ClientError error) {
  qCritical("MQTT Client error: %d", error);
}

void Client::setupSideAssistConnection() {
  qInfo("Setting up SideAssist connection to %s",
        qPrintable(mqtt_client_->host().toString()));
  mqtt_client_->subscribe(
      "side_assist/" + mqtt_client_->clientId() + "/option/#", 2);
}

void Client::uploadAll() {
  {
    QReadLocker lock(&options_lock_);
    for (auto& itr : options_) {
      uploadOptionValue(itr.second.get());
    }
  }
  {
    QReadLocker lock(&parameters_lock_);
    for (auto& itr : parameters_) {
      uploadParameterValue(itr.second.get());
    }
  }
}

}  // namespace Qt
}  // namespace SideAssist
