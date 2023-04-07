#include "client.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QReadLocker>
#include <QWriteLocker>
#include "value_validator.hpp"

namespace SideAssist {
namespace Qt {

std::shared_ptr<NamedValue> Client::addOption(const QString& name) {
  QWriteLocker lock(&options_lock_);
  auto itr = options_.emplace(
      name,
      std::make_shared<NamedValue>(name, QJsonValue(QJsonValue::Undefined)));

  if (itr.second) {
    qInfo("Created option %s", qPrintable(name));
    connect(itr.first->second.get(), &NamedValue::valueChanged, this,
            &Client::uploadChangedOptionValue);
    connect(itr.first->second.get(), &NamedValue::validatorChanged, this,
            &Client::uploadChangedOptionValidator);
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

void Client::uploadChangedOptionValue() {
  if (!mqtt_client_->isConnectedToHost())
    return;
  const auto* opt = dynamic_cast<const NamedValue*>(sender());
  assert(opt != nullptr);
  assert(options_.find(opt->name()) != options_.end());
  uploadOptionValue(opt);
}

void Client::uploadChangedOptionValidator() {
  if (!mqtt_client_->isConnectedToHost())
    return;
  const auto* opt = dynamic_cast<const NamedValue*>(sender());
  assert(opt != nullptr);
  assert(options_.find(opt->name()) != options_.end());
  uploadOptionValidator(opt);
}

void Client::uploadOptionValue(const NamedValue* option) {
  if (!mqtt_client_->isConnectedToHost()) {
    qWarning("Trying to upload option %s when not connected",
             qPrintable(option->name()));
    return;
  }
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

void Client::uploadOptionValidator(const NamedValue* option) {
  if (!mqtt_client_->isConnectedToHost()) {
    qWarning("Trying to upload option %s when not connected",
             qPrintable(option->name()));
    return;
  }
  QByteArray buf;
  if (option->validator() != nullptr) {
    buf = QJsonDocument(
              QJsonObject({qMakePair("validator",
                                     option->validator()->serializeToJson())}))
              .toJson(QJsonDocument::Compact);
  }
  QMQTT::Message message(
      0,
      "side_assist/" + mqtt_client_->clientId() + "/option/" + option->name(),
      buf, 2, true);
  qInfo("Uploading option %s...", qPrintable(option->name()));
  mqtt_client_->publish(message);
}

}  // namespace Qt
}  // namespace SideAssist
