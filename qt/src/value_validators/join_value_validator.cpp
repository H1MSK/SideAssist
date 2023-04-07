#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt {

bool JoinValueValidator::validate(const QJsonValue& value) const noexcept {
  for (const auto& ptr : validators_) {
    if (!ptr->validate(value))
      return false;
  }
  return true;
}

QJsonValue JoinValueValidator::serializeToJson() const noexcept {
  QJsonArray arr;
  for (const auto& ptr : validators_) {
    arr.append(ptr->serializeToJson());
  }
  return QJsonObject({qMakePair("all", arr)});
}

std::shared_ptr<JoinValueValidator> JoinValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto all = validator["all"];
  if (!all.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;

  std::set<std::shared_ptr<AbstractValueValidator>> validators;

  for (const auto& item : all.toArray()) {
    auto itr = AbstractValueValidator::deserializeFromJson(item);
    if (itr == nullptr) {
      qWarning("Validator array contains invalid item: %s",
               QJsonDocument(QJsonArray({item}))
                   .toJson(QJsonDocument::Compact)
                   .constData());
    }
    validators.insert(itr);
  }

  if (validators.empty()) {
    qCritical("Validator array is empty");
    return nullptr;
  }

  return std::make_shared<JoinValueValidator>(std::move(validators));
}

}  // namespace SideAssist::Qt
