#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt {

bool UnionValueValidator::validate(const QJsonValue& value) const noexcept {
  for (const auto& ptr : validators_) {
    if (ptr->validate(value))
      return true;
  }
  return false;
}

QJsonValue UnionValueValidator::serializeToJson() const noexcept {
  QJsonArray arr;
  for (const auto& ptr : validators_) {
    arr.append(ptr->serializeToJson());
  }
  return QJsonObject({qMakePair("any", arr)});
}

std::shared_ptr<UnionValueValidator> UnionValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto any = validator["any"];
  if (!any.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;

  std::set<std::shared_ptr<AbstractValueValidator>> validators;

  for (const auto& item : any.toArray()) {
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

  return std::make_shared<UnionValueValidator>(std::move(validators));
}

}  // namespace SideAssist::Qt
