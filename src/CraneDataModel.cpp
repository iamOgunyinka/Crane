/*
 * CraneDataModel.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: Joshua
 */

#include <src/CraneDataModel.hpp>
#include "DownloadInfo.hpp"

CraneDataModel::CraneDataModel( QObject *parent ): DataModel( parent ), i( 0 )
{
    InstallExtensions();
}

CraneDataModel::~CraneDataModel()
{
}

bool CraneDataModel::hasChildren( QVariantList const & indexPath )
{
    if( indexPath.size() == 0 ){
        return true;
    }
    return false;
}

int CraneDataModel::childCount( QVariantList const & indexPath )
{
    if( indexPath.size() == 0 ){
        return DownloadInfo::DownloadInfoMap().size();
    }
    return 0;
}

QString CraneDataModel::itemType( QVariantList const & indexPath )
{
    if( indexPath.size() == 1 ){
        return QString( "item" );
    }
    return QString();
}

QVariant CraneDataModel::data( QVariantList const & indexPath )
{
    if( indexPath.size() == 1 ){
        QDateTime date_time = DownloadInfo::DownloadInfoMap().keys().at( indexPath.at( 0 ).toInt() );
        QSharedPointer<Information> information = DownloadInfo::DownloadInfoMap().value( date_time );
        QVariantMap data_to_send;

        data_to_send[ "original_url" ] = information->original_url;
        data_to_send[ "filename" ] = information->filename;
        data_to_send[ "total_size"] = CraneDataModel::ConvertByte( information->size_of_file_in_bytes );
        data_to_send[ "status" ] = Information::DownloadStatusToString( information->download_status );
        data_to_send["percentage"] = information->percentage;
        data_to_send[ "speed" ] = information->speed;
        data_to_send[ "image" ] = GetDownloadLogoForFileExtension( information->filename );

        qint64 x = 0;
        for( int i = 0; i != information->thread_information_list.size(); ++i ){
            x += information->thread_information_list[i].bytes_written;
        }
        data_to_send[ "downloaded_size" ] = CraneDataModel::ConvertByte( x );
        return data_to_send;
    }
    return QVariant();
}

QString CraneDataModel::ConvertByte( qint64 size )
{
    QString unit;
    double new_size = ( double ) size; // bug, I'll find a work-around for this.
    if ( new_size < 1024) {
        unit = "B";
    } else if ( new_size < 1024*1024) {
        new_size /= 1024;
        unit = "KB";
    } else if( new_size < ( 1024 * 1024 * 1024 ) ){
        new_size /= ( 1024 * 1024 );
        unit = "MB";
    } else {
        new_size /= ( 1024 * 1024 * 1024 );
        unit = "GB";
    }
    return QString::number( new_size, 'f', 2 ) + unit;
}

QMap<QString, QString> CraneDataModel::ExtensionLogoMap;

void CraneDataModel::InstallExtensions()
{
    ExtensionLogoMap.insert( QString( "mp3" ), QString( "asset:///images/music.png" ) );
    ExtensionLogoMap.insert( QString( "mp4" ), QString( "asset:///images/video.png" ) );
    ExtensionLogoMap.insert( QString( "png" ), QString( "asset:///images/picture.png" ) );
    ExtensionLogoMap.insert( QString( "jpeg" ), QString( "asset:///images/picture.png" ) );
    ExtensionLogoMap.insert( QString( "zip" ), QString( "asset:///images/other.png" ) );
    ExtensionLogoMap.insert( QString( "pdf" ), QString( "asset:///images/doc.png" ) );
    ExtensionLogoMap.insert( QString( "doc" ), QString( "asset:///images/doc.png" ) );
    ExtensionLogoMap.insert( QString( "exe" ), QString( "asset:///images/other.png" ) );
    ExtensionLogoMap.insert( QString( "jpg" ), QString( "asset:///images/picture.png" ) );
}

QString  CraneDataModel::GetDownloadLogoForFileExtension( QString const & filename )
{
    int index_of_extension = filename.lastIndexOf( '.' );
    QString extension = QString::fromStdString( filename.toStdString().substr( index_of_extension + 1 ) );

    if( CraneDataModel::ExtensionLogoMap.contains( extension ) ){
        return CraneDataModel::ExtensionLogoMap.value( extension );
    }
    return QString( "asset:///images/other.png" );
}

CraneFilteredDataModel::CraneFilteredDataModel( bb::cascades::DataModel *data_model, QObject *parent ):
        DataModel( parent ), m_pDataModel( data_model )
{

}

CraneFilteredDataModel::~CraneFilteredDataModel()
{

}

bool CraneFilteredDataModel::hasChildren( QVariantList const & indexPath )
{
    return m_pDataModel->hasChildren( indexPath );
}

int CraneFilteredDataModel::childCount( QVariantList const & indexPath )
{
    return m_pDataModel->childCount( indexPath );
}

QString CraneFilteredDataModel::itemType( QVariantList const & indexPath )
{
    return m_pDataModel->itemType( indexPath );
}

QVariant CraneFilteredDataModel::data( QVariantList const & indexPath )
{
    return m_pDataModel->data( indexPath );
}

void CraneFilteredDataModel::changeView( int view )
{
    Q_UNUSED( view );
}

void CraneFilteredDataModel::refreshView( QString url )
{
    qDebug() << "Called to refresh view with: " << url;
}
