/*
 * Download.cpp
 *
 *  Created on: Jun 14, 2016
 *      Author: Joshua
*/

#include <src/Download.hpp>
#include <bb/cascades/QMLDocument>

using namespace bb::cascades;

DownloadItem::DownloadItem( QUrl url, qint64 low, qint64 high, qint64 bytes_written, QObject *parent ):
        QObject( parent ), url_( url ), low_( low ), high_( high ),
        number_of_bytes_written_( bytes_written ), use_range_( 0 ),
        thread_number_( 0 ), use_lock_( true ), ftp_resuming_( 0 ),
        file_( NULL ), reply_( NULL )
{
}

DownloadItem::~DownloadItem()
{
}

QNetworkAccessManager DownloadItem::http_network_manager;
QMutex                DownloadItem::mutex;

void DownloadItem::startDownload()
{
    if( number_of_bytes_written_ >= high_ ){
        emit completed();
        return;
    }
    if( url_.scheme() == QString( "ftp" ) ){
        QFtp *ftp = new QFtp;

        ftp->connectToHost( url_.host() ); // command 1.
        ftp->login(); // 2
        if( ftp_resuming_ ){
            ftp->rawCommand( "REST " + QString::number( number_of_bytes_written_ ) );
        } else {
            ftp->rawCommand( "SIZE " + url_.path() ); // 3 ->get the file size
        }
        ftp->get( url_.path() );
        ftp->close();

        QObject::connect( ftp, SIGNAL( readyRead() ), this, SLOT( onFtpReadyReadHandler() ) );
        QObject::connect( ftp, SIGNAL( rawCommandReply( int, QString ) ), this, SLOT( onFtpRawCommandReply( int, QString )) );
        QObject::connect( ftp, SIGNAL( dataTransferProgress(qint64,qint64)), this, SLOT(downloadProgressHandler(qint64,qint64)));
        QObject::connect( ftp, SIGNAL( commandFinished(int,bool) ), this, SLOT( onFtpCommandFinished( int, bool ) ) );
        QObject::connect( ftp, SIGNAL( done( bool ) ), this, SLOT( onFtpDone( bool ) ) );
    } else {
        QNetworkRequest request( url_ );
        request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

        if( use_range_ ){
            QByteArray range = QByteArray( "bytes=" ) + QByteArray::number( number_of_bytes_written_, 0xA ) + QByteArray( "-" )
                        + QByteArray::number( high_ );
            request.setRawHeader( "Range", range );
        }

        timer.start();
        reply_ = DownloadItem::GetHttpNetworkManager()->get( request );
        QObject::connect( reply_, SIGNAL( readyRead() ), this, SLOT( readyReadHandler() ) );
        QObject::connect( reply_, SIGNAL( finished() ), this, SLOT( finishedHandler()) );
        QObject::connect( reply_, SIGNAL(error(QNetworkReply::NetworkError)), this,
                SLOT( errorHandler( QNetworkReply::NetworkError ) ) );
        QObject::connect( reply_, SIGNAL( downloadProgress(qint64,qint64)), this,
                SLOT( downloadProgressHandler( qint64, qint64 ) ) );
    }
}

void DownloadItem::onFtpReadyReadHandler()
{
    QFtp *ftp = qobject_cast<QFtp*>( sender() );
    file_->seek( number_of_bytes_written_ );
    qint64 size_of_buffer = file_->write( ftp->readAll() );
    number_of_bytes_written_ += size_of_buffer;
    emit bytesWrittenStatus( thread_number_, number_of_bytes_written_ );
}

