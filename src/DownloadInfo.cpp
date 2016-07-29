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

namespace CraneDM {
    DownloadInfo::DownloadInfo( QString const & filename, QObject *parent ): QObject( parent ),
            filename_( filename )
    {

    }

    DownloadInfo::~DownloadInfo()
    {
    }

    QList<QSharedPointer<Information> > DownloadInfo::download_info_list;
    QList<QSharedPointer<Information> > & DownloadInfo::DownloadInfoList()
    {
        return DownloadInfo::download_info_list;
    }

    QSharedPointer<Information> DownloadInfo::UrlSearch( QString const & url )
    {
        QList<QSharedPointer<Information> > &download_list = DownloadInfo::DownloadInfoList();
        for( int index = 0; index != download_list.size(); ++index ){
            if( url == download_list[index]->original_url ) return download_list[index];
        }
        return QSharedPointer<Information>( NULL );
    }

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
            textS << "[]";
            file.close();
        }

        bb::data::JsonDataAccess jda;
        QVariant data = jda.load( filename_ );
        if( jda.hasError() ){
            qDebug() << "Json: " << jda.error().errorMessage();
            return;
        }

        QString const date_format = "dd.MM.yyyy hh:mm:ss.zzz";
        QVariantList file_content = data.toList();

        for( int i = 0; i != file_content.size(); ++i )
        {
            QSharedPointer<Information> info( new Information() );
            QVariantMap key_value = file_content[i].toMap();

            info->original_url = key_value[ "original_url" ].toString();
            info->redirected_url = key_value[ "new_url"].toString();
            info->filename = key_value["filename"].toString();
            info->path_to_file = key_value["path_to_file"].toString();
            info->percentage = key_value[ "percentage" ].toInt();
            info->speed = key_value["speed"].toString();
            info->accept_ranges = key_value["accept_ranges"].toUInt();
            info->download_status = Information::IntToStatus( key_value["status"].toInt() );
            info->size_of_file_in_bytes = key_value["file_size"].toULongLong();

            QVariantList threads = key_value["threads"].toList();
            Information::ThreadInfo thread_info;
            for( int x = 0; x != threads.size(); ++x ){
                QVariantMap thread_info_map = threads[x].toMap();
                thread_info.thread_low_byte = thread_info_map["low_byte"].toUInt();
                thread_info.thread_high_byte = thread_info_map["high_byte"].toUInt();
                thread_info.bytes_written = thread_info_map["bytes_written"].toUInt();
                thread_info.percentage = thread_info_map["percentage"].toUInt();
                thread_info.thread_number = thread_info_map["thread_number"].toUInt();

                info->thread_information_list.push_back( thread_info );
            }

            info->time_started = QDateTime::fromString( key_value["time_started"].toString(), date_format );
            info->time_stopped = QDateTime::fromString( key_value["time_completed"].toString(), date_format );
            DownloadInfo::download_info_list.push_back( info );
        }
    }

    void DownloadInfo::writeDownloadSettingsFile()
    {
        QVariantList root;
        QString const date_format = "dd.MM.yyyy hh:mm:ss.zzz";

        for( int i = 0; i != download_info_list.size(); ++i )
        {
            QSharedPointer<Information> const &item_info = download_info_list[i];
            QVariantMap item_map;
            item_map["original_url"] = item_info->original_url;
            item_map["new_url"] = item_info->redirected_url;
            item_map["filename"] = item_info->filename;
            item_map["path_to_file"] = item_info->path_to_file;
            item_map["time_started"] = item_info->time_started.toString( date_format );
            item_map["time_completed"] = item_info->time_stopped.toString( date_format );
            item_map["speed"] = item_info->speed;
            item_map["status"] = ( unsigned int ) item_info->download_status;
            item_map["percentage"] = item_info->percentage;
            item_map["accept_ranges"] = item_info->accept_ranges;
            item_map["file_size"] = item_info->size_of_file_in_bytes;

            QVariantList thread_info_vlist;
            for( int x = 0; x != item_info->thread_information_list.size(); ++x )
            {
                QVariantMap thread_map;
                thread_map["thread_number"] = item_info->thread_information_list.at( x ).thread_number;
                thread_map["low_byte"] = item_info->thread_information_list.at( x ).thread_low_byte;
                thread_map["high_byte"] = item_info->thread_information_list.at( x ).thread_high_byte;
                thread_map["percentage"] = item_info->thread_information_list.at( x ).percentage;
                thread_map["bytes_written"] = item_info->thread_information_list.at( x ).bytes_written;
                thread_info_vlist.push_back( thread_map );
            }
            item_map["threads"] = thread_info_vlist;
            root.push_back( item_map );
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
        }
        file.close();
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
            case Information::DownloadInProgress:
                return QString( "Active" );
        }
        return QString();
    }

    Information::DownloadStatus Information::IntToStatus( int status )
    {
        return static_cast<DownloadStatus>( status );
    }
}
