#include <QCoreApplication>
#include <memory>
#include "client.hpp"
#include "value_validator.hpp"

std::shared_ptr<SideAssist::Qt::Client> client;

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  client = std::make_shared<SideAssist::Qt::Client>();

  client->setClientId("echo");
  client->installDefaultMessageHandler();

  client->setUsername("side_assist_client");
  client->setPassword("16509490");

  client->connectToHost();

  auto int_opt = client->addOption("int");
  auto int_param = client->addParameter("int_param");
  int_opt->setValidator(std::make_shared<SideAssist::Qt::SingleTypeValueValidator>(
      SideAssist::Qt::JsonTypeFieldEnum::Integer));
  QObject::connect(int_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   int_param.get(), &SideAssist::Qt::NamedValue::setValue);
  int_opt->setValue(QJsonValue(123));

  auto plain_opt = client->addOption("plain");
  auto plain_param = client->addParameter("plain_param");
  plain_opt->setValidator(std::make_shared<SideAssist::Qt::TypesValueValidator>(
      SideAssist::Qt::JsonTypeField() |
      SideAssist::Qt::JsonTypeFieldEnum::Integer |
      SideAssist::Qt::JsonTypeFieldEnum::Bool |
      SideAssist::Qt::JsonTypeFieldEnum::Null |
      SideAssist::Qt::JsonTypeFieldEnum::Double));
  QObject::connect(plain_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   plain_param.get(), &SideAssist::Qt::NamedValue::setValue);

  auto option_opt = client->addOption("one_or_two_or_three");
  auto option_param = client->addParameter("one_or_two_or_three_param");
  option_opt->setValidator(
      std::make_shared<SideAssist::Qt::OptionValueValidator>(
          std::set<QString>{"One", "Two", "Three"}));
  QObject::connect(option_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   option_param.get(), &SideAssist::Qt::NamedValue::setValue);

  auto existed_folder_opt = client->addOption("existed_folder");
  auto existed_folder_param = client->addParameter("existed_folder_param");
  existed_folder_opt->setValidator(
      std::make_shared<SideAssist::Qt::PathValueValidator>(
          SideAssist::Qt::PathExistanceFieldEnum::Exist,
          SideAssist::Qt::PathTypeFieldEnum::Dir,
          SideAssist::Qt::PathTypeFieldEnum::Dir));
  QObject::connect(
      existed_folder_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
      existed_folder_param.get(), &SideAssist::Qt::NamedValue::setValue);

  auto nonexist_path_opt = client->addOption("nonexist_path");
  auto nonexist_path_param = client->addParameter("nonexist_path_param");
  nonexist_path_opt->setValidator(
      std::make_shared<SideAssist::Qt::PathValueValidator>(
          SideAssist::Qt::PathExistanceFieldEnum::Nonexist));
  QObject::connect(
      nonexist_path_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
      nonexist_path_param.get(), &SideAssist::Qt::NamedValue::setValue);

  return app.exec();
}