void DownloadItem::onFtpRawCommandReply( int reply_code, QString const & detail )
{
    QFtp *ftp = qobject_cast<QFtp*>( sender() );
    Q_ASSERT( ftp );

    if( ftp->currentId() == 3 && !ftp_resuming_ ){ // getting the size
        QString file_size_string( detail );
        bool success = false;
        qulonglong file_size = file_size_string.toULongLong( &success, 0xA );
        if( success ){
            if( !this->file_->resize( file_size ) ){
                this->file_->close();
                delete this->file_;
                this->file_ = NULL;
                emit error( this->file_->errorString() );
                ftp->abort();
            }
            emit fileSizeObtained( file_size );
        }
    } else if( ftp->currentId() == 4 ) {
        if( !QString::number( reply_code ).startsWith( '2' ) ){ // all successful FTP reply starts with 2
            emit error( detail );
            ftp->abort();
        }
    }
    timer.start();
}

void DownloadItem::onFtpCommandFinished( int command_id, bool b_error )
{
    if( b_error ){
        switch( command_id ){
            case 1: // connect to Host
                emit error( "Unable to connect to host." );
                break;
            case 2: //Login
                emit error( "Unsuccessful login." );
                break;
            case 3:
                emit error( "Unable to get file size." );
                break;
            default:
                emit error( "..." );
                break;
        }
    }
}

void DownloadItem::onFtpDone( bool b_error )
{
    if( b_error ){
        QFtp *ftp = qobject_cast<QFtp*>( sender() );
        emit error( ftp->errorString() );
        return;
    }
    emit completed();
}

void DownloadItem::errorHandler( QNetworkReply::NetworkError err )
{
    switch( err ){
        case QNetworkReply::ConnectionRefusedError:
            emit error( "Connection refused by the host machine." );
            break;
        case QNetworkReply::RemoteHostClosedError:
            emit error( "Remote host closed connection." );
            break;
        case QNetworkReply::ProxyNotFoundError:
            emit error( "Proxy server/host not found." );
            break;
        default:
            emit error( "An unknown error occurred." );
            break;
    }
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
        emit bytesWrittenStatus( thread_number_, number_of_bytes_written_ );
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
    if( url_.scheme() == QString( "ftp" ) ){
        QFtp *ftp = qobject_cast<QFtp*>( sender() );
        ftp->abort();

        file_->seek( number_of_bytes_written_ );
        qint64 size_of_buffer = file_->write( ftp->readAll() );
        number_of_bytes_written_ += size_of_buffer;

        QObject::disconnect( ftp, SIGNAL( readyRead() ), this, SLOT( onFtpReadyReadHandler() ) );
        QObject::disconnect( ftp, SIGNAL( rawCommandReply( int, QString ) ), this, SLOT( onFtpRawCommandReply( int, QString )) );
        QObject::disconnect( ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(downloadProgressHandler(qint64,qint64)));
        QObject::disconnect( ftp, SIGNAL( commandFinished(int,bool) ), this, SLOT( onFtpCommandFinished( int, bool ) ) );
        QObject::disconnect( ftp, SIGNAL( done( bool ) ), this, SLOT( onFtpDone( bool ) ) );

        ftp->deleteLater();
        emit stopped();
        return;
    }

    if( !reply_ ) return;

    reply_->abort();
    flush();

    QObject::disconnect( reply_, SIGNAL( readyRead() ), this, SLOT( readyReadHandler() ) );
    QObject::disconnect( reply_, SIGNAL( finished() ), this, SLOT( finishedHandler() ) );
    QObject::disconnect( reply_, SIGNAL( downloadProgress( qint64, qint64 )), this,
            SLOT( downloadProgressHandler( qint64, qint64 ) ) );
    QObject::disconnect( reply_, SIGNAL(error(QNetworkReply::NetworkError )), this,
            SLOT( errorHandler( QNetworkReply::NetworkError )) );
    reply_->deleteLater();
    emit stopped();
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
    emit completed();
}

void DownloadItem::downloadProgressHandler( qint64 bytes_received, qint64 bytes_total )
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>( sender() );
    bool has_no_error = false;
    if( reply != NULL ){
        has_no_error = ( reply->error() == QNetworkReply::NoError );
    } else {
        QFtp *ftp = qobject_cast<QFtp*>( sender() );
        has_no_error = ( ftp->error() == QFtp::NoError );
    }

    if ( has_no_error ) {
        if( bytes_received == bytes_total ) return;
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
        emit progressStatus( thread_number_, percent, speed, unit );
    }
}

