#ifndef __FILEDOWNLOAD_H__
#define __FILEDOWNLOAD_H__

#include <QtNetwork>
#include <QtCore>
#include <QMutex>
#include <QTimer>
#include <chrono>

class FileDownload : public QObject
{
    Q_OBJECT
public:
    struct DownloadInfo 
    {
        QString url;
        QString dstName;

        DownloadInfo(QString url,QString dstName)
        {
            this->url = url;
            this->dstName = dstName;
        }

        DownloadInfo()
        {
            this->url = "";
            this->dstName = "";
        }
    };

    struct SplitInfo
    {
        int64_t start;
        int64_t size;
        int64_t writePos;
        QNetworkReply* reply;
        QNetworkAccessManager* manager;
    };

public:
    FileDownload(QObject* parent = nullptr);
    ~FileDownload();

    /// <summary>
    /// 设置目标目录
    /// </summary>
    /// <param name="dstDir">目标目录</param>
    void set_dst_dir(const QString& dstDir);

    /// <summary>
    /// 设置临时目录，用于下载时防止中途失败但文件存在的情况
    /// </summary>
    /// <param name="dir">目标目录</param>
    void set_temp_dir(const QString& dir);

    /// <summary>
    /// 设置文件存在时是否覆盖下载
    /// </summary>
    /// <param name="flag">true 覆盖 false 不覆盖</param>
    void set_cover_flag(const bool& flag);

    /// <summary>
    /// 添加下载url
    /// </summary>
    /// <param name="url">下载链接</param>
    /// <param name="dstName">目标文件名，为空时文件名取url中名称</param>
    void push_url(const QString& url, const QString& dstName = "");

    /// <summary>
    /// 开始下载
    /// </summary>
    void start();

    /// <summary>
    /// 设置同时下载的个数
    /// </summary>
    /// <param name="num">个数</param>
    void set_download_num(const int& num);

private Q_SLOTS:

    void download_finished();

    void download_ready_read();

    void progress_timeout();

private:
    /// <summary>
    /// 获得要下载的文件大小
    /// </summary>
    /// <param name="link">文件链接</param>
    /// <returns>文件大小</returns>
    int get_file_total_size(const QString& link);

    /// <summary>
    /// 开始一个新的下载
    /// </summary>
    void add_new_downloader();

    /// <summary>
    /// 创建目录
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    QString create_dir(const QString path);

private:
    // 临时目录
    QString m_tmpDir;

    // 目标目录
    QString m_dstDir;

    // 同时下载的个数 
    int m_downloadNum;

    // 分片大小
    int m_minSize;

    // 下载队列
    QQueue<DownloadInfo> m_downloadQueue;

    // 保存分片信息
    QVector<SplitInfo> m_splitVector;

    // 是否覆盖的标记
    bool m_coverFlag;

    // 当前下载的文件指针
    FILE* m_lastFile;

    // 当前剩余分片下载的文件大小
    int64_t m_remainingSize;

    // 已经下载完成的大小
    int64_t m_dlSize;

    // 总文件大小
    int64_t m_fileTotalSize;

    // 当前下载的文件信息
    DownloadInfo m_lastDlInfo;

    // 当前已经分片的个数
    int64_t m_lastSplitNum;

    QTimer m_progressTimer;

    unsigned long long m_startTime;

    QMutex m_mutex;

    QMutex m_queueMutex;

};

#endif // !__FILEDOWNLOAD_H__
