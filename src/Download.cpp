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
        thread_number_( 0 ), number_of_bytes_written( 0 ),
        file_( NULL ), reply_( NULL )
{

}

DownloadItem::~DownloadItem()
{

}

QNetworkAccessManager DownloadItem::network_manager;
QReadWriteLock        DownloadItem::rw_lock;

void DownloadItem::startDownload( )
{
    QNetworkRequest request( url_ );
    request.setRawHeader( "USER-AGENT", "Mozilla Firefox" );

    QByteArray range = QByteArray("bytes=") + QByteArray::number( low_, 0xA ) + QByteArray( "-" )
                    + QByteArray::number( high_, 0xA );
    if( use_range ){
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
        flush();
    }
}

void DownloadItem::flush()
{
    rw_lock.lockForWrite();
    file_->seek( number_of_bytes_written + low_ );
    qint64 size_of_buffer = file_->write( reply_->readAll() );
    number_of_bytes_written += size_of_buffer;
    qDebug() << "Thread " << thread_number_ << ", written " << number_of_bytes_written;
    rw_lock.unlock();
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
        // pauseDownload();
        emit error( reply_->errorString() );
        return;
    }
    emit finished( thread_number_ );
}

void DownloadItem::downloadProgressHandler( qint64, qint64 )
{

}

DownloadComponent::DownloadComponent( QObject *parent ): QObject( parent ),
        file( NULL ), size_in_bytes( 0 ), accept_ranges( 0 )
{
}

void DownloadComponent::finishedHandler( unsigned int thread_number )
{
    Q_ASSERT( threads.size() == download_threads.size() );
    threads[thread_number-1] = 1;

    for( int i = 0; i != threads.size(); ++i ){
        if( !threads.at( i ) ) return;
    }
    file->close();
    emit finished( url.toString() );
}

void DownloadComponent::errorHandler( QString message )
{
    emit error( message );
}
