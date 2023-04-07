#pragma once

#include <QJsonValue>
#include <set>
#include "field_enum.hpp"
#include "global.hpp"

namespace SideAssist::Qt {

namespace Internal {}  // namespace Internal

class Q_SIDEASSIST_EXPORT AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept = 0;
  virtual QJsonValue serializeToJson() const noexcept = 0;
  static std::shared_ptr<AbstractValueValidator> deserializeFromJson(
      const QJsonValue& validator);
  constexpr explicit AbstractValueValidator() noexcept {}
  constexpr AbstractValueValidator(const AbstractValueValidator&) noexcept =
      default;
  constexpr AbstractValueValidator(AbstractValueValidator&&) noexcept = default;
};

class Q_SIDEASSIST_EXPORT DummyValueValidator : public AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  DummyValueValidator() = default;
  DummyValueValidator(const DummyValueValidator&) = default;
  DummyValueValidator(DummyValueValidator&&) = default;

  static std::shared_ptr<DummyValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);
};

enum class JsonTypeFieldEnum {
  Undefined = 0x00,
  Null = 0x01,
  Bool = 0x02,
  Integer = 0x04,
  Double = 0x08,
  String = 0x10,
  Array = 0x20,
  Object = 0x40,
};
using JsonTypeField = FieldEnum<JsonTypeFieldEnum>;

class Q_SIDEASSIST_EXPORT TypeValueValidator : public AbstractValueValidator {
 public:

  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;
  static std::shared_ptr<TypeValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  constexpr TypeValueValidator(JsonTypeField field) noexcept : type_field_(field) {}

  constexpr TypeValueValidator(QJsonValue::Type type) noexcept
      : type_field_(singleTypeToField(type)) {}
  constexpr TypeValueValidator(
      std::initializer_list<QJsonValue::Type> types) noexcept
      : type_field_(typesToField(types)) {}
  constexpr TypeValueValidator(const TypeValueValidator&) noexcept = default;
  constexpr TypeValueValidator(TypeValueValidator&&) noexcept = default;

 protected:
  constexpr inline static JsonTypeField singleTypeToField(
      QJsonValue::Type type) noexcept {
    switch (type) {
      case QJsonValue::Null:
        return JsonTypeFieldEnum::Null;
      case QJsonValue::Bool:
        return JsonTypeFieldEnum::Bool;
      case QJsonValue::Double:
        return JsonTypeFieldEnum::Double;
      case QJsonValue::String:
        return JsonTypeFieldEnum::String;
      case QJsonValue::Array:
        return JsonTypeFieldEnum::Array;
      case QJsonValue::Object:
        return JsonTypeFieldEnum::Object;
      default:
        return JsonTypeFieldEnum::Undefined;
    }
  }
  constexpr inline static JsonTypeField typesToField(
      std::initializer_list<QJsonValue::Type> types) noexcept {
    JsonTypeField f = JsonTypeFieldEnum::Undefined;
    for (auto type : types)
      f |= singleTypeToField(type);
    return f;
  }

 private:
  JsonTypeField type_field_;
};

enum class PathExistanceFieldEnum {
  Undefined = 0,
  Nonexist = 0x01,
  Exist = 0x02
};
enum class PathPermissionFieldEnum {
  None = 0,
  Executable = 0x01,
  Readable = 0x02,
  Writable = 0x04,
  All = 0x07
};
enum class PathTypeFieldEnum {
  Undefined = 0,
  File = 0x01,
  Dir = 0x02,
  All = 0x03
};
using PathExistanceField = FieldEnum<PathExistanceFieldEnum>;
using PathPermissionField = FieldEnum<PathPermissionFieldEnum>;
using PathTypeField = FieldEnum<PathTypeFieldEnum>;

