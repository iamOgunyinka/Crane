/*
 * Download.cpp
 *
 *  Created on: Jun 14, 2016
 *      Author: Joshua
 */

#include <src/Download.hpp>
#include <bb/cascades/QMLDocument>

using namespace bb::cascades;

DownloadItem::DownloadItem( QUrl url, qint64 low, qint64 high, QObject *parent ):
        QObject( parent ), url_( url ), low_( low ), high_( high ),
        number_of_bytes_written_( low_ ), use_range_( 0 ), thread_number_( 0 ),
        use_lock_( true ), file_( NULL ), reply_( NULL )
{
}

DownloadItem::~DownloadItem()
{

}

QNetworkAccessManager DownloadItem::network_manager;
QMutex                DownloadItem::mutex;

void DownloadItem::startDownload()
{
    if( number_of_bytes_written_ >= high_ ){
        emit finished( thread_number_ );
        return;
    }

    QNetworkRequest request( url_ );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    if( use_range_ ){
        QByteArray range = QByteArray("bytes=") + QByteArray::number( low_, 0xA ) + QByteArray( "-" )
                + QByteArray::number( high_, 0xA );
        request.setRawHeader( "Range", range );
    }

    reply_ = DownloadItem::GetNetworkManager()->get( request );
    QObject::connect( reply_, SIGNAL( readyRead() ), this, SLOT( readyReadHandler() ) );
    QObject::connect( reply_, SIGNAL( finished() ), this, SLOT( finishedHandler()) );
    QObject::connect( reply_, SIGNAL( downloadProgress(qint64,qint64)),
            this, SLOT( downloadProgressHandler( qint64, qint64 ) ) );
    timer.start();
}

void DownloadItem::readyReadHandler()
{
    reply_ = qobject_cast<QNetworkReply*>( sender() );
    if( reply_->error() == QNetworkReply::NoError ){
        if( use_lock_ ){
            flush();
        } else {
            flushImpl(); // flush directly, no locks.
        }
    }
    emit status( thread_number_, number_of_bytes_written_ );
}

void DownloadItem::flushImpl()
{
    file_->seek( number_of_bytes_written_ );
    qint64 size_of_buffer = file_->write( reply_->readAll() );
    number_of_bytes_written_ += size_of_buffer;
}

void DownloadItem::flush()
{
    QMutexLocker lock( &mutex );
    flushImpl();
}

void DownloadItem::stopDownload()
{
    reply_->abort();

    flush();
    QObject::disconnect( reply_, SIGNAL( readyRead() ), this, SLOT( readyReadHandler() ) );
    QObject::disconnect( reply_, SIGNAL( finished() ), this, SLOT( finishedHandler()) );
    QObject::disconnect( reply_, SIGNAL( downloadProgress(qint64,qint64)),
            this, SLOT( downloadProgressHandler( qint64, qint64 ) ) );
    reply_->deleteLater();
    emit stopped( thread_number_ );
}

void DownloadItem::finishedHandler()
{
    reply_ = qobject_cast<QNetworkReply*>( sender() );
    // do we have a network problem?
    if( reply_->error() != QNetworkReply::NoError ){
        stopDownload();
        emit error( reply_->errorString() );
        return;
    }
    emit finished( thread_number_ );
}

void DownloadItem::downloadProgressHandler( qint64 bytes_received, qint64 bytes_total )
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );

    if ( reply->error() == QNetworkReply::NoError) {
        qint64 actual_received = ( number_of_bytes_written_ - low_ ) + bytes_received;
        if( actual_received == 0 ){
            return;
        }

        double speed = actual_received * 1000.0 / timer.elapsed();
        qint64 actual_total = ( number_of_bytes_written_ - low_ ) + bytes_total;

        if( actual_total == 0 ){
            return;
        }
        QString unit;
        if (speed < 1024) {
            unit = "bytes/sec";
        } else if (speed < 1024*1024) {
            speed /= 1024;
            unit = "kB/s";
        } else {
            speed /= 1024*1024;
            unit = "MB/s";
        }
        int percent = actual_received * 100 / actual_total;
        emit status( number_of_bytes_written_, percent, speed, unit );
    }
}

DownloadComponent::DownloadComponent( QString address, QString directory, unsigned int number_of_threads, QObject *parent ):
        QObject( parent ), old_url( address ), new_url( old_url ),
        file( NULL ), size_in_bytes( 0 ), accept_ranges( 0 ), byte_range_specified( 0 ),
        max_number_of_threads( number_of_threads ), download_directory( directory )
{
    if( download_directory.startsWith( "file://" ) ){
        download_directory.remove( "file://" );
    }

//    QObject::connect( this, SIGNAL( status( int, QString, QString )), this, SLOT( updateStatus( int, QString, QString )));
}

DownloadComponent::~DownloadComponent()
{
    stopDownload( old_url );
}

