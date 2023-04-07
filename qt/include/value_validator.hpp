#pragma once

#include <QJsonValue>
#include <list>
#include <set>
#include "field_enum.hpp"
#include "global.hpp"

namespace SideAssist::Qt::ValueValidator {

namespace Internal {}  // namespace Internal

class Q_SIDEASSIST_EXPORT Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept = 0;
  virtual QJsonValue serializeToJson() const noexcept = 0;
  static std::shared_ptr<Abstract> deserializeFromJson(
      const QJsonValue& validator);
  constexpr explicit Abstract() noexcept {}
  constexpr Abstract(const Abstract&) noexcept = default;
  constexpr Abstract(Abstract&&) noexcept = default;
};

class Q_SIDEASSIST_EXPORT Dummy : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  Dummy() = default;
  Dummy(const Dummy&) = default;
  Dummy(Dummy&&) = default;

  static std::shared_ptr<Dummy> deserializeFromJson(const QJsonValue& validator,
                                                    bool* is_this_type);
};

enum class ValueTypeFieldEnum {
  Undefined = 0x00,
  Null = 0x01,
  Bool = 0x02,
  Integer = 0x04,
  Double = 0x08,
  String = 0x10,
  Array = 0x20,
  Object = 0x40,
};
using ValueTypeField = FieldEnum<ValueTypeFieldEnum>;

class Q_SIDEASSIST_EXPORT SingleType : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;
  static std::shared_ptr<SingleType> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  constexpr SingleType(QJsonValue::Type type) noexcept
      : type_(typeToField(type)) {}
  constexpr SingleType(ValueTypeField field) : type_(field) {
    // Has more than 1 bit set
    if ((ValueTypeField::NumericType(field) & (field - 1)) != 0)
      throw std::invalid_argument("Field should contain only one bit");
  }

  constexpr SingleType(const SingleType&) noexcept = default;
  constexpr SingleType(SingleType&&) noexcept = default;

  constexpr inline static ValueTypeField typeToField(
      QJsonValue::Type type) noexcept {
    switch (type) {
      case QJsonValue::Null:
        return ValueTypeFieldEnum::Null;
      case QJsonValue::Bool:
        return ValueTypeFieldEnum::Bool;
      case QJsonValue::Double:
        return ValueTypeFieldEnum::Double;
      case QJsonValue::String:
        return ValueTypeFieldEnum::String;
      case QJsonValue::Array:
        return ValueTypeFieldEnum::Array;
      case QJsonValue::Object:
        return ValueTypeFieldEnum::Object;
      default:
        return ValueTypeFieldEnum::Undefined;
    }
  }

 private:
  ValueTypeField type_;
};

class Q_SIDEASSIST_EXPORT Types : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;
  static std::shared_ptr<Types> deserializeFromJson(const QJsonValue& validator,
                                                    bool* is_this_type);

  constexpr Types(ValueTypeField field) noexcept : type_field_(field) {}

  constexpr Types(QJsonValue::Type type) noexcept
      : type_field_(SingleType::typeToField(type)) {}
  constexpr Types(std::initializer_list<QJsonValue::Type> types) noexcept
      : type_field_(typesToField(types)) {}
  constexpr Types(const Types&) noexcept = default;
  constexpr Types(Types&&) noexcept = default;

  constexpr inline static ValueTypeField typesToField(
      std::initializer_list<QJsonValue::Type> types) noexcept {
    ValueTypeField f = ValueTypeFieldEnum::Undefined;
    for (auto type : types)
      f |= SingleType::typeToField(type);
    return f;
  }

 private:
  ValueTypeField type_field_;
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

