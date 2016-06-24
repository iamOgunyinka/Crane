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

void DownloadItem::startDownload( )
{
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

void DownloadItem::downloadProgressHandler( qint64, qint64 )
{

}

DownloadComponent::DownloadComponent( QString address, QString directory, unsigned int number_of_threads, QObject *parent ):
        QObject( parent ), old_url( QUrl( address ) ), new_url( old_url ),
        file( NULL ), size_in_bytes( 0 ), accept_ranges( 0 ), byte_range_specified( 0 ),
        max_number_of_threads( number_of_threads ), download_directory( directory )
{
    if( download_directory.startsWith( "file://" ) ){
        download_directory.remove( "file://" );
    }
}

void DownloadComponent::startDownload()
{
    QNetworkRequest request;
    request.setUrl( new_url );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    QNetworkReply *reply = DownloadItem::GetNetworkManager()->head( request );

    QObject::connect( reply, SIGNAL(finished()), this, SLOT( headFinishedHandler() ) );
    QObject::connect( reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
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
            typedef QPair<QByteArray, QByteArray> HeaderPair;
            QList<HeaderPair> header = response->rawHeaderPairs();
            foreach( HeaderPair hp, header ){
                qDebug() << hp.first << ": " << hp.second;
            }
            addNewUrlImpl( response->url().toString(), response );
        } else {
            new_url = QUrl( redirected_to );
            qDebug() << "Redirecting to ..." << new_url.toString();
            startDownload();
        }
    } else {
        emit error( response->errorString() );
        qDebug() << "Error: " << response->errorString();
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
    Q_ASSERT( threads.size() == download_threads.size() );
    threads[thread_number-1] = 1;

    for( int i = 0; i != threads.size(); ++i ){
        if( !threads.at( i ) ) return;
    }
    file->close();
    delete file;
//    qDebug() << old_url.toString() << " completed successfully.";
    emit finished( old_url.toString() );
}

void DownloadComponent::errorHandler( QString message )
{
    qDebug() << "Error: " << message;
    emit error( message );
}

void DownloadComponent::addNewUrlImpl( QString const & url, QNetworkReply *response )
{
    if( DownloadInfo::DownloadInfoMap().contains( url ) ){
        QSharedPointer<Information> download_information = DownloadInfo::DownloadInfoMap().value( url );
        this->new_url = QUrl( download_information->url );
        QString filename = DownloadComponent::GetFilename( this->new_url, download_directory );
        this->file = new QFile( filename );

        if( !( this->file->open( QIODevice::ReadWrite ) ) ){
            emit error( this->file->errorString() );
            delete this->file;
            return;
        }

        this->accept_ranges = download_information->accept_ranges;
        this->size_in_bytes = download_information->size_of_file_in_bytes;
        this->byte_range_specified = 1;
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

        startDownloadImpl( threads_to_use, list );
        return;
    }

    this->new_url = QUrl( url );
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
    unsigned int threads_to_use = 1;
    if( ( this->size_in_bytes != 0 ) && this->accept_ranges ){
        threads_to_use = max_number_of_threads;
    }
    startDownloadImpl( threads_to_use );
}

void DownloadComponent::startDownloadImpl( unsigned int threads_to_use, QList<Information::ThreadInfo> thread_info_list )
{
    int increment = this->size_in_bytes == 0 ? 0 : ( this->size_in_bytes / threads_to_use );

    unsigned int byte_begin = 0, byte_end = 0;
    bool thread_list_empty = thread_info_list.isEmpty();

    for( unsigned int i = 1; i <= threads_to_use; ++i )
    {
        QThread *new_thread = new QThread( this );
        DownloadItem *download = NULL;
        // mark all the threads as "not finished"
        this->threads.append( 0 );

        if( thread_list_empty ){
            if( i == threads_to_use ){
                // last/only thread to spawn? Download file to the end OR set no limit at all if size is unknown
                download = new DownloadItem( this->new_url, byte_begin, this->size_in_bytes, new_thread );
            } else {
                byte_end += increment;
                download = new DownloadItem( this->new_url, byte_begin, byte_end, new_thread );
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
        QObject::connect( download, SIGNAL( error( QString )), this, SLOT( errorHandler( QString ) ) );
        QObject::connect( download, SIGNAL( finished( unsigned int ) ), this,
                SLOT( finishedHandler( unsigned int ) ) );
        QObject::connect( download, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
        QObject::connect( download, SIGNAL( finished( unsigned int ) ), new_thread, SLOT( quit() ) );
        QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );

        new_thread->start();
        this->download_threads.push_back( new_thread );
    }
    emit downloadStarted( old_url.toString() );
}

QUrl DownloadComponent::redirectUrl( QUrl const & possibleRedirectUrl, QUrl const & oldRedirectUrl ) const
{
    QUrl redirectUrl;
    if( !possibleRedirectUrl.isEmpty() && possibleRedirectUrl != oldRedirectUrl ) {
        redirectUrl = possibleRedirectUrl;
    }
    return redirectUrl;
}

QString DownloadComponent::GetFilename( QUrl const & url, QString const & directory )
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