Information *DownloadComponent::find( QString const & url )
{
    QMap<QString, QSharedPointer<Information> >::iterator download_info_iterator;
    download_info_iterator = DownloadInfo::DownloadInfoMap().find( url );

    return download_info_iterator != DownloadInfo::DownloadInfoMap().end() ?
            download_info_iterator->data() : NULL;
}

void DownloadComponent::updateCreateDownloadInfo()
{
    Information *download_info = find( old_url );

    bool data_stored_already = download_info != NULL;
    if( data_stored_already ){
        updateDownloadInfo( download_info );
        return;
    }

    download_info = new Information;
    download_info->original_url = old_url;
    download_info->redirected_url = new_url;
    download_info->filename = file->fileName();
    download_info->accept_ranges = this->accept_ranges;
    download_info->size_of_file_in_bytes = this->size_in_bytes;
    download_info->time_started = this->time_started.toString();
    download_info->time_stopped = "-";
    download_info->download_status = Information::DownloadInProgress;

    for( int i = 0; i != thread_data.size(); ++i ){
        download_info->threads.push_back( this->thread_data[i].thread_info );
    }
    DownloadInfo::DownloadInfoMap().insert( old_url, QSharedPointer<Information>( download_info ) );
    ApplicationData::application_housekeeping_info.writeDownloadSettingsFile();
}

void DownloadComponent::updateThread( unsigned int thread_number, qint64 bytes_written )
{
    thread_data[thread_number-1].thread_info.bytes_written = bytes_written;
    updateCreateDownloadInfo();
}

void DownloadComponent::updateDownloadInfo( Information *info, bool isDownloadCompleted )
{
    for( int i = 0; i != info->threads.size(); ++i ){
        info->threads[i] = thread_data[i].thread_info;
    }
    if( isDownloadCompleted ){
        info->download_status = Information::DownloadCompleted;
        this->time_completed = QDateTime::currentDateTime();
        info->time_stopped = this->time_completed.toString();
        ApplicationData::application_housekeeping_info.writeDownloadSettingsFile();
    }
}

void DownloadComponent::startDownload()
{
    if( DownloadInfo::DownloadInfoMap().contains( old_url ) ){
        QSharedPointer<Information> download_information = DownloadInfo::DownloadInfoMap().value( old_url );

        if( download_information->download_status == Information::DownloadCompleted ){
            emit finished( download_information->original_url );
            return;
        }

        this->new_url = download_information->redirected_url;
        this->old_url = download_information->original_url;
        QString filename = download_information->filename;
        this->file = new QFile( filename );

        if( !( this->file->open( QIODevice::ReadWrite ) ) ){
            emit error( this->file->errorString() );
            delete this->file;
            return;
        }
        this->accept_ranges = download_information->accept_ranges;
        this->size_in_bytes = download_information->size_of_file_in_bytes;
        this->byte_range_specified = 1;

        Information::ThreadList list = download_information->threads;
        startDownloadImpl( list.size(), list );
        return;
    }

    QNetworkRequest request;
    request.setUrl( new_url );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    QNetworkReply *reply = DownloadItem::GetNetworkManager()->head( request );

    QObject::connect( reply, SIGNAL(finished()), this, SLOT( headFinishedHandler() ) );
    QObject::connect( reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
}

void DownloadComponent::stopDownload( QString address )
{
    if( address != old_url || address != new_url ){
        return;
    }
    for( int i = 0; i != download_threads.size(); ++i ){
        download_threads[i]->terminate();
    }
    download_threads.clear();
    if( file ){
        file->close();
        delete file;
    }
    ApplicationData::application_housekeeping_info.writeDownloadSettingsFile();
}

void DownloadComponent::headFinishedHandler()
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
            new_url = redirected_to.toString();
            startDownload();
        }
    } else {
        emit error( response->errorString() );
    }
}

void DownloadComponent::errorHandler( QNetworkReply::NetworkError what )
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

void DownloadComponent::finishedHandler( unsigned int thread_number )
{
    Q_ASSERT( thread_data.size() == download_threads.size() );
    thread_data[thread_number-1].thread_completed = 1;

    for( int i = 0; i != thread_data.size(); ++i ){
        if( !thread_data.at( i ).thread_completed ) return;
    }
    updateDownloadInfo( find( old_url ), true );
    file->close();
    delete file;
    emit finished( old_url );
}

void DownloadComponent::errorHandler( QString message )
{
    emit error( message );
}

