#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt::ValueValidator {

bool StringPrefix::validate(
    const QJsonValue& value) const noexcept {
  if (!value.isString())
    return false;
  QString str = value.toString();
  for (const auto& prefix : prefixes_) {
    if (str.left(prefix.length()) == prefix)
      return true;
  }
  return false;
}

QJsonValue StringPrefix::serializeToJson() const noexcept {
  return QJsonObject({qMakePair("prefix", QJsonValue(QJsonValue::Null))});
}

std::shared_ptr<StringPrefix>
StringPrefix::deserializeFromJson(const QJsonValue& validator,
                                                bool* is_this_type) {
  auto prefix_val = validator["prefix"];
  if (!prefix_val.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;
  std::set<QString> prefixes;
  for (const auto& item : prefix_val.toArray()) {
    if (!item.isString()) {
      qWarning("String array contains invalid item: %s",
               QJsonDocument(QJsonArray({item}))
                   .toJson(QJsonDocument::Compact)
                   .constData());
    }
    prefixes.insert(item.toString());
  }

  if (prefixes.empty()) {
    qCritical("Option array is empty");
    return nullptr;
  }

  return std::make_shared<StringPrefix>(std::move(prefixes));
}

bool StringSuffix::validate(
    const QJsonValue& value) const noexcept {
  if (!value.isString())
    return false;
  QString str = value.toString();
  for (const auto& suffix : suffixes_) {
    if (str.right(suffix.length()) == suffix)
      return true;
  }
  return false;
}

QJsonValue StringSuffix::serializeToJson() const noexcept {
  return QJsonObject({qMakePair("suffix", QJsonValue(QJsonValue::Null))});
}

std::shared_ptr<StringSuffix>
StringSuffix::deserializeFromJson(const QJsonValue& validator,
                                                bool* is_this_type) {
  auto suffix_val = validator["suffix"];
  if (!suffix_val.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;
  std::set<QString> suffixes;
  for (const auto& item : suffix_val.toArray()) {
    if (!item.isString()) {
      qWarning("String array contains invalid item: %s",
               QJsonDocument(QJsonArray({item}))
                   .toJson(QJsonDocument::Compact)
                   .constData());
    }
    suffixes.insert(item.toString());
  }

  if (suffixes.empty()) {
    qCritical("Option array is empty");
    return nullptr;
  }

  return std::make_shared<StringSuffix>(std::move(suffixes));
}

}  // namespace SideAssist::Qt::ValueValidator
