#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "value_validator.hpp"

namespace SideAssist::Qt {

bool PathValueValidator::validate(const QJsonValue& value) const noexcept {
  if (!value.isString())
    return false;
  QFileInfo info(value.toString());
  if (existance_ != PathExistanceFieldEnum::Undefined &&
      (info.exists() ^ (existance_ == PathExistanceFieldEnum::Exist)))
    return false;
  PathPermissionField perm = PathPermissionFieldEnum::None;
  if (info.isReadable())
    perm |= PathPermissionFieldEnum::Readable;
  if (info.isWritable())
    perm |= PathPermissionFieldEnum::Writable;
  if (info.isExecutable())
    perm |= PathPermissionFieldEnum::Executable;
  if (!perm.between(min_perm_, max_perm_))
    return false;
  PathTypeField type = PathTypeFieldEnum::Undefined;
  if (info.isFile())
    type |= PathTypeFieldEnum::File;
  if (info.isDir())
    type |= PathTypeFieldEnum::Dir;
  if (!type.between(min_type_, max_type_))
    return false;
  return true;
}

QJsonValue PathValueValidator::serializeToJson() const noexcept {
  QJsonObject obj;
  if (existance_ != PathExistanceFieldEnum::Undefined)
    obj.insert("existance", existance_ == PathExistanceFieldEnum::Exist);
  if (min_perm_ != PathPermissionFieldEnum::None ||
      max_perm_ != PathPermissionFieldEnum::All)
    obj.insert("perm", QJsonArray({QJsonValue((qint64)min_perm_),
                                   QJsonValue((qint64)max_perm_)}));
  if (min_type_ != PathTypeFieldEnum::Undefined ||
      max_type_ != PathTypeFieldEnum::All)
    obj.insert("type", QJsonArray({QJsonValue((qint64)min_type_),
                                   QJsonValue((qint64)max_type_)}));
  return QJsonObject({qMakePair("path", obj)});
}

std::shared_ptr<PathValueValidator> PathValueValidator::deserializeFromJson(
    const QJsonValue& validator,
    bool* is_this_type) {
  auto val = validator["path"];
  if (!val.isObject())
    return nullptr;
  auto obj = val.toObject();
  if (is_this_type != nullptr)
    *is_this_type = true;

  PathExistanceField exist = PathExistanceFieldEnum::Undefined;
  int validated_fields = 0;
  if (obj.contains("existance")) {
    exist = obj["existance"].toBool() ? PathExistanceFieldEnum::Exist
                                      : PathExistanceFieldEnum::Nonexist;
  }

  PathPermissionField min_perm = PathPermissionFieldEnum::None,
                  max_perm = PathPermissionFieldEnum::All;
  auto perm_val = obj["perm"];
  if (perm_val.isArray()) {
    auto perm = perm_val.toArray();
    if (perm.count() != 2) {
      qCritical("Permission field contains not exactly 2 items");
      qDebug("Json: %s", QJsonDocument(obj).toJson().constData());
      return nullptr;
    }
    min_perm = perm[0].toInteger(INT64_MAX);
    max_perm = perm[1].toInteger(0);
    if (!min_perm.between(0, PathPermissionFieldEnum::All) ||
        !max_perm.between(min_perm, PathPermissionFieldEnum::All)) {
      qCritical("Permission field contains invalid ramge");
      qDebug("Json: %s", QJsonDocument(obj).toJson().constData());
      return nullptr;
    }
  }

  PathTypeField min_type = PathTypeFieldEnum::Undefined,
            max_type = PathTypeFieldEnum::All;
  auto type_val = obj["type"];
  if (type_val.isArray()) {
    auto type = type_val.toArray();
    if (type.count() != 2) {
      qCritical("Type field contains not exactly 2 items");
      qDebug("Json: %s", QJsonDocument(obj).toJson().constData());
      return nullptr;
    }
    min_type = type[0].toInteger(INT64_MAX);
    max_type = type[1].toInteger(0);
    if (!min_type.between(0, PathTypeFieldEnum::All) ||
        !max_type.between(min_type, PathTypeFieldEnum::All)) {
      qCritical("Type field contains invalid ramge");
      qDebug("Json: %s", QJsonDocument(obj).toJson().constData());
      return nullptr;
    }
  }
  return std::make_shared<PathValueValidator>(exist, min_perm, max_perm,
                                              min_type, max_type);
}

}  // namespace SideAssist::Qt