void DownloadComponent::addNewUrlImpl( QString const & url, QNetworkReply *response )
{

    this->new_url = url;
    QString filename = DownloadComponent::GetFilename( this->new_url, download_directory );

    this->file = new QFile( filename );
    if( !( this->file->open( QIODevice::ReadWrite ) ) ){
        emit error( this->file->errorString() );
        delete this->file;
        return;
    }

    // accepts pause/resume and slicing of files for different threads to process
    if( response->hasRawHeader( QByteArray( "Accept-Ranges" ) ) ){
        this->accept_ranges = 1;
    } else {
        this->accept_ranges = 0;
    }
    //size of the file to download
    if( response->hasRawHeader( QByteArray( "Content-Length" ) ) ){
        this->size_in_bytes = response->rawHeader( "Content-Length" ).toInt();
    } else {
        this->size_in_bytes = 0;
    }

    if( !this->file->resize( this->size_in_bytes ) ){
        this->file->close();
        delete this->file;

        emit error( this->file->errorString() );
        return;
    }
    this->time_started = QDateTime::currentDateTime();

    // if the size of file is unknown, let's use a single thread
    int threads_to_use = 1;
    if( ( this->size_in_bytes != 0 ) && this->accept_ranges ){
        threads_to_use = max_number_of_threads;
    }
    startDownloadImpl( threads_to_use );
}

void DownloadComponent::startDownloadImpl( int threads_to_use, QList<Information::ThreadInfo> thread_info_list )
{
    int increment = this->size_in_bytes == 0 ? 0 : ( this->size_in_bytes / threads_to_use );

    qint64 byte_begin = 0, byte_end = 0;
    bool thread_list_empty = thread_info_list.isEmpty();

    this->thread_data.reserve( threads_to_use );

    for( int i = 1; i <= threads_to_use; ++i )
    {
        QThread *new_thread = new QThread( this );
        DownloadItem *download = NULL;
        this->percentage.push_back( 0 );

        if( thread_list_empty ){
            // last/only thread to spawn? Download file to the end OR set no limit at all if size is unknown
            if( i == threads_to_use ){
                // mark all the threads as "not finished"
                thread_data.push_back( ThreadData{ Information::ThreadInfo{ byte_begin, size_in_bytes, byte_begin, i }, 0 } );
                download = new DownloadItem( QUrl( this->new_url ), byte_begin, this->size_in_bytes, new_thread );
            } else {
                byte_end += increment;
                thread_data.push_back( ThreadData{ Information::ThreadInfo{ byte_begin, byte_end, byte_begin, i }, 0 } );
                download = new DownloadItem( QUrl( this->new_url ), byte_begin, byte_end, new_thread );
            }
            byte_begin = byte_end + 1;
            download->setThreadNumber( i );
            download->setFile( this->file );
            if( threads_to_use == 1 ){
                download->acceptRange( false );
                download->useLock( false );
            } else {
                download->acceptRange( true );
                download->useLock();
            }
        } else {
            Information::ThreadInfo thread_info = thread_info_list.at( i - 1 );
            download = new DownloadItem( this->new_url, thread_info.thread_low_byte,
                    thread_info.thread_high_byte, new_thread );
            download->setThreadNumber( thread_info.thread_number );
            download->setFile( this->file );
            download->acceptRange( this->accept_ranges );
            download->useLock();
        }

        QObject::connect( new_thread, SIGNAL( started() ), download, SLOT( startDownload() ) );
        QObject::connect( download, SIGNAL( status( unsigned int, qint64 )), this, SLOT( updateThread( unsigned int, qint64 )) );

        QObject::connect( download, SIGNAL( status( unsigned int, int, double, QString )),
                this, SLOT( statusHandler( unsigned int, int, double, QString ) ) );

        QObject::connect( download, SIGNAL( error( QString )), this, SLOT( errorHandler( QString ) ) );
        QObject::connect( download, SIGNAL( finished( unsigned int ) ), this,
                SLOT( finishedHandler( unsigned int ) ) );
        QObject::connect( download, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
        QObject::connect( download, SIGNAL( finished( unsigned int ) ), new_thread, SLOT( quit() ) );
        QObject::connect( new_thread, SIGNAL( terminated() ), download, SLOT( stopDownload() ) );
        QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );

        new_thread->start();
        this->download_threads.push_back( new_thread );
    }
    emit downloadStarted( old_url );
}

void DownloadComponent::statusHandler( unsigned int thread_no, int percent, double speed, QString unit )
{
    /*
    this->percentage[ thread_no - 1 ] = percent;
    percent = 0;
    for( int i = 0; i != percentage.size(); ++i ){
        percent += percentage[i];
    }

    Information *info = find( old_url );
    if( info != NULL ) {
        info->percentage = percent / 4;
        info->speed = QString::number( speed ) + unit;
    }

    emit statusChanged( old_url );
    */
}

QUrl DownloadComponent::redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl ) const
{
    QUrl redirectUrl;
    if( !possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl ) {
        redirectUrl = possibleRedirectUrl;
    }
    return redirectUrl;
}

QString DownloadComponent::GetFilename( QString const & url, QString const & directory )
{
    QFileInfo fileInfo( url );
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
