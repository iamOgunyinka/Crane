/*
 * DownloadInfo.hpp
 *
 *  Created on: Jun 22, 2016
 *      Author: Joshua
 */

#ifndef DOWNLOADINFO_H_
#define DOWNLOADINFO_H_

#include <QObject>
#include <QMap>
#include <QSharedPointer>

struct Information
{
    typedef struct _Header {
        unsigned int old_url_length;
        unsigned int new_url_length;
        unsigned int filename_length;
        unsigned int time_started_length;
        unsigned int time_ended_length;
    } Header;

    typedef struct  _ThreadInfo {
        qint64       thread_low_byte; // will be where last download stopped
        qint64       thread_high_byte; // byte destination in the range [ thread_low_byte - thread_high_byte ]
        qint64       bytes_written;
        unsigned int thread_number;
    } ThreadInfo;

    struct CustomDeleter {
        void operator()( Information::ThreadInfo *thread_info_pointer ){
            delete []thread_info_pointer;
        }
    };

    typedef QSharedPointer<ThreadInfo> SP_ThreadInfo;

    Header          info_header;
    QString         original_url;
    QString         redirected_url;
    QString         filename;
    QString         time_started;
    QString         time_stopped;

    unsigned int    download_completed;
    unsigned int    number_of_threads_used;
    unsigned int    accept_ranges;

    SP_ThreadInfo   pthread_info;
    qint64          size_of_file_in_bytes;
};

class DownloadInfo: public QObject
{
    Q_OBJECT

    QString filename_;
public:
    DownloadInfo( QString const & filename, QObject *parent = NULL );
    virtual ~DownloadInfo();
    static QMap<QString, QSharedPointer<Information> > & DownloadInfoMap();
private:
    static QMap<QString, QSharedPointer<Information> > download_info_map;
public slots:
    void readDownloadSettingsFile();
    void writeDownloadSettingsFile();

signals:
    void finished();
    void error( QString );
    void informationUpdated();
};

#endif /* DOWNLOADINFO_H_ */