DownloadComponent::DownloadComponent( QString address, QString directory, unsigned int number_of_threads, QObject *parent ):
        QObject( parent ), file( NULL ), byte_range_specified( 0 ), download_information( new Information ),
        max_number_of_threads( number_of_threads ), download_directory( directory )
{
    if( download_directory.startsWith( "file://" ) ){
        download_directory.remove( "file://" );
    }

    download_information->redirected_url = download_information->original_url = address;
    download_information->path_to_file = directory;
    download_information->accept_ranges = 0;
    download_information->percentage = 0;
    download_information->size_of_file_in_bytes = 0;
    download_information->filename = "";
    download_information->path_to_file = "";

    timer.setInterval( 1000000 );
    QObject::connect( &this->timer, SIGNAL( timeout() ), ApplicationData::m_pDownloadInfo.data(),
            SLOT( writeDownloadSettingsFile() ) );
}

DownloadComponent::~DownloadComponent()
{
    cleanUpOnExit();
    emit closed();
}

void DownloadComponent::updateBytesWritten( unsigned int thread_number, qint64 bytes_written )
{
    download_information->thread_information_list[ thread_number - 1 ].bytes_written = bytes_written;
}

void DownloadComponent::onFileSizeObtained( quint64 file_size )
{
    download_information->size_of_file_in_bytes = file_size;
}

void DownloadComponent::startDownload()
{
    timer.start();

    QUrl url( download_information->redirected_url );
    bool isFtp = url.scheme() == QString( "ftp" );

    QSharedPointer<Information> local_download_information = DownloadInfo::UrlSearch( download_information->original_url );

    if( local_download_information ){
        download_information = local_download_information;

        bool download_was_completed = download_information->download_status == Information::DownloadCompleted;

        if( download_was_completed ){
            QFile file( download_information->path_to_file );

            // if the file was completed then and still exists, don't do anything.
            if( file.exists() ){
                emit error( QString( "File already exist." ), download_information->original_url );
                return;
            }
            DownloadInfo::DownloadInfoMap().remove( download_information->time_started );
            download_information->thread_information_list.clear();
            Q_ASSERT( download_information->thread_information_list.size() == 0 );

            startDownload();
            return;
        } else {
            QFile file( download_information->path_to_file );
            // if was completed halfway and it doesn't exist at its location -- create a new file.
            if( !file.exists() ){
                QString new_path = download_directory + "/" + download_information->filename;
                QFile new_file ( new_path );
                if( !new_file.open( QIODevice::ReadWrite ) ){
                    emit error( new_file.errorString(), download_information->original_url );
                    return;
                }
                // we created it successfully, now let's reset the file pointers.
                download_information->path_to_file = new_path;
                download_information->thread_information_list[0].bytes_written = 0;
                download_information->percentage = 0;
                for( int i = 1; i != download_information->thread_information_list.size(); ++i ){
                    download_information->thread_information_list[i].bytes_written =
                            download_information->thread_information_list[i].thread_low_byte;
                    download_information->thread_information_list[i].percentage = 0;
                }
            }
            file.close();
            this->file = new QFile( download_information->path_to_file );
        }

        if( !( this->file->open( QIODevice::ReadWrite ) ) ){
            emit error( this->file->errorString(), download_information->filename );
            delete this->file;
            this->file = NULL;
            return;
        }

        if( !this->file->resize( download_information->size_of_file_in_bytes ) ){
            this->file->close();
            delete this->file;
            this->file = NULL;
            emit error( this->file->errorString(), download_information->original_url );
            return;
        }

        this->byte_range_specified = download_information->thread_information_list.size() > 1;
        download_information->download_status = Information::DownloadInProgress;

        if( isFtp ){
            this->ftpDownloadFile();
        } else {
            startDownloadImpl( download_information->thread_information_list.size() );
        }
        qDebug() << "Download continued...";
        return;
    }

    if( isFtp ){
        addNewFtpUrlImpl( download_information->redirected_url );
        return;
    }
    QNetworkRequest request;
    request.setUrl( download_information->redirected_url );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    QNetworkReply *reply = DownloadItem::GetHttpNetworkManager()->head( request );

    QObject::connect( reply, SIGNAL(finished()), this, SLOT( headFinishedHandler() ) );
    QObject::connect( reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorHandler(QNetworkReply::NetworkError)));
}

