/*
 * DownloadManager.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: Joshua
 */

#include <src/DownloadManager.hpp>
#include <QNetworkRequest>
#include <QDir>
#include <QUuid>
#include <QFileInfo>
#include <src/DownloadInfo.hpp>

namespace CraneDM {

    QMap<QString, DownloadComponent*>   DownloadManager::active_download_list;
    QList<QString>  DownloadManager::inactive_downloads;
    unsigned int    DownloadManager::max_number_of_downloads = 2;
    unsigned int    DownloadManager::max_number_of_threads = 2;

    DownloadManager::DownloadManager( QObject *parent ): QObject( parent )
    {
    }

    DownloadManager::~DownloadManager()
    {
    }

    void DownloadManager::errorHandler( QString message, QString url )
    {

        emit error( message, url );
    }

    void DownloadManager::completionHandler( QString url )
    {
        DownloadManager::active_download_list.remove( url ); // belongs to a deleted thread

        if( DownloadManager::inactive_downloads.size() > 0 ){
            QString next_download = inactive_downloads.front();
            DownloadManager::inactive_downloads.pop_front();
            CraneDownloader::addNewUrl( next_download, DownloadManager::max_number_of_threads, DownloadManager::max_number_of_downloads );
        }
        emit completed( url );
    }

    void DownloadManager::downloadStartedHandler( QString url )
    {
        emit downloadStarted( url );
    }

    void DownloadManager::progressHandler( QString url )
    {
        emit progressed( url );
    }

    void DownloadManager::onDownloadStopped( QString url )
    {
        emit progressed( url );
    }
    QSharedPointer<DownloadManager> CraneDownloader::m_pDownloadManager ( new DownloadManager );

    CraneDownloader::CraneDownloader(): QObject( NULL )
    {
        //    QFile file( "data/download_info.json" );
        //    if( file.exists() ){
        //        file.remove();
        //    }
        ApplicationData::m_pDownloadInfo->readDownloadSettingsFile();

        QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( error( QString, QString ) ),
                this, SLOT( errorHandler( QString, QString ) ) );
        QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( downloadStarted(QString) ),
                this, SLOT( onDownloadStarted( QString ) ) );
        QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( completed( QString ) ),
                this, SLOT( onDownloadCompleted( QString ) ) );
        QObject::connect( CraneDownloader::m_pDownloadManager.data(), SIGNAL( progressed( QString ) ),
                this, SLOT( progressHandler( QString ) ) );
    }

    CraneDownloader::~CraneDownloader(){ }
    void CraneDownloader::addNewUrlWithManager( QString const & address, QString location, DownloadManager *dm )
    {
        QString new_uri_address =  QUrl::fromUserInput( address ).toString();

        QThread *new_download_thread = new QThread();
        DownloadComponent *new_download = new DownloadComponent( new_uri_address, location,
                DownloadManager::max_number_of_threads, new_download_thread );

        DownloadManager::active_download_list.insert( new_uri_address, new_download );

        QObject::connect( new_download_thread, SIGNAL( started() ), new_download, SLOT( startDownload() ) );
        QObject::connect( new_download, SIGNAL( progressChanged( QString ) ), CraneDownloader::m_pDownloadManager.data(),
                SLOT( progressHandler( QString ) ) );
        QObject::connect( new_download, SIGNAL( error( QString, QString )), dm, SLOT( errorHandler( QString, QString )) );
        QObject::connect( new_download, SIGNAL( downloadStarted( QString )), dm, SLOT( downloadStartedHandler( QString )) );
        QObject::connect( new_download, SIGNAL( completed( QString ) ), dm, SLOT( completionHandler( QString ) ) );
        QObject::connect( new_download, SIGNAL( stopped( QString ) ), dm, SLOT( onDownloadStopped( QString ) ) );
        QObject::connect( new_download, SIGNAL( stopped( QString ) ), new_download_thread, SLOT( quit() ) );
        QObject::connect( new_download, SIGNAL( completed( QString ) ), new_download_thread, SLOT( quit() ) );
        QObject::connect( new_download_thread, SIGNAL( finished() ), new_download_thread, SLOT( deleteLater() ) );

        new_download_thread->start();
    }

    void CraneDownloader::addNewUrl( QString const & address, unsigned int threads_to_use,
            unsigned int download_limit, QString location )
    {
        DownloadManager::max_number_of_downloads = download_limit;
        DownloadManager::max_number_of_threads = threads_to_use;

        if( DownloadManager::max_number_of_downloads != 0 ){
            if( DownloadManager::active_download_list.size() >= DownloadManager::max_number_of_downloads ){
                DownloadManager::inactive_downloads.push_back( address );
                qDebug() << "New download queued up";
                return;
            }
            if( DownloadManager::inactive_downloads.contains( address ) ){
                qDebug() << "Don't worry, we've queued the download for you already.";
                return;
            }
        }

        if( DownloadManager::active_download_list.contains( address ) ){
            qDebug() << "Download in progress";
            return;
        }

        addNewUrlWithManager( address, location, CraneDownloader::m_pDownloadManager.data() );
    }

    void CraneDownloader::stopDownload( QString const & url, bool toPause )
    {
        if( DownloadManager::active_download_list.contains( url ) ){
            DownloadComponent *download = DownloadManager::active_download_list.value( url );
            if( !download ) return;

            download->stopDownload();

            if( !toPause ) return;
            QSharedPointer<Information> p_info = DownloadInfo::UrlSearch( url );
            if( p_info ){
                p_info->download_status = Information::DownloadPaused;
            }
        }
    }

    void CraneDownloader::removeItem( QString const & url, bool deleteFile )
    {
        if( url.isEmpty() ) return;
        if( DownloadManager::inactive_downloads.contains( url ) ){
            DownloadManager::inactive_downloads.removeAll( url );
        }
        CraneDownloader::stopDownload( url, false );

        QSharedPointer<Information> p_info = DownloadInfo::UrlSearch( url );
        if( p_info ){
            if( !deleteFile ) return;

            QFile file( p_info->path_to_file );
            if( file.exists() ){
                file.remove();
            }
        }
    }

    void CraneDownloader::errorHandler( QString what, QString url )
    {
        DownloadManager::active_download_list.remove( url );
        emit progressed( url );
        emit error( what );
    }

    void CraneDownloader::onDownloadCompleted( QString url )
    {
        emit progressed( url );
        emit completed( url );
    }

    void CraneDownloader::onDownloadStarted( QString url )
    {
        emit downloadStarted( url );
    }

    void CraneDownloader::progressHandler( QString url )
    {
        emit progressed( url );
    }

    void CraneDownloader::aboutToExit()
    {
        QList<QString> keys = DownloadManager::active_download_list.keys();
        for( int i = 0; i != keys.size(); ++i ){
            DownloadComponent *component = DownloadManager::active_download_list.value( keys[i] );
            component->stopDownload();
        }
        ApplicationData::m_pDownloadInfo->writeDownloadSettingsFile();
    }
}