class Q_SIDEASSIST_EXPORT Path : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;
  static std::shared_ptr<Path> deserializeFromJson(const QJsonValue& validator,
                                                   bool* is_this_type);

  Path(PathExistanceField exist,
       PathPermissionField min_perm = PathPermissionFieldEnum::None,
       PathPermissionField max_perm = PathPermissionFieldEnum::All,
       PathTypeField min_type = PathTypeFieldEnum::Undefined,
       PathTypeField max_type = PathTypeFieldEnum::All)
      : existance_(exist),
        min_perm_(min_perm),
        max_perm_(max_perm),
        min_type_(min_type),
        max_type_(max_type) {}
  Path(PathExistanceField exist, PathTypeField min_type, PathTypeField max_type)
      : Path(exist,
             PathPermissionFieldEnum::None,
             PathPermissionFieldEnum::All,
             min_type,
             max_type) {}
  Path(PathPermissionField min_perm,
       PathPermissionField max_perm,
       PathTypeField min_type = PathTypeFieldEnum::Undefined,
       PathTypeField max_type = PathTypeFieldEnum::All)
      : Path(PathExistanceFieldEnum::Undefined,
             min_perm,
             max_perm,
             min_type,
             max_type) {}
  Path(PathTypeField min_type, PathTypeField max_type)
      : Path(PathExistanceFieldEnum::Undefined,
             PathPermissionFieldEnum::None,
             PathPermissionFieldEnum::All,
             min_type,
             max_type) {}
  Path(const Path&) = default;
  Path(Path&&) = default;

 private:
  PathExistanceField existance_;
  PathPermissionField min_perm_, max_perm_;
  PathTypeField min_type_, max_type_;
};

class Q_SIDEASSIST_EXPORT Option : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  Option(const std::set<QString>& options) : options_(options) {}
  Option(std::set<QString>&& options) : options_(options) {}
  Option(const Option&) = default;
  Option(Option&&) = default;

  static std::shared_ptr<Option> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

  std::set<QString> options_;
};

class Q_SIDEASSIST_EXPORT StringPrefix : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  StringPrefix(const std::set<QString>& prefix) : prefixes_(prefix) {}
  StringPrefix(std::set<QString>&& prefix) : prefixes_(prefix) {}
  StringPrefix(StringPrefix&&) = default;

  static std::shared_ptr<StringPrefix> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

 private:
  std::set<QString> prefixes_;
};

class Q_SIDEASSIST_EXPORT StringSuffix : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  StringSuffix(const std::set<QString>& suffix) : suffixes_(suffix) {}
  StringSuffix(std::set<QString>&& suffix) : suffixes_(suffix) {}
  StringSuffix(StringSuffix&&) = default;

  static std::shared_ptr<StringSuffix> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

 private:
  std::set<QString> suffixes_;
};

class Q_SIDEASSIST_EXPORT AbstractArray : public Abstract {
 public:
  AbstractArray(const std::list<std::shared_ptr<Abstract>>& validators)
      : validators_(validators) {}
  AbstractArray(std::list<std::shared_ptr<Abstract>>&& validators)
      : validators_(validators) {}
  AbstractArray(const AbstractArray&) = default;
  AbstractArray(AbstractArray&&) = default;

  static std::list<std::shared_ptr<Abstract>> deserializeListFromJsonArray(
      const QJsonValue& validator);

  auto begin() const { return validators_.begin(); }
  auto end() const { return validators_.end(); }

 private:
  std::list<std::shared_ptr<Abstract>> validators_;
};

class Q_SIDEASSIST_EXPORT Any : public AbstractArray {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  using AbstractArray::AbstractArray;

  static std::shared_ptr<Any> deserializeFromJson(const QJsonValue& validator,
                                                  bool* is_this_type);
};

class Q_SIDEASSIST_EXPORT All : public AbstractArray {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  using AbstractArray::AbstractArray;

  static std::shared_ptr<All> deserializeFromJson(const QJsonValue& validator,
                                                  bool* is_this_type);
};

class Q_SIDEASSIST_EXPORT ListItem : public Abstract {
 public:
  virtual bool validate(const QJsonValue& value) const noexcept final;
  virtual QJsonValue serializeToJson() const noexcept final;

  ListItem(const std::shared_ptr<Abstract>& item_validator)
      : item_validator_(item_validator) {}
  ListItem(std::shared_ptr<Abstract>&& item_validator)
      : item_validator_(std::move(item_validator)) {}
  ListItem(const ListItem&) = default;
  ListItem(ListItem&&) = default;

  static std::shared_ptr<ListItem> deserializeFromJson(
      const QJsonValue& validator,
      bool* is_this_type);

 private:
  std::shared_ptr<Abstract> item_validator_;
};

}  // namespace SideAssist::Qt::ValueValidator