void DownloadComponent::cleanUpOnExit()
{
    download_item_list.clear();
    if( file ){
        file->close();
        delete file;
        file = NULL;
    }
    ApplicationData::m_pDownloadInfo->writeDownloadSettingsFile();
}

void DownloadComponent::stopDownload()
{
    for( int i = 0; i != download_item_list.size(); ++i ){
        download_item_list[i]->stopDownload();
    }
    download_item_list.clear();
    download_information->download_status = Information::DownloadStopped;
    if( file ){
        file->close();
        delete file;
        file = NULL;
    }
    emit stopped();
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
            addNewHttpUrlImpl( response->url().toString(), response );
        } else {
            download_information->redirected_url = redirected_to.toString();
            startDownload();
        }
        return;
    }
    emit error( response->errorString(), download_information->original_url );
}

void DownloadComponent::errorHandler( QNetworkReply::NetworkError what )
{
    switch( what ){
        case QNetworkReply::ContentAccessDenied:
            emit error( "Access denied to content.", download_information->original_url );
            break;
        case QNetworkReply::AuthenticationRequiredError:
            emit error( "Authentication required", download_information->original_url );
            break;
        default:
            emit error( "Some other errors occurred", download_information->original_url );
            break;
    }
}

void DownloadComponent::completionHandler()
{
    for( int i = 0; i != download_information->thread_information_list.size(); ++i ){
        Information::ThreadInfo local_thread_info = download_information->thread_information_list.at( i );
        if( local_thread_info.bytes_written < local_thread_info.thread_high_byte ) return;
    }

    download_information->download_status = Information::DownloadCompleted;
    download_information->percentage = 100;
    download_information->time_stopped = QDateTime::currentDateTime();

    file->close();
    delete file;
    file = NULL;
    emit completed( download_information->original_url, download_information->time_started );
}

void DownloadComponent::errorHandler( QString message )
{
    emit error( message, download_information->original_url );
}

void DownloadComponent::addNewFtpUrlImpl( QString const & address )
{
    download_information->filename = DownloadComponent::GetFilename( address, download_directory );
    download_information->path_to_file = download_directory + "/" + download_information->filename;
    this->file = new QFile( download_information->path_to_file );

    if( !( this->file->open( QIODevice::ReadWrite ) ) ){
        emit error( this->file->errorString(), download_information->original_url );
        delete this->file;
        file = NULL;
        return;
    }

    download_information->download_status = Information::DownloadInProgress;
    download_information->time_started = QDateTime::currentDateTime();

    DownloadInfo::DownloadInfoMap().insert( download_information->time_started, download_information );
    ftpDownloadFile();
}

void DownloadComponent::ftpDownloadFile()
{
    QThread *pThread = new QThread;
    DownloadItem *pDownload = NULL;

    qint64 byte_begin = 0, byte_end = 1, bytes_written = 0;
    unsigned int percentage = 0, thread_number = 1;
    bool ftp_download_resuming = !download_information->thread_information_list.isEmpty();
    if( ftp_download_resuming ){
        byte_begin = download_information->thread_information_list[0].thread_low_byte;
        byte_end = download_information->thread_information_list[0].thread_high_byte;
        bytes_written = download_information->thread_information_list[0].bytes_written;
        percentage = download_information->percentage;
    }

    download_information->thread_information_list.push_back( Information::ThreadInfo{ byte_begin, byte_end, bytes_written,
        percentage, thread_number } );
    pDownload = new DownloadItem( QUrl( download_information->redirected_url ), bytes_written, byte_end, bytes_written, pThread );
    pDownload->setThreadNumber( thread_number );
    pDownload->setFile( this->file );

    if( ftp_download_resuming ){
        pDownload->ftpResume( true );
    }
    connectSignalsCallbacks( pDownload, pThread );
    this->download_item_list.push_back( pDownload );
    pThread->start();
}

