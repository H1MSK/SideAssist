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
      R"RAW((([^ \n]+ )*)(([^:\(\n ]+::)*?)(([^:\(\n ]+)::)?([^:\(\n ]+)\([^\n]+\))RAW");
  auto funcsig = QString(context.function);
  auto matches = re.match(funcsig);
  QString funcname = matches.captured(6);
  if (funcname.isEmpty())
    funcname = matches.captured(7);
  if (funcname.isEmpty())
    funcname = funcsig;
  static const QString levelNames[] = {"Debug", "Info", "Warn", "Error",
                                       "Fatal"};
  auto line = QString("%1 (%2)[%3] %4\n")
                  .arg(QDateTime::currentDateTime().toString())
                  .arg(funcname)
                  .arg(levelNames[type])
                  .arg(msg);
  fprintf(stderr, "%s", qPrintable(line));
  log_file->write(line.toLocal8Bit());
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
}  // namespace SideAssist::Qt
