#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt::ValueValidator {

bool SingleType::validate(
    const QJsonValue& value) const noexcept {
  if (type_ == ValueTypeFieldEnum::Integer)
    return value.toInteger(-1) != -1 || value.toInteger(0) != 0;
  return typeToField(value.type()) == type_;
}

QJsonValue SingleType::serializeToJson() const noexcept {
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

std::shared_ptr<SingleType>
SingleType::deserializeFromJson(const QJsonValue& validator,
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

  return std::make_shared<SingleType>(type);
}

bool Types::validate(const QJsonValue& value) const noexcept {
  if ((type_field_ & ValueTypeFieldEnum::Null) && value.isNull())
    return true;
  if ((type_field_ & ValueTypeFieldEnum::Bool) && value.isBool())
    return true;
  if ((type_field_ & ValueTypeFieldEnum::Integer) &&
      (value.toInteger(-1) != -1 || value.toInteger(0) != 0))
    return true;
  if ((type_field_ & ValueTypeFieldEnum::Double) && value.isDouble())
    return true;
  if ((type_field_ & ValueTypeFieldEnum::String) && value.isString())
    return true;
  if ((type_field_ & ValueTypeFieldEnum::Array) && value.isArray())
    return true;
  if ((type_field_ & ValueTypeFieldEnum::Object) && value.isObject())
    return true;
  return false;
}

QJsonValue Types::serializeToJson() const noexcept {
  QJsonArray arr;
  if (type_field_ & ValueTypeFieldEnum::Null)
    arr.append("Null");
  if (type_field_ & ValueTypeFieldEnum::Bool)
    arr.append("Bool");
  if (type_field_ & ValueTypeFieldEnum::Integer)
    arr.append("Integer");
  if (type_field_ & ValueTypeFieldEnum::Double)
    arr.append("Double");
  if (type_field_ & ValueTypeFieldEnum::String)
    arr.append("String");
  if (type_field_ & ValueTypeFieldEnum::Array)
    arr.append("Array");
  if (type_field_ & ValueTypeFieldEnum::Object)
    arr.append("Object");
  return QJsonObject({qMakePair("types", arr)});
}

std::shared_ptr<Types> Types::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto obj = validator["types"];
  if (!obj.isArray())
    return nullptr;

  if (is_this_type != nullptr)
    *is_this_type = true;

  ValueTypeField f = ValueTypeFieldEnum::Undefined;
  for (auto item : obj.toArray()) {
    bool success = true;
    do {
      if (!item.isString()) {
        success = false;
        break;
      }
      QString type = item.toString();
      if (type == "Null")
        f |= ValueTypeFieldEnum::Null;
      else if (type == "Bool")
        f |= ValueTypeFieldEnum::Bool;
      else if (type == "Integer")
        f |= ValueTypeFieldEnum::Integer;
      else if (type == "Double")
        f |= ValueTypeFieldEnum::Double;
      else if (type == "String")
        f |= ValueTypeFieldEnum::String;
      else if (type == "Array")
        f |= ValueTypeFieldEnum::Array;
      else if (type == "Object")
        f |= ValueTypeFieldEnum::Object;
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
  if (f == ValueTypeFieldEnum::Undefined)
    return nullptr;
  return std::make_shared<Types>(f);
}

}  // namespace SideAssist::Qt::ValueValidator