class Q_SIDEASSIST_EXPORT PathValueValidator : public AbstractValueValidator {
 public:

  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;
  static std::shared_ptr<PathValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  PathValueValidator(PathExistanceField exist,
                     PathPermissionField min_perm = PathPermissionFieldEnum::None,
                     PathPermissionField max_perm = PathPermissionFieldEnum::All,
                     PathTypeField min_type = PathTypeFieldEnum::Undefined,
                     PathTypeField max_type = PathTypeFieldEnum::All)
      : existance_(exist),
        min_perm_(min_perm),
        max_perm_(max_perm),
        min_type_(min_type),
        max_type_(max_type) {}
  PathValueValidator(PathExistanceField exist,
                     PathTypeField min_type,
                     PathTypeField max_type)
      : PathValueValidator(exist,
                           PathPermissionFieldEnum::None,
                           PathPermissionFieldEnum::All,
                           min_type,
                           max_type) {}
  PathValueValidator(PathPermissionField min_perm,
                     PathPermissionField max_perm,
                     PathTypeField min_type = PathTypeFieldEnum::Undefined,
                     PathTypeField max_type = PathTypeFieldEnum::All)
      : PathValueValidator(PathExistanceFieldEnum::Undefined,
                           min_perm,
                           max_perm,
                           min_type,
                           max_type) {}
  PathValueValidator(PathTypeField min_type, PathTypeField max_type)
      : PathValueValidator(PathExistanceFieldEnum::Undefined,
                           PathPermissionFieldEnum::None,
                           PathPermissionFieldEnum::All,
                           min_type,
                           max_type) {}
  PathValueValidator(const PathValueValidator&) = default;
  PathValueValidator(PathValueValidator&&) = default;

 private:
  PathExistanceField existance_;
  PathPermissionField min_perm_, max_perm_;
  PathTypeField min_type_, max_type_;
};

class Q_SIDEASSIST_EXPORT OptionValueValidator : public AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  OptionValueValidator(const std::set<QString>& options) : options_(options) {}
  OptionValueValidator(std::set<QString>&& options) : options_(options) {}
  OptionValueValidator(const OptionValueValidator&) = default;
  OptionValueValidator(OptionValueValidator&&) = default;

  static std::shared_ptr<OptionValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  std::set<QString> options_;
};

class Q_SIDEASSIST_EXPORT StringPrefixValueValidator
    : public AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  StringPrefixValueValidator(const std::set<QString>& prefix)
      : prefixes_(prefix) {}
  StringPrefixValueValidator(std::set<QString>&& prefix) : prefixes_(prefix) {}
  StringPrefixValueValidator(StringPrefixValueValidator&&) = default;

  static std::shared_ptr<StringPrefixValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

 private:
  std::set<QString> prefixes_;
};

class Q_SIDEASSIST_EXPORT StringSuffixValueValidator
    : public AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  StringSuffixValueValidator(const std::set<QString>& suffix)
      : suffixes_(suffix) {}
  StringSuffixValueValidator(std::set<QString>&& suffix) : suffixes_(suffix) {}
  StringSuffixValueValidator(StringSuffixValueValidator&&) = default;

  static std::shared_ptr<StringSuffixValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

 private:
  std::set<QString> suffixes_;
};

class Q_SIDEASSIST_EXPORT UnionValueValidator : public AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  UnionValueValidator(
      const std::set<std::shared_ptr<AbstractValueValidator>>& validators)
      : validators_(validators) {}
  UnionValueValidator(
      std::set<std::shared_ptr<AbstractValueValidator>>&& validators)
      : validators_(validators) {}
  UnionValueValidator(const UnionValueValidator&) = default;
  UnionValueValidator(UnionValueValidator&&) = default;

  static std::shared_ptr<UnionValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  std::set<std::shared_ptr<AbstractValueValidator>> validators_;
};

class Q_SIDEASSIST_EXPORT JoinValueValidator : public AbstractValueValidator {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  JoinValueValidator(
      const std::set<std::shared_ptr<AbstractValueValidator>>& validators)
      : validators_(validators) {}
  JoinValueValidator(
      std::set<std::shared_ptr<AbstractValueValidator>>&& validators)
      : validators_(validators) {}
  JoinValueValidator(const JoinValueValidator&) = default;
  JoinValueValidator(JoinValueValidator&&) = default;

  static std::shared_ptr<JoinValueValidator> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  std::set<std::shared_ptr<AbstractValueValidator>> validators_;
};

}  // namespace SideAssist::Qt