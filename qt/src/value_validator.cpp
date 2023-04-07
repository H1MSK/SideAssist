#include "value_validator.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace SideAssist::Qt::ValueValidator {

bool Dummy::validate(const QJsonValue& value) const noexcept {
  return true;
}

QJsonValue Dummy::serializeToJson() const noexcept {
  return QJsonObject({qMakePair("dummy", QJsonValue(QJsonValue::Null))});
}

std::shared_ptr<Dummy> Dummy::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto dummy = validator["dummy"];
  if (!dummy.isNull())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;
  return std::make_shared<Dummy>();
}

std::shared_ptr<Abstract>
Abstract::deserializeFromJson(const QJsonValue& validator) {
  bool matched = false;
  std::shared_ptr<Abstract> ptr;
  ptr = Dummy::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = Types::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = Path::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = Option::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = StringPrefix::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = StringSuffix::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = Any::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = All::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  return nullptr;
}

std::list<std::shared_ptr<Abstract>>
AbstractArray::deserializeListFromJsonArray(const QJsonValue& validator) {
  std::list<std::shared_ptr<Abstract>> list;
  for (const auto& item : validator.toArray()) {
    auto ptr = deserializeFromJson(item);
    if (ptr != nullptr) {
      list.push_back(ptr);
    } else {
      qWarning("Validator array contains invalid item: %s",
               QJsonDocument(QJsonArray({item}))
                   .toJson(QJsonDocument::Compact)
                   .constData());
    }
  }
  return list;
}

}  // namespace SideAssist::Qt::ValueValidator
