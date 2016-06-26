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
    typedef struct  _ThreadInfo {
        qint64       thread_low_byte; // will be where last download stopped
        qint64       thread_high_byte; // byte destination in the range [ thread_low_byte - thread_high_byte ]
        qint64       bytes_written;
        unsigned int thread_number;
    } ThreadInfo;
    typedef QList<ThreadInfo> ThreadList;

    enum DownloadStatus {
        DownloadCompleted = 0,
        DownloadStopped,
        DownloadPaused,
        DownloadError,
        DownloadInProgress
    };
    static QString DownloadStatusToString( Information::DownloadStatus status );

    QString         original_url;
    QString         redirected_url;
    QString         filename;
    QString         time_started;
    QString         time_stopped;
    QString         speed;
    DownloadStatus  download_status;
    unsigned int    percentage;

    unsigned int    accept_ranges;
    ThreadList      threads;
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
