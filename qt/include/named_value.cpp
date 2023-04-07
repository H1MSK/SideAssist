#include "named_value.hpp"
#include "value_validator.hpp"

namespace SideAssist::Qt {
bool NamedValue::validate(const QJsonValue& val) {
  return validator_ == nullptr ? true : validator_->validate(val);
}

}  // namespace SideAssist::Qt
