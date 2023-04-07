#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt {

bool SingleTypeValueValidator::validate(
    const QJsonValue& value) const noexcept {
  if (type_ == JsonTypeFieldEnum::Integer)
    return value.toInteger(-1) != -1 || value.toInteger(0) != 0;
  return typeToField(value.type()) == type_;
}

QJsonValue SingleTypeValueValidator::serializeToJson() const noexcept {
  QString name;
  switch (type_) {
    case QJsonValue::Null:
      name = "Null";
      break;
    case QJsonValue::Bool:
      name = "Bool";
      break;
    case QJsonValue::Double:
      name = "Double";
      break;
    case QJsonValue::String:
      name = "String";
      break;
    case QJsonValue::Array:
      name = "Array";
      break;
    case QJsonValue::Object:
      name = "Object";
      break;
  }
  return QJsonObject({qMakePair("type", name)});
}

std::shared_ptr<SingleTypeValueValidator>
SingleTypeValueValidator::deserializeFromJson(const QJsonValue& validator,
                                              bool* is_this_type) {
  auto name_val = validator["types"];
  if (!name_val.isString())
    return nullptr;

  if (is_this_type != nullptr)
    *is_this_type = true;

  QString name = name_val.toString();

  QJsonValue::Type type;

  if (name == "Null")
    type = QJsonValue::Null;
  else if (name == "Bool")
    type = QJsonValue::Bool;
  else if (name == "Double")
    type = QJsonValue::Double;
  else if (name == "String")
    type = QJsonValue::String;
  else if (name == "Array")
    type = QJsonValue::Array;
  else if (name == "Object")
    type = QJsonValue::Object;

  return std::make_shared<SingleTypeValueValidator>(type);
}

bool TypesValueValidator::validate(const QJsonValue& value) const noexcept {
  if ((type_field_ & JsonTypeFieldEnum::Null) && value.isNull())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Bool) && value.isBool())
    return true;
  if ((type_field_ & JsonTypeFieldEnum::Integer) &&
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

QJsonValue TypesValueValidator::serializeToJson() const noexcept {
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
  return QJsonObject({qMakePair("types", arr)});
}

std::shared_ptr<TypesValueValidator> TypesValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto obj = validator["types"];
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
  return std::make_shared<TypesValueValidator>(f);
}

}  // namespace SideAssist::Qt
