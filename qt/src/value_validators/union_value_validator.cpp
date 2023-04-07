#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt::ValueValidator {

bool Any::validate(const QJsonValue& value) const noexcept {
  for (const auto& ptr : *this) {
    if (ptr->validate(value))
      return true;
  }
  return false;
}

QJsonValue Any::serializeToJson() const noexcept {
  QJsonArray arr;
  for (const auto& ptr : *this) {
    arr.append(ptr->serializeToJson());
  }
  return QJsonObject({qMakePair("any", arr)});
}

std::shared_ptr<Any> Any::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto any = validator["any"];
  if (!any.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;

  auto validators = deserializeListFromJsonArray(any);

  if (validators.empty()) {
    qCritical("Validator array is empty");
    return nullptr;
  }

  return std::make_shared<Any>(std::move(validators));
}

}  // namespace SideAssist::Qt::ValueValidator
