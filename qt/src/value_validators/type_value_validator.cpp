#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt {

bool TypeValueValidator::validate(const QJsonValue& value) const noexcept {
  if ((type_field_ & JsonTypeFieldEnum::Null) && value.isNull())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Bool) && value.isBool())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Integer) && value.isDouble() &&
      (value.toInteger(-1) != -1 || value.toInteger(0) != 0))
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Double) && value.isDouble())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::String) && value.isString())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Array) && value.isArray())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Object) && value.isObject())
    return true;
  return false;
}

QJsonValue TypeValueValidator::serializeToJson() const noexcept {
  QJsonArray arr;
  if (type_field_ & JsonTypeFieldEnum::Null)
    arr.append("Null");
  if (type_field_ & JsonTypeFieldEnum::Bool)
    arr.append("Bool");
  if (type_field_ & JsonTypeFieldEnum::Integer)
    arr.append("Integer");
  if (type_field_ & JsonTypeFieldEnum::Double)
    arr.append("Double");
  if (type_field_ & JsonTypeFieldEnum::String)
    arr.append("String");
  if (type_field_ & JsonTypeFieldEnum::Array)
    arr.append("Array");
  if (type_field_ & JsonTypeFieldEnum::Object)
    arr.append("Object");
  return QJsonObject({qMakePair("type", arr)});
}

std::shared_ptr<TypeValueValidator> TypeValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto obj = validator["type"];
  if (!obj.isArray())
    return nullptr;

  if (is_this_type != nullptr)
    *is_this_type = true;

  JsonTypeField f = JsonTypeFieldEnum::Undefined;
  for (auto item : obj.toArray()) {
    bool success = true;
    do {
      if (!item.isString()) {
        success = false;
        break;
      }
      QString type = item.toString();
      if (type == "Null")
        f |= JsonTypeFieldEnum::Null;
      else if (type == "Bool")
        f |= JsonTypeFieldEnum::Bool;
      else if (type == "Integer")
        f |= JsonTypeFieldEnum::Integer;
      else if (type == "Double")
        f |= JsonTypeFieldEnum::Double;
      else if (type == "String")
        f |= JsonTypeFieldEnum::String;
      else if (type == "Array")
        f |= JsonTypeFieldEnum::Array;
      else if (type == "Object")
        f |= JsonTypeFieldEnum::Object;
      else
        success = false;
    } while (0);
    if (!success) {
      qWarning("Type array contains invalid item: %s",
               QJsonDocument(QJsonArray({item}))
                   .toJson(QJsonDocument::Compact)
                   .constData());
    }
  }
  if (f == JsonTypeFieldEnum::Undefined)
    return nullptr;
  return std::make_shared<TypeValueValidator>(f);
}

}  // namespace SideAssist::Qt