void DownloadComponent::addNewHttpUrlImpl( QString const & url, QNetworkReply *response )
{
    download_information->redirected_url = url;
    download_information->time_started = QDateTime::currentDateTime();
    download_information->download_status = Information::DownloadInProgress;

    // accepts pause/resume and slicing of files for different threads to process
    if( response->hasRawHeader( QByteArray( "Accept-Ranges" ) ) ){
        download_information->accept_ranges = 1;
    }
    //size of the file to download
    if( response->hasRawHeader( QByteArray( "Content-Length" ) ) ){
        download_information->size_of_file_in_bytes = response->rawHeader( "Content-Length" ).toInt();
    }

    if( response->hasRawHeader( QByteArray( "Content-Disposition" ) ) ){
        QString cdp_str = response->rawHeader( "Content-Disposition" );
        int index_of_filename = cdp_str.indexOf( "filename=" );
        if( index_of_filename != -1 ){
            QString filename_itself = QString::fromStdString( cdp_str.toStdString().substr( index_of_filename + 9 ) );
            if( filename_itself.startsWith( '"' ) ){
                QFileInfo fileInfo( filename_itself );
                download_information->filename = fileInfo.fileName();
                while( download_information->filename.endsWith( '"') ){
                    download_information->filename.chop(1);
                }
            } else {
                download_information->filename = filename_itself;
            }
        }
    }

    if( download_information->filename.isEmpty() ){
        download_information->filename = DownloadComponent::GetFilename( download_information->redirected_url, download_directory );
    }

    download_information->path_to_file = download_directory + "/" + download_information->filename;
    this->file = new QFile( download_information->path_to_file );

    if( !( this->file->open( QIODevice::ReadWrite ) ) ){
        emit error( this->file->errorString(), download_information->original_url );
        delete this->file;
        file = NULL;
        return;
    }

    if( !this->file->resize( download_information->size_of_file_in_bytes ) ){
        this->file->close();
        delete this->file;
        this->file = NULL;
        emit error( this->file->errorString(), download_information->original_url );
        return;
    }

    // if the size of file is unknown, let's use a single thread
    int threads_to_use = 1;
    if( ( download_information->size_of_file_in_bytes != 0 ) && download_information->accept_ranges ){
        threads_to_use = max_number_of_threads;
    }

    DownloadInfo::DownloadInfoMap().insert( download_information->time_started, download_information );
    startDownloadImpl( threads_to_use );
}

void DownloadComponent::startDownloadImpl( int threads_to_use )
{
    int increment = download_information->size_of_file_in_bytes == 0 ? 0 :
            ( download_information->size_of_file_in_bytes / threads_to_use );

    qint64 byte_begin = 0, byte_end = 0;
    bool thread_list_empty = download_information->thread_information_list.isEmpty();

    emit downloadStarted( download_information->original_url );
    for( int i = 1; i <= threads_to_use; ++i )
    {
        QThread *new_thread = new QThread( this );
        DownloadItem *download = NULL;

        if( thread_list_empty ){ // starting download afresh
            // last/only thread to spawn? Download file to the end OR set no limit at all if size is unknown
            if( i == threads_to_use ){
                download_information->thread_information_list.push_back( Information::ThreadInfo{ byte_begin,
                    download_information->size_of_file_in_bytes, byte_begin, 0, i } );
                download = new DownloadItem( QUrl( download_information->redirected_url ), byte_begin,
                        download_information->size_of_file_in_bytes, byte_begin, new_thread );
            } else {
                byte_end += increment;
                download_information->thread_information_list.push_back(
                        Information::ThreadInfo{ byte_begin, byte_end, byte_begin, 0, i } );
                download = new DownloadItem( QUrl( download_information->redirected_url ), byte_begin, byte_end,
                        byte_begin, new_thread );
            }
            byte_begin = byte_end + 1;
            download->setThreadNumber( i );
            download->setFile( this->file );
            if( threads_to_use == 1 ){
                download->acceptRange( false );
                download->useLock( false );
            } else {
                download->acceptRange( true );
                download->useLock( true );
            }
        } else {
            Information::ThreadInfo thread_info = download_information->thread_information_list[i-1];
            download = new DownloadItem{ download_information->redirected_url, thread_info.thread_low_byte,
                thread_info.thread_high_byte, thread_info.bytes_written, new_thread };
            download->setThreadNumber( thread_info.thread_number );
            download->setFile( this->file );
            download->acceptRange( download_information->accept_ranges );
            download->useLock( download_information->thread_information_list.size() != 1 );
        }

        connectSignalsCallbacks( download, new_thread );
        this->download_item_list.push_back( download );
        new_thread->start();
    }
}

