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
#include <src/DownloadInfo.hpp>

NetworkManager::NetworkManager( QObject *parent ): QObject( parent ),
max_number_of_threads( 1 ), max_number_of_downloads( 2 )
{
    QThread      *new_thread = new QThread( this );
    DownloadInfo *download_info = new DownloadInfo( "data/download_info.db", new_thread );

    QObject::connect( new_thread, SIGNAL( started() ), download_info, SLOT( readDownloadSettingsFile() ) );
    QObject::connect( download_info, SIGNAL( finished() ), new_thread, SLOT(quit() ) );
    QObject::connect( download_info, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
    QObject::connect( download_info, SIGNAL( error( QString )), this, SLOT( errorHandler( QString ) ) );
    QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );

    new_thread->start();
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

void NetworkManager::addNewUrl( QString const & address, int threads_to_use, int download_limit, QString location )
{
    QNetworkRequest request;
    request.setUrl( QUrl( address ) );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    QNetworkReply *reply = DownloadItem::GetNetworkManager()->head( request );

    max_number_of_threads = threads_to_use;
    max_number_of_downloads = download_limit;

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
            addNewUrlImpl( response->url().toString(), response );
        } else {
            QUrl new_url( redirected_to );
            qDebug() << "Redirecting to ..." << new_url.toString();
            addNewUrl( new_url.toString(), max_number_of_threads, max_number_of_downloads );
        }
    } else {
        emit error( response->errorString() );
        qDebug() << "Error: " << response->errorString();
    }
}

void NetworkManager::addNewUrlImpl( QString const & url, QNetworkReply *response )
{
    if( active_download_list.size() == max_number_of_downloads ){
        inactive_downloads.push_back( url );
        qDebug() << "New download queued up";
        return;
    }

    DownloadComponent *new_download = NULL;

    if( active_download_list.contains( url ) ){
        qDebug() << "URL is currently under download.";
        return;
    } else if( inactive_downloads.contains( url ) ){
        qDebug() << "URL is already queue up";
        return;
    } else if( DownloadInfo::DownloadInfoMap().contains( url ) ){
        QSharedPointer<Information> download_information = DownloadInfo::DownloadInfoMap().value( url );
        new_download = new DownloadComponent();
        new_download->url = QUrl( download_information->url );
        QString filename = NetworkManager::GetFilename( new_download->url, download_directory );
        new_download->file = new QFile( filename );

        if( !( new_download->file->open( QIODevice::ReadWrite ) ) ){
            emit error( new_download->file->errorString() );
            delete new_download->file;
            new_download->deleteLater();
            return;
        }

        new_download->accept_ranges = download_information->accept_ranges;
        new_download->size_in_bytes = download_information->size_of_file_in_bytes;
        new_download->byte_range_specified = 1;
        unsigned int threads_to_use = download_information->number_of_threads_used;

        QList<Information::ThreadInfo> list;
        Information::ThreadInfo thread_info;
        for( int i = 0; i != list.size(); ++i ){
            Information::ThreadInfo *pthread_info = download_information->pthread_info.data() + i;
            thread_info.thread_low_byte = pthread_info->thread_low_byte;
            thread_info.thread_high_byte = pthread_info->thread_high_byte;
            thread_info.thread_number = pthread_info->thread_number;

            list.append( thread_info );
        }

        startDownloadImpl( new_download, threads_to_use, list );
        return;
    }

    new_download = new DownloadComponent();
    new_download->url = QUrl( url );
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

    // if the size of file is unknown, let's use a single thread
    unsigned int threads_to_use = 1;
    if( ( new_download->size_in_bytes != 0 ) && new_download->accept_ranges ){
        threads_to_use = max_number_of_threads;
    }
    startDownloadImpl( new_download, threads_to_use );
}

void NetworkManager::startDownloadImpl( DownloadComponent *new_download, int threads_to_use,
        QList<Information::ThreadInfo> thread_info_list )
{

    int increment = new_download->size_in_bytes == 0 ? 0 : ( new_download->size_in_bytes / threads_to_use );

    unsigned int byte_begin = 0, byte_end = 0;
    bool thread_list_empty = thread_info_list.isEmpty();
    for( unsigned int i = 1; i <= threads_to_use; ++i )
    {
        QThread *new_thread = new QThread( this );
        DownloadItem *download = NULL;
        // mark all the threads as "not finished"
        new_download->threads.append( 0 );

        if( thread_list_empty ){
            if( i == threads_to_use ){
                download = new DownloadItem( new_download->url, byte_begin, new_download->size_in_bytes, new_thread );
            } else {
                byte_end += increment;
                download = new DownloadItem( new_download->url, byte_begin, byte_end, new_thread );
            }
            byte_begin = byte_end + 1;
            download->setThreadNumber( i );
            download->setFile( new_download->file );
            download->acceptRange( false );
        } else {
            Information::ThreadInfo thread_info = thread_info_list.at( i - 1 );
            download = new DownloadItem( new_download->url, thread_info.thread_low_byte,
                    thread_info.thread_high_byte, new_thread );
            download->setThreadNumber( thread_info.thread_number );
            download->setFile( new_download->file );
            download->acceptRange( new_download->accept_ranges );
        }

        QObject::connect( new_thread, SIGNAL( started() ), download, SLOT( startDownload() ) );
        QObject::connect( download, SIGNAL( error( QString )), new_download, SLOT( errorHandler( QString ) ) );
        QObject::connect( download, SIGNAL( finished( unsigned int ) ), new_download,
                SLOT( finishedHandler( unsigned int ) ) );
        QObject::connect( download, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
        QObject::connect( download, SIGNAL( finished( unsigned int ) ), new_thread, SLOT( quit() ) );
        QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );
        QObject::connect( new_thread, SIGNAL(started()), this, SLOT( newDownloadStarted() ) );

        new_thread->start();
        new_download->download_threads.push_back( new_thread );
    }
    qDebug() << "Download started";
    active_download_list.insert( new_download->url.toString(), new_download );
    QObject::connect( new_download, SIGNAL( finished( QString ) ), this, SLOT( finishedHandler( QString ) ) );
    QObject::connect( new_download, SIGNAL( error( QString ) ), this, SLOT( errorHandler( QString ) ) );
}
void NetworkManager::errorHandler( QString what )
{
    qDebug() << "Error: " << what;
    emit error( what );
}

void NetworkManager::finishedHandler( QString url )
{
    DownloadComponent *dc = active_download_list.value( url );
    delete dc->file;
    delete dc;
    active_download_list.remove( url );

    qDebug() << "download completed";
    emit finished( url );

    if( inactive_downloads.size() > 0 ){
        QString next_download = inactive_downloads.front();
        inactive_downloads.pop_front();
        addNewUrl( next_download, max_number_of_threads, max_number_of_downloads );
    }
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
        new_name.resize( 16 );
        basename = new_name + ".nil";
    }

    QString filepath = directory + QString( "/" ) + basename;
    if( QFile::exists( filepath ) ){
        int index_of_extension = basename.lastIndexOf( '.' );
        QString filename = basename.left( index_of_extension ),
                extension = QString::fromStdString( basename.toStdString().substr( index_of_extension + 1 ) );

        int i = 1;
        filepath = directory + QString( "/" ) + filename + QString( "(%1).%2" ).arg( i ).arg( extension );
        while( QFile::exists( filepath ) ){
            ++i;
            filepath = directory + QString( "/" ) + filename + QString( "(%1).%2" ).arg( i ).arg( extension );
        }
    }
    return filepath;
}

void NetworkManager::newDownloadStarted()
{
}
