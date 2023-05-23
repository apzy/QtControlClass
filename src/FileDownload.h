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
    /// ����Ŀ��Ŀ¼
    /// </summary>
    /// <param name="dstDir">Ŀ��Ŀ¼</param>
    void set_dst_dir(const QString& dstDir);

    /// <summary>
    /// ������ʱĿ¼����������ʱ��ֹ��;ʧ�ܵ��ļ����ڵ����
    /// </summary>
    /// <param name="dir">Ŀ��Ŀ¼</param>
    void set_temp_dir(const QString& dir);

    /// <summary>
    /// �����ļ�����ʱ�Ƿ񸲸�����
    /// </summary>
    /// <param name="flag">true ���� false ������</param>
    void set_cover_flag(const bool& flag);

    /// <summary>
    /// �������url
    /// </summary>
    /// <param name="url">��������</param>
    /// <param name="dstName">Ŀ���ļ�����Ϊ��ʱ�ļ���ȡurl������</param>
    void push_url(const QString& url, const QString& dstName = "");

    /// <summary>
    /// ��ʼ����
    /// </summary>
    void start();

    /// <summary>
    /// ����ͬʱ���صĸ���
    /// </summary>
    /// <param name="num">����</param>
    void set_download_num(const int& num);

private Q_SLOTS:

    void download_finished();

    void download_ready_read();

    void progress_timeout();

private:
    /// <summary>
    /// ���Ҫ���ص��ļ���С
    /// </summary>
    /// <param name="link">�ļ�����</param>
    /// <returns>�ļ���С</returns>
    int get_file_total_size(const QString& link);

    /// <summary>
    /// ��ʼһ���µ�����
    /// </summary>
    void add_new_downloader();

    /// <summary>
    /// ����Ŀ¼
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    QString create_dir(const QString path);

private:
    // ��ʱĿ¼
    QString m_tmpDir;

    // Ŀ��Ŀ¼
    QString m_dstDir;

    // ͬʱ���صĸ��� 
    int m_downloadNum;

    // ��Ƭ��С
    int m_minSize;

    // ���ض���
    QQueue<DownloadInfo> m_downloadQueue;

    // �����Ƭ��Ϣ
    QVector<SplitInfo> m_splitVector;

    // �Ƿ񸲸ǵı��
    bool m_coverFlag;

    // ��ǰ���ص��ļ�ָ��
    FILE* m_lastFile;

    // ��ǰʣ���Ƭ���ص��ļ���С
    int64_t m_remainingSize;

    // �Ѿ�������ɵĴ�С
    int64_t m_dlSize;

    // ���ļ���С
    int64_t m_fileTotalSize;

    // ��ǰ���ص��ļ���Ϣ
    DownloadInfo m_lastDlInfo;

    // ��ǰ�Ѿ���Ƭ�ĸ���
    int64_t m_lastSplitNum;

    QTimer m_progressTimer;

    unsigned long long m_startTime;

    QMutex m_mutex;

    QMutex m_queueMutex;

};

#endif // !__FILEDOWNLOAD_H__
