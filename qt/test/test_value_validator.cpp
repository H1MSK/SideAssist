#include <gtest/gtest.h>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include "value_validator.hpp"

TEST(ValueValidator, Dummy) {
  auto ptr = SideAssist::Qt::ValueValidator::Abstract::deserializeFromJson(
      QJsonObject({qMakePair("dummy", QJsonValue())}));
  EXPECT_NE(ptr, nullptr);
  bool ret = ptr->validate(QJsonValue());
  EXPECT_TRUE(ret);
  auto val = ptr->serializeToJson();
  bool is_this_type = false;
  auto ptr1 = SideAssist::Qt::ValueValidator::Dummy::deserializeFromJson(
      val, &is_this_type);
  EXPECT_TRUE(is_this_type);
}

TEST(ValueValidator, Type) {
  auto ptr = SideAssist::Qt::ValueValidator::Abstract::deserializeFromJson(
      QJsonObject({qMakePair(
          "types", QJsonArray({QJsonValue("Null"), QJsonValue("Integer")}))}));
  EXPECT_NE(ptr, nullptr);

  bool ret = ptr->validate(QJsonValue());
  EXPECT_TRUE(ret);
  ret = ptr->validate(QJsonValue(true));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(2.3));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(12748941));
  EXPECT_TRUE(ret);
  ret = ptr->validate(QJsonValue("str"));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonArray());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonObject());
  EXPECT_FALSE(ret);

  auto val = ptr->serializeToJson();
  bool is_this_type = false;
  auto ptr1 = SideAssist::Qt::ValueValidator::Types::deserializeFromJson(
      val, &is_this_type);
  EXPECT_TRUE(is_this_type);
}

TEST(ValueValidator, FilePath) {
  auto ptr = SideAssist::Qt::ValueValidator::Abstract::deserializeFromJson(
      QJsonObject({qMakePair(
          "path",
          QJsonObject(
              {qMakePair("existance", true),
               qMakePair(
                   "type",
                   QJsonArray(
                       {QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathTypeFieldEnum::File),
                        QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathTypeFieldEnum::File)}))}))}));
  EXPECT_NE(ptr, nullptr);

  bool ret = ptr->validate(QJsonValue());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(true));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(2.3));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(12748941));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue("str"));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonArray());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonObject());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(__FILE__));
  EXPECT_TRUE(ret);
  ret = ptr->validate(
      QJsonValue(QFileInfo(__FILE__).absoluteDir().canonicalPath()));
  EXPECT_FALSE(ret);

  auto val = ptr->serializeToJson();
  bool is_this_type = false;
  auto ptr1 = SideAssist::Qt::ValueValidator::Path::deserializeFromJson(
      val, &is_this_type);
  EXPECT_TRUE(is_this_type);
}

TEST(ValueValidator, DirPath) {
  auto ptr = SideAssist::Qt::ValueValidator::Abstract::deserializeFromJson(
      QJsonObject({qMakePair(
          "path",
          QJsonObject(
              {qMakePair("existance", true),
               qMakePair(
                   "type",
                   QJsonArray(
                       {QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathTypeFieldEnum::Dir),
                        QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathTypeFieldEnum::Dir)}))}))}));
  EXPECT_NE(ptr, nullptr);

  bool ret = ptr->validate(QJsonValue());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(true));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(2.3));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(12748941));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue("str"));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonArray());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonObject());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(__FILE__));
  EXPECT_FALSE(ret);
  ret = ptr->validate(
      QJsonValue(QFileInfo(__FILE__).absoluteDir().canonicalPath()));
  EXPECT_TRUE(ret);

  auto val = ptr->serializeToJson();
  bool is_this_type = false;
  auto ptr1 = SideAssist::Qt::ValueValidator::Path::deserializeFromJson(
      val, &is_this_type);
  EXPECT_TRUE(is_this_type);
}

TEST(ValueValidator, ReadOnlyFilePath) {
  auto ptr = SideAssist::Qt::ValueValidator::Abstract::deserializeFromJson(
      QJsonObject({qMakePair(
          "path",
          QJsonObject(
              {qMakePair("existance", true),
               qMakePair(
                   "type",
                   QJsonArray(
                       {QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathTypeFieldEnum::File),
                        QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathTypeFieldEnum::File)})),
               qMakePair(
                   "perm",
                   QJsonArray(
                       {QJsonValue((qint64)SideAssist::Qt::ValueValidator::
                                       PathPermissionFieldEnum::None),
                        QJsonValue(
                            (qint64)SideAssist::Qt::ValueValidator::
                                PathPermissionFieldEnum::Readable)}))}))}));
  EXPECT_NE(ptr, nullptr);

  bool ret = ptr->validate(QJsonValue());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(true));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(2.3));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(12748941));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue("str"));
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonArray());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonObject());
  EXPECT_FALSE(ret);
  ret = ptr->validate(QJsonValue(__FILE__));
  EXPECT_FALSE(ret);
  ret = ptr->validate(
      QJsonValue(QFileInfo(__FILE__).absoluteDir().canonicalPath()));
  EXPECT_FALSE(ret);

  auto val = ptr->serializeToJson();
  bool is_this_type = false;
  auto ptr1 = SideAssist::Qt::ValueValidator::Path::deserializeFromJson(
      val, &is_this_type);
  EXPECT_TRUE(is_this_type);
}
