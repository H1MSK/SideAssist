#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QReadLocker>
#include <QWriteLocker>
#include "client.hpp"
#include "value_validator.hpp"

namespace SideAssist::Qt {

std::shared_ptr<NamedValue> Client::addOption(
    const QString& name,
    bool accept_remote_initial_value) {
  QWriteLocker lock(&options_lock_);
  auto itr = options_.emplace(
      name,
      std::make_shared<NamedValue>(name, QJsonValue(QJsonValue::Undefined)));

  if (itr.second) {
    qInfo("Created option %s", qUtf8Printable(name));
    connect(itr.first->second.get(), &NamedValue::valueChanged, this,
            &Client::uploadChangedOptionValue);
    connect(itr.first->second.get(), &NamedValue::validatorChanged, this,
            &Client::uploadChangedOptionValidator);
    if (mqtt_client_->isConnectedToHost())
      setupSubscriptionsForOption(itr.first->second.get(),
                                  accept_remote_initial_value);
  } else {
    qWarning("Trying to create existed parameter %s", qUtf8Printable(name));
  }
  // uploadOptionValue(itr.first->second.get());
  return itr.first->second;
}

std::shared_ptr<NamedValue> Client::option(
    const QString& name,
    bool create_if_not_found,
    bool accept_remote_initial_value_if_created) {
  QReadLocker lock(&options_lock_);
  auto itr = options_.find(name);
  if (itr != options_.end())
    return itr->second;

  lock.unlock();

  if (create_if_not_found)
    return addOption(name, accept_remote_initial_value_if_created);
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
             qUtf8Printable(option->name()));
    return;
  }
  if (option->value().isUndefined()) {
    qCritical("Ignore uploading undefined option %s",
              qUtf8Printable(option->name()));
    return;
  }
  QMQTT::Message message(
      0,
      "side_assist/" + mqtt_client_->clientId() + "/option/" + option->name(),
      QJsonDocument(QJsonObject({qMakePair("value", option->value())}))
          .toJson(QJsonDocument::Compact),
      2, true);
  qInfo("Uploading option %s...", qUtf8Printable(option->name()));
  mqtt_client_->publish(message);
}

void Client::uploadOptionValidator(const NamedValue* option) {
  if (!mqtt_client_->isConnectedToHost()) {
    qWarning("Trying to upload validator of option %s when not connected",
             qUtf8Printable(option->name()));
    return;
  }
  QByteArray buf;
  if (option->validator() != nullptr) {
    buf = QJsonDocument(
              QJsonObject({qMakePair("validator",
                                     option->validator()->serializeToJson())}))
              .toJson(QJsonDocument::Compact);
  }
  QMQTT::Message message(0,
                         "side_assist/" + mqtt_client_->clientId() +
                             "/option/" + option->name() + "/validator",
                         buf, 2, true);
  qInfo("Uploading validator for option %s...", qUtf8Printable(option->name()));
  mqtt_client_->publish(message);
}

void Client::unsubscribeInitialValueWhenOptionIsNotUndefined() {
  if (!mqtt_client_->isConnectedToHost())
    return;
  const auto* opt = dynamic_cast<const NamedValue*>(sender());
  assert(opt != nullptr);
  assert(options_.find(opt->name()) != options_.end());
  if (!opt->value().isUndefined()) {
    auto sync_topic =
        "side_assist/" + mqtt_client_->clientId() + "/option/" + opt->name();
    mqtt_client_->unsubscribe(sync_topic);
    disconnect(opt, &NamedValue::valueChanged, this,
               &Client::unsubscribeInitialValueWhenOptionIsNotUndefined);
  }
}

void Client::setupSubscriptionsForOption(const NamedValue* option,
                                         bool accept_remote_initial_value) {
  if (!mqtt_client_->isConnectedToHost())
    return;
  auto sync_topic =
      "side_assist/" + mqtt_client_->clientId() + "/option/" + option->name();
  auto remote_set_topic = sync_topic + "/set";
  mqtt_client_->subscribe(remote_set_topic, 1);

  if (accept_remote_initial_value) {
    mqtt_client_->subscribe(sync_topic, 2);
    connect(option, &NamedValue::valueChanged, this,
            &Client::unsubscribeInitialValueWhenOptionIsNotUndefined);
  }
}

}  // namespace SideAssist::Qt
