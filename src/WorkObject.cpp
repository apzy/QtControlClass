#include "WorkObject.h"
#include <QProcess>

#include <Windows.h>
#include <tlhelp32.h>
#include <qjsondocument.h>
#include <qnetworkrequest.h>
#include <qnetworkaccessmanager.h>
#include <qnetwork.h>
#include <qeventloop.h>
#include <QtNetwork>

WorkObject::WorkObject()
{
    m_manager = new QNetworkAccessManager();
}

WorkObject::~WorkObject()
{}

bool WorkObject::remove_folder(const QString& folderDir)
{
    if (folderDir.isEmpty())
        return false;
    qDebug("remove folder %s", folderDir);
    QDir dir(folderDir);
    QFileInfoList fileList;
    QFileInfo curFile;
    bool ret;
    if (!dir.exists())
    {
        ret = false;
    }
    else
    {
        //文件不存，则返回false
        fileList = dir.entryInfoList(QDir::Dirs | QDir::Files
            | QDir::Readable | QDir::Writable
            | QDir::Hidden | QDir::NoDotAndDotDot
            , QDir::Name);
        while (fileList.size() > 0)
        {
            int infoNum = fileList.size();
            for (int i = infoNum - 1; i >= 0; i--)
            {
                curFile = fileList[i];
                if (curFile.isFile())//如果是文件，删除文件
                {
                    QFile fileTemp(curFile.filePath());
                    fileTemp.remove();
                    fileList.removeAt(i);
                }
                if (curFile.isDir())//如果是文件夹
                {
                    QDir dirTemp(curFile.filePath());
                    QFileInfoList fileList1 = dirTemp.entryInfoList(QDir::Dirs | QDir::Files
                        | QDir::Readable | QDir::Writable
                        | QDir::Hidden | QDir::NoDotAndDotDot
                        , QDir::Name);
                    if (fileList1.size() == 0)//下层没有文件或文件夹
                    {
                        dirTemp.rmdir(".");
                        fileList.removeAt(i);
                    }
                    else//下层有文件夹或文件
                    {
                        for (int j = 0; j < fileList1.size(); j++)
                        {
                            if (!(fileList.contains(fileList1[j])))
                                fileList.append(fileList1[j]);
                        }
                    }
                }
            }
        }
        ret = dir.removeRecursively();
    }

    return ret;
}

bool WorkObject::copy_dir(const QDir& fromDir, const QDir& toDir, bool bCoverIfFileExists)
{
    QDir formDir_ = fromDir;
    QDir toDir_ = toDir;
    bool ret = true;

    if (!toDir_.exists())
    {
        if (!toDir_.mkdir(toDir.absolutePath()))
            ret = false;
    }
    if (ret)
    {
        QFileInfoList fileInfoList = formDir_.entryInfoList();
        foreach(QFileInfo fileInfo, fileInfoList)
        {
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;

            //拷贝子目录
            if (fileInfo.isDir())
            {
                //递归调用拷贝
                if (!copy_dir(fileInfo.filePath(), toDir_.filePath(fileInfo.fileName()), true))
                {
                    ret = false;
                    break;
                }
            }
            //拷贝子文件
            else
            {
                if (bCoverIfFileExists && toDir_.exists(fileInfo.fileName()))
                {
                    toDir_.remove(fileInfo.fileName());
                }
                if (!QFile::copy(fileInfo.filePath(), toDir_.filePath(fileInfo.fileName())))
                {
                    ret = false;
                    break;
                }
            }
        }
    }
    return ret;
}

QString WorkObject::create_dir(const QString path)
{
    QDir dir(path);
    if (dir.exists(path) || !path.contains("/"))
    {
        return path;
    }
    QString parentDir = create_dir(path.mid(0, path.lastIndexOf('/')));
    QString dirname = path.mid(path.lastIndexOf('/') + 1);
    QDir parentPath(parentDir);
    if (!dirname.isEmpty())
        parentPath.mkpath(dirname);
    return parentDir + "/" + dirname;
}

void WorkObject::set_path_to_safe(QString& path)
{
    path.replace("\\", "/");
    if (!path.contains(":"))
    {
        if (!(path.startsWith("./") || path.startsWith("../")))
        {
            path = "./" + path;
        }
    }
    if (!path.endsWith("/"))
    {
        path += "/";
    }
}

QString WorkObject::get_mac_address()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
    int nCnt = nets.count();
    QString strMacAddr = "";
    for (int i = 0; i < nCnt; i++)
    {
        if (nets[i].flags().testFlag(QNetworkInterface::IsUp) &&
            nets[i].flags().testFlag(QNetworkInterface::IsRunning)
            && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            for (int j = 0; j < nets[i].addressEntries().size(); j++)
            {
                if (nets[i].addressEntries().at(j).ip() != QHostAddress::LocalHost &&
                    nets[i].addressEntries().at(j).ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    strMacAddr = nets[i].hardwareAddress();
                }
            }
        }
    }
    return strMacAddr;
}


bool WorkObject::find_process(const QString process)
{
    bool ret = false;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32);
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    BOOL bMore = Process32First(hProcessSnap, &pe32);
    while (bMore)
    {
#ifdef UNICODE
        QString exeName = QString::fromWCharArray(pe32.szExeFile);
#else
        QString exeName = pe32.szExeFile;
#endif
        if (exeName.contains(process))
        {
            ret = true;
            break;
        }
        bMore = Process32Next(hProcessSnap, &pe32);
    }
    CloseHandle(hProcessSnap);
    return ret;
}

void WorkObject::run_cmd_show(const QStringList& args)
{
    QProcess p;
    QString program = "C:/Windows/System32/cmd.exe";
    p.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args)
        {
            args->flags |= CREATE_NEW_CONSOLE;
            args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
            args->startupInfo->dwFlags |= STARTF_USEFILLATTRIBUTE;
            args->startupInfo->dwFillAttribute = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    );
    p.start(program, args);

    p.waitForStarted();
    p.waitForFinished(-1);
    p.close();
}

QProcess* WorkObject::launch_bat(const QString& filePath,const QString& runDir)
{
    SetCurrentDirectoryW(runDir.toStdWString().c_str());
    QStringList arguments;
    arguments << "/K";
    arguments << filePath;

    AllocConsole();
    QProcess *p = new QProcess();
    QString program = "C:/Windows/System32/cmd.exe";
    p->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args)
        {
            args->flags |= CREATE_NEW_CONSOLE;
            args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
            args->startupInfo->dwFlags |= STARTF_USEFILLATTRIBUTE;
            args->startupInfo->dwFillAttribute = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    );
    p->start(program, arguments);
    p->waitForStarted(-1);
    return p;
}

QString WorkObject::post(const QString& url, const QJsonObject& json)
{
    QJsonDocument document;
    document.setObject(json);
    QByteArray dataArray = document.toJson(QJsonDocument::Compact);
    qDebug("post data \n>%s\n", qPrintable(dataArray));

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Content-Type", "application/json;charset=UTF-8");

    QNetworkReply* reply = m_manager->post(request, dataArray);
    QEventLoop eventLoop;
    connect(m_manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    QByteArray data = reply->readAll();
    return data;
}

QString WorkObject::get(const QString& url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Content-Type", "application/json;charset=UTF-8");

    QNetworkReply* reply = m_manager->get(request);
    QEventLoop eventLoop;
    connect(m_manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    QByteArray data = reply->readAll();
    return data;
}
