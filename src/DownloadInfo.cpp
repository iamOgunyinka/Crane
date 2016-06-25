/*
 * DownloadInfo.cpp
 *
 *  Created on: Jun 22, 2016
 *      Author: Joshua
 */

#include <src/DownloadInfo.hpp>
#include <stdio.h>
#include <QDebug>

DownloadInfo::DownloadInfo( QString const & filename, QObject *parent ): QObject( parent ),
        filename_( filename )
{

}

DownloadInfo::~DownloadInfo()
{
}

QMap<QString, QSharedPointer<Information> > DownloadInfo::download_info_map;
QMap<QString, QSharedPointer<Information> > & DownloadInfo::DownloadInfoMap(){ return DownloadInfo::download_info_map; }

void DownloadInfo::readDownloadSettingsFile()
{
    FILE *file = fopen( filename_.toStdString().c_str(), "rb" );
    if( !file ){ // file doesn't exist
        fclose( file );
        file = fopen( filename_.toStdString().c_str(), "wb" );
        if( !file ){
            emit error( "Unable to create download info file." );
            return;
        }
        int reads = 0;
        fwrite( &reads, sizeof( int ), 1, file );
        fclose( file );
        emit finished();
        return;
    }

    unsigned int number_of_downloads = 0;
    fread( &number_of_downloads, sizeof( int ), 1, file );

    qDebug() << "Reading download files";
    Information::CustomDeleter deleter;

    for( unsigned int i = 0; i != number_of_downloads; ++i )
    {
        QSharedPointer<Information> info( new Information() );
        fread( &( info->info_header.old_url_length ), sizeof( unsigned int ), 1, file );
        fread( &( info->info_header.new_url_length ), sizeof( unsigned int ), 1, file );
        fread( &( info->info_header.filename_length ), sizeof( unsigned int ), 1, file );
        fread( &( info->info_header.time_started_length ), sizeof( unsigned int ), 1, file );
        fread( &( info->info_header.time_ended_length ), sizeof( unsigned int ), 1, file );
        fread( &( info->download_completed ), sizeof( unsigned int ), 1, file );

        info->original_url.resize( info->info_header.old_url_length + 1 );
        info->redirected_url.resize( info->info_header.new_url_length + 1 );
        info->filename.resize( info->info_header.filename_length + 1 );

        fread( info->original_url.data(), sizeof( QChar ), info->info_header.old_url_length, file );
        fread( info->redirected_url.data(), sizeof( QChar ), info->info_header.new_url_length, file );
        fread( info->filename.data(), sizeof( QChar ), info->info_header.filename_length, file );

        info->original_url[info->info_header.old_url_length] = '\0';
        info->redirected_url[info->info_header.new_url_length] = '\0';
        info->filename[info->info_header.filename_length] = '\0';

        fread( &info->number_of_threads_used, sizeof( unsigned int ), 1, file );
        if( info->number_of_threads_used == 1 ){
            info->pthread_info = Information::SP_ThreadInfo( new Information::ThreadInfo );
        } else {
            info->pthread_info = Information::SP_ThreadInfo( new Information::ThreadInfo[info->number_of_threads_used], deleter );
        }
        for( unsigned int x = 0; x != info->number_of_threads_used; ++x ){
            fread( &info->pthread_info->thread_low_byte, sizeof( qint64 ), 1, file );
            fread( &info->pthread_info->thread_high_byte, sizeof( qint64 ), 1, file );
            fread( &info->pthread_info->bytes_written, sizeof( qint64 ), 1, file );
            fread( &info->pthread_info->thread_number, sizeof( unsigned int ), 1, file );
        }

        fread( &info->size_of_file_in_bytes, sizeof( qint64 ), 1, file );
        fread( &info->accept_ranges, sizeof( unsigned int ), 1, file );

        info->time_started.resize( info->info_header.time_started_length + 1 );
        fread( info->time_started.data(), sizeof( QChar ), info->info_header.time_started_length, file );
        info->time_started[info->info_header.time_started_length] = '\0';

        if( info->info_header.time_ended_length != 0 ){
            info->time_stopped.resize( info->info_header.time_ended_length + 1 );
            fread( info->time_stopped.data(), sizeof( QChar ), info->info_header.time_ended_length, file );
            info->time_stopped[info->info_header.time_ended_length] = '\0';
        }

        download_info_map.insert( info->original_url, info );
    }
    fclose( file );
    emit finished();
}

void DownloadInfo::writeDownloadSettingsFile()
{
    FILE *file = fopen( filename_.toStdString().c_str(), "wb" );

    Q_ASSERT( file != NULL && "File should exist by now." );

    int size_of_downloads = download_info_map.size();
    fwrite( &size_of_downloads, sizeof( unsigned int ), 1, file );
    for( QMap<QString, QSharedPointer<Information> >::const_iterator cbegin = download_info_map.constBegin();
            cbegin != download_info_map.constEnd(); ++cbegin )
    {
        Information *p_info = cbegin->data();
        fwrite( &p_info->info_header.old_url_length, sizeof( unsigned int ), 1, file );
        fwrite( &p_info->info_header.new_url_length, sizeof( unsigned int ), 1, file );
        fwrite( &p_info->info_header.filename_length, sizeof( unsigned int ), 1, file );
        fwrite( &p_info->info_header.time_started_length, sizeof( unsigned int ), 1, file );
        fwrite( &p_info->info_header.time_ended_length, sizeof( unsigned int ), 1, file );
        fwrite( &p_info->download_completed, sizeof( unsigned int ), 1, file );

        fwrite( p_info->original_url.data(), sizeof( QChar ), p_info->original_url.length(), file );
        fwrite( p_info->redirected_url.data(), sizeof( QChar ), p_info->redirected_url.length(), file );
        fwrite( p_info->filename.data(), sizeof( QChar ), p_info->filename.length(), file );
        fwrite( &p_info->number_of_threads_used, sizeof( unsigned int ), 1, file );

        for( unsigned int i = 0; i != p_info->number_of_threads_used; ++i ){
            fwrite( &p_info->pthread_info->thread_low_byte, sizeof( qint64 ), 1, file );
            fwrite( &p_info->pthread_info->thread_high_byte, sizeof( qint64 ), 1, file );
            fwrite( &p_info->pthread_info->bytes_written, sizeof( qint64 ), 1, file );
            fwrite( &p_info->pthread_info->thread_number, sizeof( unsigned int ), 1, file );
        }

        fwrite( &p_info->size_of_file_in_bytes, sizeof( qint64 ), 1, file );
        fwrite( &p_info->accept_ranges, sizeof( unsigned int ), 1, file );

        fwrite( p_info->time_started.data(), sizeof( QChar ), p_info->info_header.time_started_length, file );
        if( p_info->info_header.time_ended_length != 0 ){
            fwrite( p_info->time_stopped.data(), sizeof( QChar ), p_info->info_header.time_ended_length, file );
        }
    }

    fclose( file );
}
