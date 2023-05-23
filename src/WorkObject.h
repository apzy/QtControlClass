#ifndef __WORKOBJECT_H__
#define __WORKOBJECT_H__

#include <QThread>
#include <QDir>
#include <QJsonObject>
#include <qnetworkreply.h>
#include <qprocess.h>

class WorkObject : public QObject
{
    Q_OBJECT

public:
    enum WorkType
    {
        REMOVE_FOLDER,
        COPY_DIR
    };

public:
    WorkObject();
    ~WorkObject();

    static bool remove_folder(const QString& folderDir);

    static bool copy_dir(const QDir& fromDir, const QDir& toDir, bool bCoverIfFileExists = true);

    /// <summary>
    /// 创建目录
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    static QString create_dir(const QString path);

    /// <summary>
    /// 修改路径为可以使用的路径
    /// </summary>
    /// <param name="path"></param>
    static void set_path_to_safe(QString& path);

    static bool find_process(const QString process);

    static void run_cmd_show(const QStringList& args);

    static QProcess* launch_bat(const QString& filePath, const QString& runDir = "./");

    QString post(const QString& url, const QJsonObject& json);

    QString get(const QString& url);

private Q_SLOTS:

Q_SIGNALS:
    void sig_work_finished(WorkType, bool);

private:
    QNetworkAccessManager* m_manager;

};

#endif // !__UPDATETHREAD_H__
