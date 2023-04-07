#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt::ValueValidator {

bool All::validate(const QJsonValue& value) const noexcept {
  for (const auto& ptr : *this) {
    if (!ptr->validate(value))
      return false;
  }
  return true;
}

QJsonValue All::serializeToJson() const noexcept {
  QJsonArray arr;
  for (const auto& ptr : *this) {
    arr.append(ptr->serializeToJson());
  }
  return QJsonObject({qMakePair("all", arr)});
}

std::shared_ptr<All> All::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto all = validator["all"];
  if (!all.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;

  auto validators = deserializeListFromJsonArray(all);

  if (validators.empty()) {
    qCritical("Validator array is empty");
    return nullptr;
  }

  return std::make_shared<All>(std::move(validators));
}

}  // namespace SideAssist::Qt::ValueValidator
