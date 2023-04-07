#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include "client.hpp"

namespace SideAssist::Qt {

static std::shared_ptr<QFile> log_file;
static void messageHandler(QtMsgType type,
                           const QMessageLogContext& context,
                           const QString& msg) {
  static QRegularExpression re(
      R"RAW(.+ ([A-Za-z_].+)\(.+?$)RAW");
  auto funcsig = QString(context.function);
  auto matches = re.match(funcsig);
  QString func_name = matches.captured(1).replace("SideAssist::Qt::", "");

  QString level_name;
  switch (type) {
    case QtDebugMsg:
      level_name = "Debg";
      break;
    case QtWarningMsg:
      level_name = "Warn";
      break;
    case QtCriticalMsg:
      level_name = "Erro";
      break;
    case QtFatalMsg:
      level_name = "Fata";
      break;
    case QtInfoMsg:
      level_name = "Info";
      break;
    default:
      level_name = "Unkn";
      break;
  }
  auto line = QString("%1 [%2] (%3) %4\n")
                  .arg(QDateTime::currentDateTime().toString())
                  .arg(level_name)
                  .arg(func_name)
                  .arg(msg);
  fprintf(stderr, "%s", qPrintable(line));
  log_file->write(line.toLocal8Bit());
  log_file->flush();
  if (type == QtFatalMsg) {
    abort();
  }
}

bool Client::installDefaultMessageHandler() {
  Q_ASSERT(log_file == nullptr);
  QString id = mqtt_client_->clientId();
  Q_ASSERT(!id.isEmpty());
  auto file = std::make_shared<QFile>(id + ".log");
  bool ret = file->open(QIODeviceBase::Append | QIODeviceBase::Text);
  if (!ret) {
    qCritical("Log file open failed: %s",
              qPrintable(QFileInfo(*file).canonicalFilePath()));
    return false;
  }
  log_file = file;
  qInstallMessageHandler(messageHandler);
  return true;
}

void Client::logConnected() {
  qInfo("Connected to %s", qPrintable(mqtt_client_->host().toString()));
}

void Client::logDisconnected() {
  qCritical("Disconnected from %s",
            qPrintable(mqtt_client_->host().toString()));
}

void Client::logPublished(const QMQTT::Message& message, quint16 id) {
  qInfo("Published message#%d on topic %s", id, qPrintable(message.topic()));
}

void Client::logSubscribed(const QString& topic, const quint8 qos) {
  qInfo("Subscribed with qos=%d to topic %s", qos, qPrintable(topic));
}

void Client::logError(const QMQTT::ClientError error) {
  qCritical("MQTT Client error: %d", error);
}

}  // namespace SideAssist::Qt
