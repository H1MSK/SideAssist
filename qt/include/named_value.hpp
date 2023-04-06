#pragma once
#include <QJsonValue>
#include <QObject>
#include "global.hpp"

namespace SideAssist::Qt {

class Q_SIDEASSIST_EXPORT NamedValue : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString name READ name MEMBER name_ CONSTANT)
  Q_PROPERTY(QJsonValue value READ value MEMBER value_ WRITE setValue NOTIFY
                 valueChanged)

 public:
  const QString& name() const { return name_; }
  const QJsonValue& value() const { return value_; }

  bool validate(const QJsonValue& val) const { return true; }

  NamedValue(const QString& name, const QJsonValue& value)
      : name_(name), value_(value) {}
  NamedValue(QString&& name, QJsonValue&& value) : name_(name), value_(value) {}
  NamedValue(const NamedValue& other) : NamedValue(other.name_, other.value_) {}
  NamedValue(NamedValue&& other)
      : NamedValue(std::move(other.name_), std::move(other.value_)) {}

 public slots:
  bool setValue(const QJsonValue& value) {
    if (value == value_)
      return true;
    if (!validate(value))
      return false;
    value_ = value;
    emit valueChanged(value_);
    return true;
  }

 signals:
  void valueChanged(const QJsonValue& value);

 private:
  const QString name_;
  QJsonValue value_;
};

}  // namespace SideAssist::Qt
