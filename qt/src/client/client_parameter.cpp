#include "client.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QReadLocker>
#include <QWriteLocker>
#include "value_validator.hpp"

namespace SideAssist {
namespace Qt {

std::shared_ptr<NamedValue> Client::addParameter(const QString& name) {
  QWriteLocker lock(&parameters_lock_);
  auto itr = parameters_.emplace(
      name,
      std::make_shared<NamedValue>(name, QJsonValue(QJsonValue::Undefined)));

  if (itr.second) {
    qInfo("Created parameter %s", qUtf8Printable(name));
    connect(itr.first->second.get(), &NamedValue::valueChanged, this,
            &Client::uploadChangedParameterValue);
    connect(itr.first->second.get(), &NamedValue::validatorChanged, this,
            &Client::uploadChangedParameterValidator);
  } else {
    qWarning("Trying to create existed parameter %s", qUtf8Printable(name));
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

void Client::uploadChangedParameterValue() {
  if (!mqtt_client_->isConnectedToHost())
    return;
  const auto* param = dynamic_cast<const NamedValue*>(sender());
  assert(param != nullptr);
  assert(parameters_.find(param->name()) != parameters_.end());
  uploadParameterValue(param);
}

void Client::uploadChangedParameterValidator() {
  if (!mqtt_client_->isConnectedToHost())
    return;
  const auto* param = dynamic_cast<const NamedValue*>(sender());
  assert(param != nullptr);
  assert(parameters_.find(param->name()) != parameters_.end());
  uploadParameterValidator(param);
}

void Client::uploadParameterValue(const NamedValue* parameter) {
  if (!mqtt_client_->isConnectedToHost()) {
    qWarning("Trying to upload parameter %s when not connected",
             qUtf8Printable(parameter->name()));
    return;
  }
  if (parameter->value().isUndefined()) {
    qCritical("Ignore uploading undefined parameter %s",
              qUtf8Printable(parameter->name()));
    return;
  }
  QMQTT::Message message(
      0,
      "side_assist/" + mqtt_client_->clientId() + "/param/" + parameter->name(),
      QJsonDocument(QJsonObject({qMakePair("value", parameter->value())}))
          .toJson(QJsonDocument::Compact),
      2, true);
  qInfo("Uploading parameter %s...", qUtf8Printable(parameter->name()));
  mqtt_client_->publish(message);
}

void Client::uploadParameterValidator(const NamedValue* parameter) {
  if (!mqtt_client_->isConnectedToHost()) {
    qWarning("Trying to upload option %s when not connected",
             qUtf8Printable(parameter->name()));
    return;
  }
  QByteArray buf;
  if (parameter->validator() != nullptr) {
    buf = QJsonDocument(
              QJsonObject({qMakePair(
                  "validator", parameter->validator()->serializeToJson())}))
              .toJson(QJsonDocument::Compact);
  }
  QMQTT::Message message(0,
                         "side_assist/" + mqtt_client_->clientId() +
                             "/parameter/" + parameter->name(),
                         buf, 2, true);
  qInfo("Uploading parameter %s...", qUtf8Printable(parameter->name()));
  mqtt_client_->publish(message);
}

}  // namespace Qt
}  // namespace SideAssist
