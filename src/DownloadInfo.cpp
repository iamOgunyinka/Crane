/*
 * DownloadInfo.cpp
 *
 *  Created on: Jun 22, 2016
 *      Author: Joshua
 */

#include <src/DownloadInfo.hpp>
#include <bb/data/JsonDataAccess.hpp>
#include <QFile>
#include <QDebug>
#include <QList>
#include <QDir>
#include <qtextstream.h>

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
    QFile file( filename_ );
    if( !file.exists() ){
        QDir dir;
        dir.mkpath( "data/" );
        if( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ){
            qDebug() << "Unable to open file.";
            return;
        }
        QTextStream textS( &file );
        textS << "{}";
        file.close();
    }

    bb::data::JsonDataAccess jda;
    QVariant data = jda.load( filename_ );
    if( jda.hasError() ){
        qDebug() << "Json: " << jda.error().errorMessage();
        return;
    }

    QVariantMap file_content = data.toMap();
    QList<QString> urls = file_content.keys();

    for( int i = 0; i != urls.size(); ++i )
    {
        QSharedPointer<Information> info( new Information() );
        QVariantMap key_value = file_content.value( urls.at( i ) ).toMap();

        info->original_url = key_value[ "original_url" ].toString();
        info->redirected_url = key_value[ "new_url"].toString();
        info->filename = key_value["filename"].toString();
        info->time_started = key_value["time_started"].toString();
        info->time_stopped = key_value["time_completed"].toString();
        info->percentage = key_value[ "percentage" ].toInt();
        info->speed = key_value["speed"].toString();

        QVariantList threads = key_value["threads"].toList();
        Information::ThreadInfo thread_info;
        for( int x = 0; x != threads.size(); ++x ){
            QVariantMap thread_info_map = threads[x].toMap();
            thread_info.thread_low_byte = thread_info_map["low_byte"].toUInt();
            thread_info.thread_high_byte = thread_info_map["high_byte"].toUInt();
            thread_info.bytes_written = thread_info_map["bytes_written"].toUInt();
            thread_info.thread_number = thread_info_map["thread_number"].toUInt();

            info->threads.push_back( thread_info );
        }
        info->accept_ranges = key_value["accept_ranges"].toUInt();
        info->accept_ranges = key_value["status"].toInt();
        info->size_of_file_in_bytes = key_value["file_size"].toUInt();

        DownloadInfo::download_info_map.insert( info->original_url, info );
    }
    emit finished();
}

void DownloadInfo::writeDownloadSettingsFile()
{
    QVariantMap root;
    QList<QString> keys = download_info_map.keys();

    Q_ASSERT( keys.size() == download_info_map.size() );

    for( int i = 0; i != keys.size(); ++i )
    {
        QSharedPointer<Information> item_info = download_info_map[ keys[i] ];
        QVariantMap item_map;
        item_map["original_url"] = item_info->original_url;
        item_map["new_url"] = item_info->redirected_url;
        item_map["filename"] = item_info->filename;
        item_map["time_started"] = item_info->time_started;
        item_map["time_completed"] = item_info->time_stopped;
        item_map["speed"] = item_info->speed;
        item_map["percentage"] = item_info->percentage;

        QVariantList thread_info_list;
        for( int i = 0; i != item_info->threads.size(); ++i )
        {
            QVariantMap thread_map;
            thread_map["thread_number"] = item_info->threads.at( i ).thread_number;
            thread_map["low_byte"] = item_info->threads.at( i ).thread_low_byte;
            thread_map["high_byte"] = item_info->threads.at( i ).thread_high_byte;
            thread_map["bytes_written"] = item_info->threads.at( i ).bytes_written;
            thread_info_list.push_back( thread_map );
        }

        item_map["threads"] = thread_info_list;
        item_map["accept_ranges"] = item_info->accept_ranges;
        item_map["file_size"] = item_info->size_of_file_in_bytes;
        item_map["status"] = ( unsigned int ) item_info->download_status;

        root.insert( item_info->original_url, item_map );
    }

    QFile file( filename_ );
    if( !file.exists() ){
        QDir dir;
        dir.mkpath( "data/" );
        if( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ){
            qDebug() << "Unable to open file.";
            return;
        }
    }

    bb::data::JsonDataAccess jda;
    jda.save( root, filename_ );

    if( jda.hasError() ){
        qDebug() << "We have an error: " << jda.error().errorMessage();
        file.close();
    } else {
        file.close();
        qDebug() << "File was saved correctly to: " << file.fileName();
    }
}

QString Information::DownloadStatusToString( Information::DownloadStatus status )
{
    switch( status ){
        case Information::DownloadCompleted:
            return QString( "Completed" );
        case Information::DownloadStopped:
            return QString( "Stopped" );
        case Information::DownloadError:
            return QString( "Error" );
        case Information::DownloadPaused:
            return QString( "Paused" );
        default:
            return QString( "" );
    }
}
