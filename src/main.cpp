#include <QCoreApplication>
#include "FileDownload.h"

int main(int argc, char* argv[])
{
    qputenv("QT_LOGGING_RULES", "qt.network.monitor=false");

    QCoreApplication a(argc, argv);

    FileDownload downloader;

    QString link = "https://mrstage-oss.oss-cn-shanghai.aliyuncs.com/nocode/20230522/tt_qzQCUwvxtd.zip";
    downloader.push_url(link);
    //downloader.set_cover_flag(true);
    //downloader.set_dst_dir("hello");
    //downloader.set_download_num(10);
    downloader.set_temp_dir("./Temp");
    downloader.start();

    return a.exec();
}
