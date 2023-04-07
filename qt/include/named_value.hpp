#pragma once
#include <QJsonValue>
#include <QObject>
#include "global.hpp"

namespace SideAssist::Qt {

namespace ValueValidator {
class Abstract;
}  // namespace ValueValidator

class Q_SIDEASSIST_EXPORT NamedValue : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name MEMBER name_ CONSTANT)
  Q_PROPERTY(QJsonValue value READ value MEMBER value_ WRITE setValue NOTIFY
                 valueChanged)

 public:
  const QString& name() const { return name_; }
  const QJsonValue& value() const { return value_; }
  const std::shared_ptr<ValueValidator::Abstract>& validator() const {
    return validator_;
  }

  bool validate(const QJsonValue& val);

  NamedValue(const QString& name, const QJsonValue& value)
      : name_(name), value_(value) {}
  NamedValue(QString&& name, QJsonValue&& value) : name_(name), value_(value) {}
  NamedValue(const NamedValue& other) : NamedValue(other.name_, other.value_) {}
  NamedValue(NamedValue&& other)
      : NamedValue(std::move(other.name_), std::move(other.value_)) {}

 public slots:
  void setValue(const QJsonValue& value) {
    if (value == value_)
      return;
    value_ = value;
    emit valueChanged(value_);
  }
  void setValidator(std::shared_ptr<ValueValidator::Abstract> validator) {
    if (validator == validator_)
      return;
    validator_ = validator;
    emit validatorChanged(validator);
  }

 signals:
  void valueChanged(const QJsonValue& value);
  void validatorChanged(
      const std::shared_ptr<ValueValidator::Abstract>& validator);

 private:
  const QString name_;
  QJsonValue value_;
  std::shared_ptr<ValueValidator::Abstract> validator_;
};

}  // namespace SideAssist::Qt
