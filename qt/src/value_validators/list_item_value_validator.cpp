#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt::ValueValidator {

bool ListItem::validate(const QJsonValue& value) const noexcept {
  if (!value.isArray()) return false;
  for (const auto& item :value.toArray()) {
    if (!item_validator_->validate(item))
      return false;
  }
  return true;
}

QJsonValue ListItem::serializeToJson() const noexcept {
  return QJsonObject({qMakePair("list", item_validator_->serializeToJson())});
}

std::shared_ptr<ListItem>
ListItem::deserializeFromJson(const QJsonValue& validator,
                                            bool* is_this_type) {
  auto all = validator["list"];
  if (all.isUndefined())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;

  auto item_validator = Abstract::deserializeFromJson(all);

  if (item_validator == nullptr) {
    qCritical("Item validator is invalid");
    return nullptr;
  }

  return std::make_shared<ListItem>(std::move(item_validator));
}

}  // namespace SideAssist::Qt::ValueValidator
