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
        unsigned int url_length;
        unsigned int filename_length;
        unsigned int time_started_length;
        unsigned int time_ended_length;
    } Header;

    typedef struct  _ThreadInfo {
        qint64       thread_low_byte; // will be where last download stopped
        qint64       thread_high_byte; // byte destination in the range [ thread_low_byte - thread_high_byte ]
        unsigned int thread_number;
    } ThreadInfo;
    typedef QSharedPointer<ThreadInfo> SP_ThreadInfo;

    Header          info_header;
    QString         url;
    QString         filename;
    QString         time_started;
    QString         time_stopped;

    unsigned int    number_of_threads_used;
    SP_ThreadInfo   pthread_info;
    unsigned int    accept_ranges;

    qint64          size_of_file_in_bytes;
};

class DownloadInfo: public QObject
{
    Q_OBJECT

    QString filename_;
public:
    DownloadInfo( QString const & filename, QObject *parent = NULL );
    virtual ~DownloadInfo();
    static QMap<QString, QSharedPointer<Information> > const & DownloadInfoMap();
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
