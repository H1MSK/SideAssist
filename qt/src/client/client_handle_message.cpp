#include "client.hpp"
#include <QJsonDocument>

namespace SideAssist {
namespace Qt {

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
    if (!(seg.length() == 3 && seg[2] == "set" || remoteSavedLocalValue)) {
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
      QString str = QString(message.payload())
          .replace(QRegularExpression("[ \t\n][ \t\n]+"), " ");
      if (str.length() > 256)
        str = str.left(253) + "...";
      return str;
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

    bool ret = itr->second->validate(value);
    if (!ret) {
      qCritical("Validation failed on json(topic=\"%s\"): %s",
                qPrintable(message.topic()), qPrintable(payload_formatter()));
      return;
    }
    itr->second->setValue(value);
    qInfo("Remote changed option %s: %s", qPrintable(seg[1]),
          qPrintable(payload_formatter()));
  }
}

}  // namespace Qt
}  // namespace SideAssist
