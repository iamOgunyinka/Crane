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
#include <QDate>
#include <QSharedPointer>

namespace CraneDM {

    struct Information
    {
        struct ThreadInfo {
            qint64       thread_low_byte; // will be where last download stopped
            qint64       thread_high_byte; // byte destination in the range [ thread_low_byte - thread_high_byte ]
            qint64       bytes_written;
            unsigned int percentage;
            unsigned int thread_number;
        };
        typedef QList<ThreadInfo> ThreadInfoList;

        enum DownloadStatus {
            DownloadInProgress = 0,
            DownloadCompleted,
            DownloadStopped,
            DownloadPaused,
            DownloadError,
        };
        static QString DownloadStatusToString( Information::DownloadStatus status );
        static DownloadStatus IntToStatus( int status );
        QString         original_url;
        QString         redirected_url;
        QString         filename;
        QString         path_to_file;
        QDateTime       time_started;
        QDateTime       time_stopped;
        QString         speed;
        DownloadStatus  download_status;
        unsigned int    percentage;

        unsigned int    accept_ranges;
        ThreadInfoList  thread_information_list;
        quint64         size_of_file_in_bytes;
    };

    class DownloadInfo: public QObject
    {
        Q_OBJECT

        QString filename_;
    public:
        DownloadInfo( QString const & filename, QObject *parent = NULL );
        virtual ~DownloadInfo();
        static QList<QSharedPointer<Information> > & DownloadInfoList();
        static QSharedPointer<Information> UrlSearch( QString const & url );
    private:
        static QList<QSharedPointer<Information> > download_info_list;
        public slots:
        void readDownloadSettingsFile();
        void writeDownloadSettingsFile();
    };
}
#endif /* DOWNLOADINFO_H_ */
