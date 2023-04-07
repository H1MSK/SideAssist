#include "value_validator.hpp"
#include <QJsonObject>

namespace SideAssist::Qt {

bool DummyValueValidator::validate(const QJsonValue& value) const noexcept {
  return true;
}

QJsonValue DummyValueValidator::serializeToJson() const noexcept {
  return QJsonObject({qMakePair("dummy", QJsonValue(QJsonValue::Null))});
}

std::shared_ptr<DummyValueValidator> DummyValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto dummy = validator["dummy"];
  if (!dummy.isNull())
    return nullptr;
  if (is_this_type != nullptr)
    *is_this_type = true;
  return std::make_shared<DummyValueValidator>();
}

std::shared_ptr<AbstractValueValidator>
AbstractValueValidator::deserializeFromJson(const QJsonValue& validator) {
  bool matched = false;
  std::shared_ptr<AbstractValueValidator> ptr;
  ptr = DummyValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = TypesValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = PathValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = OptionValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = StringPrefixValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = StringSuffixValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = UnionValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  ptr = JoinValueValidator::deserializeFromJson(validator, &matched);
  if (matched)
    return ptr;
  return nullptr;
}

}  // namespace SideAssist::Qt
