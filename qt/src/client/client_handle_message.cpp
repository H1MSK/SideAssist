#include <QJsonDocument>
#include "client.hpp"

namespace SideAssist::Qt {

void Client::handleMessage(const QMQTT::Message& message) {
  const QString& topic = message.topic();
  QString start = "side_assist/" + mqtt_client_->clientId() + "/";
  if (!topic.startsWith(start)) {
    qCritical("Illegal topic prefix: %s", qUtf8Printable(topic));
    return;
  }
  QStringList seg = topic.right(topic.length() - start.length()).split('/');

  if (seg.length() == 0) {
    qCritical("Illegal topic: %s", qUtf8Printable(topic));
    return;
  }

  if (seg[0] == "option") {
    bool remoteSavedLocalValue = seg.length() == 2;
    if (!(seg.length() == 3 && seg[2] == "set" || remoteSavedLocalValue)) {
      qCritical("Illegal option operation: %s", qUtf8Printable(topic));
      return;
    }
    auto itr = options_.find(seg[1]);
    if (itr == options_.end()) {
      qCritical("Illegal option name: %s", qUtf8Printable(topic));
      return;
    }

    if (remoteSavedLocalValue) {
      if (!itr->second->value().isUndefined()) {
        qWarning("Ignore remote saved value for option %s",
                 qUtf8Printable(seg[1]));
        return;
      } else {
        qInfo("Found remote saved value for option %s", qUtf8Printable(seg[1]));
      }
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.payload(), &error);
    auto payload_formatter = [&]() {
      QString str = QString(message.payload())
                        .replace(QRegularExpression("[ \t\n][ \t\n]+"), " ");
      if (str.length() > 256)
        str = str.left(253) + "...";
      return str;
    };
    if (error.error != QJsonParseError::NoError) {
      qCritical("Invalid json from payload(payload=\"%s\", topic=\"%s\"): %s",
                qUtf8Printable(payload_formatter()),
                qUtf8Printable(message.topic()),
                qUtf8Printable(error.errorString()));
      return;
    }
    auto& value = doc["value"];
    if (value.isUndefined()) {
      qCritical("Invalid value from json(topic=\"%s\"): %s",
                qUtf8Printable(message.topic()),
                qUtf8Printable(payload_formatter()));
      return;
    }

    bool ret = itr->second->validate(value);
    if (!ret) {
      qCritical("Validation failed on json(topic=\"%s\"): %s",
                qUtf8Printable(message.topic()),
                qUtf8Printable(payload_formatter()));
      return;
    }

    itr->second->setValue(value);

    if (!remoteSavedLocalValue) {
      qInfo("Remote changed option %s: %s", qUtf8Printable(seg[1]),
            qUtf8Printable(payload_formatter()));
    }
  }
}

}  // namespace SideAssist::Qt
