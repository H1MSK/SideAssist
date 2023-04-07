#include <QCoreApplication>
#include <memory>
#include "client.hpp"
#include "value_validator.hpp"

std::shared_ptr<SideAssist::Qt::Client> client;

int main(int argc, char* argv[]) {
  namespace Validator = SideAssist::Qt::ValueValidator;
  QCoreApplication app(argc, argv);
  client = std::make_shared<SideAssist::Qt::Client>();

  client->setClientId("echo");
  client->installDefaultMessageHandler();

  client->setUsername("side_assist_client");
  client->setPassword("16509490");

  client->connectToHost();

  auto int_opt = client->addOption("int");
  auto plain_opt = client->addOption("plain");
  auto option_opt = client->addOption("one_or_two_or_three");
  auto existed_folder_opt = client->addOption("existed_folder");
  auto nonexist_path_opt = client->addOption("nonexist_path");

  int_opt->setValidator(
      std::make_shared<Validator::SingleType>(
          Validator::ValueTypeFieldEnum::Integer));
  plain_opt->setValidator(
      std::make_shared<Validator::Types>(
          Validator::ValueTypeField() |
          Validator::ValueTypeFieldEnum::Integer |
          Validator::ValueTypeFieldEnum::Bool |
          Validator::ValueTypeFieldEnum::Null |
          Validator::ValueTypeFieldEnum::Double));
  option_opt->setValidator(std::make_shared<Validator::Option>(
      std::set<QString>{"One", "Two", "Three"}));
  existed_folder_opt->setValidator(std::make_shared<Validator::Path>(
      Validator::PathExistanceFieldEnum::Exist,
      Validator::PathTypeFieldEnum::Dir,
      Validator::PathTypeFieldEnum::Dir));
  nonexist_path_opt->setValidator(std::make_shared<Validator::Path>(
      Validator::PathExistanceFieldEnum::Nonexist));

  auto int_param = client->addParameter("int_param");
  auto plain_param = client->addParameter("plain_param");
  auto option_param = client->addParameter("one_or_two_or_three_param");
  auto existed_folder_param = client->addParameter("existed_folder_param");
  auto nonexist_path_param = client->addParameter("nonexist_path_param");

  QObject::connect(int_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   int_param.get(), &SideAssist::Qt::NamedValue::setValue);
  QObject::connect(plain_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   plain_param.get(), &SideAssist::Qt::NamedValue::setValue);
  QObject::connect(option_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   option_param.get(), &SideAssist::Qt::NamedValue::setValue);
  QObject::connect(
      existed_folder_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
      existed_folder_param.get(), &SideAssist::Qt::NamedValue::setValue);
  QObject::connect(
      nonexist_path_opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
      nonexist_path_param.get(), &SideAssist::Qt::NamedValue::setValue);

  return app.exec();
}
