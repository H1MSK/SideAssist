#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt {

bool OptionValueValidator::validate(const QJsonValue& value) const noexcept {
  return value.isString() && options_.find(value.toString()) != options_.end();
}

QJsonValue OptionValueValidator::serializeToJson() const noexcept {
  QJsonArray arr;
  for (const auto& str : options_)
    arr.append(str);
  return QJsonObject({qMakePair("options", arr)});
}

std::shared_ptr<OptionValueValidator> OptionValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto options = validator["options"];
  if (!options.isArray())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;
  std::set<QString> option_set;
  for (const auto& item : options.toArray()) {
    if (!item.isString()) {
      qWarning("String array contains invalid item: %s",
               QJsonDocument(QJsonArray({item}))
                   .toJson(QJsonDocument::Compact)
                   .constData());
    }
    option_set.insert(item.toString());
  }

  if (option_set.empty()) {
    qCritical("Option array is empty");
    return nullptr;
  }

  return std::make_shared<OptionValueValidator>(std::move(option_set));
}

}  // namespace SideAssist::Qt
