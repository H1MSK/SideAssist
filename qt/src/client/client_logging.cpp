#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QMutex>
#include "client.hpp"

namespace SideAssist::Qt {

static std::shared_ptr<QFile> log_file;
static void messageHandler(QtMsgType type,
                           const QMessageLogContext& context,
                           const QString& msg) {
  static QMutex log_mutex;
  static QRegularExpression re(R"RAW(.+ ([A-Za-z_].+)\(.+?$)RAW");
  auto funcsig = QString(context.function);
  auto matches = re.match(funcsig);
  QString func_name = matches.captured(1).replace("SideAssist::Qt::", "");
  if (func_name.isEmpty())
    func_name = funcsig;

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
  
  {
    QMutexLocker locker(&log_mutex);
    fprintf(stderr, "%s", qPrintable(line));
  }
  log_file->write(line.toLocal8Bit());
  log_file->flush();
}

bool Client::installDefaultMessageHandler() {
  // Assure the handler is not set yet
  Q_ASSERT(log_file == nullptr);

  // Generate directory string & QFileInfo
  QString id = mqtt_client_->clientId();
  Q_ASSERT(!id.isEmpty());
  auto log_dir_string = QDir::current().canonicalPath() + "/log";
  auto log_dir_info = QFileInfo(log_dir_string);

  // Create the directory if not found
  if (auto info = QFileInfo(log_dir_string); !info.isDir()) {
    bool succeed = info.dir().mkdir("log");
    if (!succeed) {
      qCritical("Cannot create log directory on %s",
                qUtf8Printable(log_dir_string));
      return false;
    }
  }

  // Rename the previous file to {dir}/{id}.{num}.log
  QString log_file_name = log_dir_string + '/' + id + ".log";
  if (QFileInfo::exists(log_file_name)) {
    int count = 1;
    QString first_not_occupied;
    do {
      first_not_occupied =
          log_dir_string + '/' + id + '.' + QString::number(count) + ".log";
      ++count;
    } while (QFileInfo::exists(first_not_occupied));
    bool succeed = QFile::rename(log_file_name, first_not_occupied);
    if (!succeed) {
      qCritical("Cannot rename old log file %s", qUtf8Printable(log_file_name));
      return false;
    }
  }

  // Open file
  auto file = std::make_shared<QFile>(log_dir_string + '/' + id + ".log");
  bool ret = file->open(QIODeviceBase::Append | QIODeviceBase::Text);
  if (!ret) {
    qCritical("Log file open failed: %s",
              qUtf8Printable(QFileInfo(*file).canonicalFilePath()));
    return false;
  }
  log_file = std::move(file);

  // Set handler
  qInstallMessageHandler(messageHandler);
  qInfo("Default message handler installed.");
  return true;
}

void Client::logConnected() {
  qInfo("Connected to %s", qUtf8Printable(mqtt_client_->host().toString()));
}

void Client::logDisconnected() {
  qCritical("Disconnected from %s",
            qUtf8Printable(mqtt_client_->host().toString()));
}

void Client::logPublished(const QMQTT::Message& message, quint16 id) {
  qInfo("Published message#%d on topic %s", id,
        qUtf8Printable(message.topic()));
}

void Client::logSubscribed(const QString& topic, const quint8 qos) {
  qInfo("Subscribed with qos=%d to topic %s", qos, qUtf8Printable(topic));
}

void Client::logUnsubscribed(const QString& topic) {
  qInfo("Unsubscribed from topic %s", qUtf8Printable(topic));
}

void Client::handleMqttError(const QMQTT::ClientError error) {
  if (error == QMQTT::ClientError::SocketConnectionRefusedError) {
    qCritical("Could not connect to server!");
    qInfo("Exiting...");
    exit(1);
  }
  qCritical("MQTT Client error: %d", error);
}

}  // namespace SideAssist::Qt
