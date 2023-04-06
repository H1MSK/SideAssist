#include <QCoreApplication>
#include <memory>
#include "client.hpp"

std::shared_ptr<SideAssist::Qt::Client> client;

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  client = std::make_shared<SideAssist::Qt::Client>();

  client->setClientId("echo");
  client->installDefaultMessageHandler();

  client->setUsername("side_assist_client");
  client->setPassword("16509490");

  client->connectToHost();

  auto opt = client->addOption("opt");
  auto param = client->addParameter("param");
  QObject::connect(opt.get(), &SideAssist::Qt::NamedValue::valueChanged,
                   param.get(), &SideAssist::Qt::NamedValue::setValue);

  opt->setValue(QJsonValue(123));
  
  return app.exec();
}
