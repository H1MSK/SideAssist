#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QGuiApplication>
#include <QImage>
#include <QJsonArray>
#include <list>
#include <memory>
#include <mutex>
#include "client.hpp"
#include "value_validator.hpp"

std::mutex mutex;
std::list<QString> monitored_paths;
std::shared_ptr<QFileSystemWatcher> monitor;
std::shared_ptr<SideAssist::Qt::Client> client;

std::shared_ptr<SideAssist::Qt::NamedValue> monitored_path;
std::shared_ptr<SideAssist::Qt::NamedValue> filename;
std::shared_ptr<SideAssist::Qt::NamedValue> timestamp;

void updateMonitoredPath(const QJsonValue& val) {
  // TODO: use builtin validator
  if (!val.isArray())
    monitored_path->setValue(QJsonArray());
  monitor->removePaths(monitor->directories());
  qInfo("Resetting monitored paths...");
  for (auto itr : val.toArray()) {
    if (!itr.isString())
      continue;
    QString path = itr.toString();
    QFileInfo info(path);
    if (!info.isDir())
      continue;
    monitor->addPath(path);
    qInfo("Added monitor path %s", qUtf8Printable(path));
  }
}

void tryCopyImage(const QString& path) {
  qInfo("Detected directory %s changed.", qUtf8Printable(path));
  for (auto info : QDir(path).entryInfoList(QDir::Filter::Files, QDir::Time)) {
    auto time = info.lastModified().toMSecsSinceEpoch();
    if (time < timestamp->value().toInteger()) {
      qInfo("Nothing new in dir %s.", qUtf8Printable(path));
      break;
    }
    QImage img(info.canonicalFilePath());
    if (img.isNull())
      continue;
    qInfo("New image %s in dir %s.", qUtf8Printable(info.fileName()),
          qUtf8Printable(path));
    QGuiApplication::clipboard()->setImage(img);
    filename->setValue(info.canonicalFilePath());
    timestamp->setValue(time);
    break;
  }
}

int main(int argc, char* argv[]) {
  namespace Validator = SideAssist::Qt::ValueValidator;
  QGuiApplication app(argc, argv);
  client = std::make_shared<SideAssist::Qt::Client>();
  monitor = std::make_shared<QFileSystemWatcher>();

  client->setClientId("screenshot_copier");
  client->installDefaultMessageHandler();

  client->setUsername("side_assist_client");
  client->setPassword("16509490");

  monitored_path = client->addOption("monitored_path");
  filename = client->addParameter("filename");
  timestamp = client->addParameter("timestamp");

  client->connectToHost();

  filename->setValue("");
  timestamp->setValue(0);
  monitored_path->setValidator(
      std::make_shared<Validator::ListItem>(std::make_shared<Validator::Path>(
          Validator::PathExistanceFieldEnum::Exist,
          Validator::PathTypeFieldEnum::Dir,
          Validator::PathTypeFieldEnum::Dir)));

  QObject::connect(monitored_path.get(),
                   &SideAssist::Qt::NamedValue::valueChanged,
                   updateMonitoredPath);

  QObject::connect(monitor.get(), &QFileSystemWatcher::directoryChanged,
                   tryCopyImage);
  return app.exec();
}
