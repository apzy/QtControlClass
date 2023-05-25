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
    enum DownlodStatus
    {
        Failed,
        Success,
        Exist,
        ErrorUrl
    };

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
    /// ������ʱĿ¼����������ʱ��ֹ��;ʧ�ܵ��ļ����ڵ������������û��md5�����򿪳�������
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
    /// ����ͬʱ���ط�Ƭ�ĸ���
    /// </summary>
    /// <param name="num">����</param>
    void set_download_num(const int& num);

Q_SIGNALS: 
    void sig_download_progress(QString url, int progress, QString speed);

    void sig_download_status(QString url, DownlodStatus status);

    void sig_all_download_finished();

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
    int64_t get_file_total_size(const QString& link);

    /// <summary>
    /// ��ʼһ���µ�����
    /// </summary>
    void add_new_downloader();

private:
    // ��ʱĿ¼
    QString m_tmpDir;

    // Ŀ��Ŀ¼
    QString m_dstDir;

    // ͬʱ���صĸ��� 
    int m_downloadNum;

    // ��Ƭ��С
    int64_t m_minSize;

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

    // ���ڼ������
    QTimer m_progressTimer;

    // ��ʼ���ص�ʱ��
    unsigned long long m_startTime;

};

#endif // !__FILEDOWNLOAD_H__
