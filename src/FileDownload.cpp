#include "FileDownload.h"
#include "WorkObject.h"

FileDownload::FileDownload(QObject* parent)
    :QObject(parent)
    , m_tmpDir("")
    , m_downloadNum(5)
    , m_minSize(1024 * 1024 * 10)
    , m_coverFlag(false)
    , m_remainingSize(0)
    , m_lastFile(nullptr)
    , m_lastSplitNum(0)
    , m_dstDir("./")
    , m_fileTotalSize(0)
    , m_dlSize(0)
{
    m_progressTimer.setInterval(1000);
    connect(&m_progressTimer, &QTimer::timeout, this, &FileDownload::progress_timeout);
}

FileDownload::~FileDownload()
{}

void FileDownload::set_dst_dir(const QString& dstDir)
{
    m_dstDir = dstDir;
    WorkObject::set_path_to_safe(m_dstDir);
}

void FileDownload::set_temp_dir(const QString& dir)
{
    m_tmpDir = dir;
    WorkObject::set_path_to_safe(m_tmpDir);
}

void FileDownload::set_cover_flag(const bool& flag)
{
    m_coverFlag = flag;
}

void FileDownload::push_url(const QString& url, const QString& dstName)
{
    DownloadInfo info;
    info.url = url.trimmed();
    if (dstName.isEmpty())
    {
        info.dstName = info.url.mid(info.url.lastIndexOf("/") + 1);
    }
    else
    {
        info.dstName = dstName;
    }
    m_downloadQueue.push_back(info);
}

void FileDownload::start()
{
    m_lastSplitNum = 0;
    m_dlSize = 0;

    if (m_downloadQueue.isEmpty())
    {
        Q_EMIT sig_all_download_finished();
        return;
    }

    if (m_splitVector.size() > 0)
    {
        return;
    }

    m_lastDlInfo = m_downloadQueue.dequeue();

    if (QFile::exists(m_dstDir + m_lastDlInfo.dstName) && !m_coverFlag)
    {
        Q_EMIT sig_download_status(m_lastDlInfo.url, FileDownload::Exist);
        start();
        return;
    }
    QFile::remove(m_dstDir + m_lastDlInfo.dstName);

    m_remainingSize = get_file_total_size(m_lastDlInfo.url);
    if (m_remainingSize <= 0)
    {
        Q_EMIT sig_download_status(m_lastDlInfo.url, FileDownload::ErrorUrl);
        start();
        return;
    }

    m_fileTotalSize = m_remainingSize;

    WorkObject::create_dir(m_dstDir);
    if (m_tmpDir.isEmpty())
    {
        fopen_s(&m_lastFile, (m_dstDir + m_lastDlInfo.dstName).toStdString().c_str(), "wb");
    }
    else
    {
        WorkObject::create_dir(m_tmpDir);
        fopen_s(&m_lastFile, (m_tmpDir + m_lastDlInfo.dstName).toStdString().c_str(), "wb");
    }

    m_startTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    m_progressTimer.start();

    while (m_remainingSize > 0 && m_splitVector.size() < m_downloadNum)
    {
        add_new_downloader();
    }
}

void FileDownload::set_download_num(const int& num)
{
    m_downloadNum = num;
}

void FileDownload::download_finished()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    int i = 0;
    for (SplitInfo info : m_splitVector)
    {
        if (info.reply == reply)
        {
            info.reply->close();
            info.manager->destroyed();
            m_splitVector.remove(i);
            break;
        }
        ++i;
    }

    if (m_remainingSize > 0 && m_splitVector.size() < m_downloadNum)
    {
        add_new_downloader();
    }
    if (m_splitVector.size() == 0)
    {
        m_progressTimer.stop();
        qDebug("download %s finished", qPrintable(m_lastDlInfo.url));
        fclose(m_lastFile);
        if (!m_tmpDir.isEmpty())
        {
            QFile::rename(m_tmpDir + m_lastDlInfo.dstName, m_dstDir + m_lastDlInfo.dstName);
        }
        Q_EMIT sig_download_status(m_lastDlInfo.url, FileDownload::Success);
        start();
    }
}

void FileDownload::download_ready_read()
{
    QNetworkReply* reply = (QNetworkReply*)sender();
    QByteArray data = reply->readAll();
    for (SplitInfo& info : m_splitVector)
    {
        if (info.reply == reply)
        {
            m_dlSize += data.size();
            _fseeki64(m_lastFile, info.start + info.writePos, SEEK_SET);
            fwrite(data.data(), sizeof(char), data.size(), m_lastFile);
            info.writePos += data.size();
            if (info.writePos > info.size)
            {
                qDebug("write error");
            }
            break;
        }
    }
}

void FileDownload::progress_timeout()
{
    unsigned long long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t speed = 0;
    if (m_dlSize != 0)
    {
        speed = m_dlSize * 1000 / (now - m_startTime);
    }
    unsigned long long elapsed = (now - m_startTime);

    QString unit;
    if (speed < 1024)
    {
        unit = "bytes/sec";
    }
    else if (speed < 1024 * 1024)
    {
        speed /= 1024;
        unit = "kB/s";
    }
    else
    {
        speed /= 1024 * 1024;
        unit = "MB/s";
    }

    int progress = 1.0 * m_dlSize / m_fileTotalSize * 100.0;
    qDebug("progress = %d speed = %d %s", progress, speed, qPrintable(unit));
    Q_EMIT sig_download_progress(m_lastDlInfo.url, progress, QString::number(speed) + " " + unit);
}

int64_t FileDownload::get_file_total_size(const QString& link)
{
    QUrl url(link);
    QNetworkAccessManager manager;
    QEventLoop loop;

    QNetworkReply* reply = manager.head(QNetworkRequest(link));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
    __int64 size = var.toLongLong();
    reply->deleteLater();
    manager.destroyed();
    return size;
}

void FileDownload::add_new_downloader()
{
    if (m_remainingSize <= 0)
        return;

    SplitInfo info;
    info.writePos = 0;
    info.start = m_lastSplitNum * m_minSize;
    ++m_lastSplitNum;
    info.size = m_remainingSize > m_minSize ? m_minSize : m_remainingSize;
    m_remainingSize -= m_minSize;

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = nullptr;
    QNetworkRequest request;
    QString range = QString("bytes=%1-%2").arg(info.start).arg(info.start + info.size - 1);
    request.setRawHeader("Range", range.toUtf8());
    request.setUrl(m_lastDlInfo.url);
    reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, &FileDownload::download_finished);
    connect(reply, &QNetworkReply::readyRead, this, &FileDownload::download_ready_read);

    info.reply = reply;
    info.manager = manager;

    m_splitVector.push_back(info);
}

