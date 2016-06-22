/*
 * NetworkManager.cpp
 *
 *  Created on: Jun 17, 2016
 *      Author: adonai
 */

#include <src/NetworkManager.hpp>
#include <QNetworkRequest>
#include <QDir>
#include <QUuid>
#include <QFileInfo>

NetworkManager::NetworkManager( QObject *parent ): QObject( parent ),
    number_of_threads( 1 )
{

}

NetworkManager::~NetworkManager()
{

}

QUrl NetworkManager::redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl ) const
{
    QUrl redirectUrl;
    if( !possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl ) {
        redirectUrl = possibleRedirectUrl;
    }
    return redirectUrl;
}

void NetworkManager::addNewUrl( QString const & address, int threads_to_use, QString location )
{
    QNetworkRequest request;
    request.setUrl( QUrl( address ) );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    QNetworkReply *reply = DownloadItem::GetNetworkManager()->head( request );

    number_of_threads = threads_to_use;
    if( !location.isEmpty() ){
        download_directory = location;
    }

    QObject::connect( reply, SIGNAL(finished()), this, SLOT( headFinishedHandler() ) );
    QObject::connect( reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
}

void NetworkManager::headFinishedHandler()
{
    QNetworkReply *response = qobject_cast<QNetworkReply*>( sender() );
    if( response->error() == QNetworkReply::NoError )
    {
        // check for redirection
        QVariant redirection = response->attribute( QNetworkRequest::RedirectionTargetAttribute );
        QUrl redirected_to;

        redirected_to = redirectUrl( redirection.toUrl(), redirected_to );

        // we have a redirection, no?
        if( redirected_to.isEmpty() )
        {
            DownloadComponent *new_download = new DownloadComponent();
            new_download->url = response->url();
            QString filename = NetworkManager::GetFilename( new_download->url, download_directory );

            new_download->file = new QFile( filename );
            if( !( new_download->file->open( QIODevice::ReadWrite ) ) ){
                emit error( new_download->file->errorString() );
                delete new_download->file;
                new_download->deleteLater();
                return;
            }

            // accepts pause/resume and slicing of files for different threads to process
            if( response->hasRawHeader( QByteArray( "Accept-Ranges" ) ) ){
                new_download->accept_ranges = 1;
            } else {
                new_download->accept_ranges = 0;
            }
            //size of the file to download
            if( response->hasRawHeader( QByteArray( "Content-Length" ) ) ){
                new_download->size_in_bytes = response->rawHeader( "Content-Length" ).toInt();
            } else {
                new_download->size_in_bytes = 0;
            }

            if( !new_download->file->resize( new_download->size_in_bytes ) ){
                new_download->file->close();
                delete new_download->file;
                new_download->deleteLater();

                emit error( new_download->file->errorString() );
                return;
            }
            new_download->time_started = QDateTime::currentDateTime();

            int increment = new_download->size_in_bytes == 0 ? 0
                    : ( new_download->size_in_bytes / number_of_threads );

            // if the size of file is unknown, let's use a single thread
            unsigned int threads_to_use = new_download->size_in_bytes == 0 ? 1 : number_of_threads;

            unsigned int byte_begin = 0, byte_end = 0;

            QNetworkRequest request( new_download->url );
            request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

            for( unsigned int i = 1; i <= threads_to_use; ++i )
            {
                QThread *new_thread = new QThread( this );
                DownloadItem *download = NULL;
                // mark all the threads as "not finished"
                new_download->threads.append( 0 );

                if( i == threads_to_use ){
                    download = new DownloadItem( new_download->url, byte_begin, new_download->size_in_bytes, new_thread );
                } else {
                    byte_end += increment;
                    download = new DownloadItem( new_download->url, byte_begin, byte_end, new_thread );
                }
                byte_begin = byte_end + 1;

                download->setThreadNumber( i );
                download->setFile( new_download->file );

                QObject::connect( new_thread, SIGNAL( started() ), download, SLOT( startDownload() ) );
                QObject::connect( download, SIGNAL( error( QString )), new_download, SLOT( errorHandler( QString ) ) );
                QObject::connect( download, SIGNAL( finished( unsigned int ) ), new_download,
                        SLOT( finishedHandler( unsigned int ) ) );
                QObject::connect( download, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
                QObject::connect( download, SIGNAL( finished( unsigned int ) ), new_thread, SLOT( quit() ) );
                QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );

                new_thread->start();
                new_download->download_threads.push_back( new_thread );
            }
            active_download_list.insert( new_download->url.toString(), new_download );
            QObject::connect( new_download, SIGNAL( finished( QString ) ), this, SLOT( finishedHandler( QString ) ) );
            QObject::connect( new_download, SIGNAL( error( QString ) ), this, SLOT( errorHandler( QString ) ) );

        } else {
            QUrl new_url( redirected_to );
            qDebug() << "Redirecting to ..." << new_url.toString();
            addNewUrl( new_url.toString(), number_of_threads );
        }
    } else {
        emit error( "Invalid web address or problem with network" );
        qDebug() << "Invalid web address or problem with network";
        return;
    }
}

void NetworkManager::errorHandler( QString what )
{
    qDebug() << "Error: " << what;
    emit error( what );
}

void NetworkManager::finishedHandler( QString url )
{
    active_download_list.remove( url );
    qDebug() << "download completed";
    emit finished( url );
}

void NetworkManager::errorHandler( QNetworkReply::NetworkError what )
{
    switch( what ){
        case QNetworkReply::ContentAccessDenied:
            emit error( "Access denied to content." );
            break;
        case QNetworkReply::AuthenticationRequiredError:
            emit error( "Authentication required" );
            break;
        default:
            emit error( "Some other errors occurred" );
            break;
    }
}

QString NetworkManager::GetFilename( QUrl const & url, QString const & directory )
{
    QFileInfo fileInfo( url.toString() );
    QString basename = fileInfo.fileName();

    if( basename.isEmpty() ){
        QString new_name = QUuid::createUuid();
        if( new_name.contains( '{' ) ){
            new_name.remove( '{' );
        }

        if( new_name.contains( '}' ) ){
            new_name.remove( '}' );
        }
        new_name.resize( 8 );
        basename = new_name;
    }

    QString filepath = directory + QString( "/%1" ).arg( basename );
    if( QFile::exists( filepath ) ){
        int i = 1;
        while( QFile::exists( filepath + QString( "(%1)" ).arg( i ) ) ){
            ++i;
        }
        filepath += QString( "(%1)" ).arg( i );
    }

    return filepath;
}