void DownloadComponent::connectSignalsCallbacks( DownloadItem *download, QThread *new_thread )
{
    QObject::connect( new_thread, SIGNAL( started() ), download, SLOT( startDownload() ) );
    QObject::connect( download, SIGNAL( fileSizeObtained( quint64 )), this, SLOT( onFileSizeObtained( quint64 ) ) );
    QObject::connect( download, SIGNAL( bytesWrittenStatus( unsigned int, qint64 )), this,
            SLOT( updateBytesWritten( unsigned int, qint64 )) );

    QObject::connect( download, SIGNAL( progressStatus( unsigned int, int, double, QString )), this,
            SLOT( progressStatusHandler( unsigned int, int, double, QString ) ) );
    QObject::connect( download, SIGNAL( error( QString )), this, SLOT( errorHandler( QString ) ) );
    QObject::connect( download, SIGNAL( error( QString )), new_thread, SLOT( quit() ) );
    QObject::connect( download, SIGNAL( completed() ), this, SLOT( completionHandler() ) );
    QObject::connect( download, SIGNAL( completed() ), new_thread, SLOT( quit() ) );
    QObject::connect( download, SIGNAL( stopped()), new_thread, SLOT( quit() ) );
    QObject::connect( new_thread, SIGNAL( finished() ), new_thread, SLOT( deleteLater() ) );
}

void DownloadComponent::progressStatusHandler( unsigned int thread, int percent, double speed, QString unit )
{
    download_information->thread_information_list[thread-1].percentage = percent;
    int number_of_threads = download_information->thread_information_list.size(), x = 0;
    for( int i = 0; i != number_of_threads; ++i ){
        x += download_information->thread_information_list[i].percentage;
    }
    download_information->percentage = ( x / number_of_threads );
    download_information->speed = QString::number( speed ) + unit;
    emit progressChanged( download_information->original_url, download_information->time_started );
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

    int index_of_extension = basename.lastIndexOf( '.' );
    QString file_extension = QString::fromStdString( basename.toStdString().substr( index_of_extension ) );
    if( QFile::exists( directory + QString( "/" ) + basename ) ){
        QString filename = basename.left( index_of_extension );

        int i = 1;
        QString filepath = directory + QString( "/" ) + filename + QString( "(%1)%2" ).arg( i ).arg( file_extension );
        while( QFile::exists( filepath ) ){
            ++i;
            filepath = directory + QString( "/" ) + filename + QString( "(%1)%2" ).arg( i ).arg( file_extension );
        }
        filename = filename + QString( "(%1)" ).arg( i );
        if( filename.length() > 255 ){ // let's not incur the wrath of the FS -- max file size is 255
            filename.resize( 100 );
        }
        return filename + file_extension;
    }

    if( basename.length() > 255 ){ // again, let's not incur the wrath of the file system.
        basename.resize( 100 );
        basename += file_extension;
    }
    return basename; // actually, it's the filename
}
