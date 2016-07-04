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
        Information::DateTime date_time = DownloadInfo::DownloadInfoMap().keys().at( indexPath.at( 0 ).toInt() );
        QSharedPointer<Information> information = DownloadInfo::DownloadInfoMap().value( date_time.date_time_ );
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
            x += ( information->thread_information_list[i].bytes_written - information->thread_information_list[i].thread_low_byte );
        }
        data_to_send[ "downloaded_size" ] = CraneDataModel::ConvertByte( x );
        data_to_send["path"] = information->path_to_file;
        data_to_send["resumable"] = information->accept_ranges;

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

void CraneFilteredDataModel::removeItem( QVariantList const & indexPath )
{
    if( indexPath.size() == 1 ){
        DownloadInfo::DownloadInfoMap().remove( DownloadInfo::DownloadInfoMap().keys()[ indexPath[0].toInt() ]);
    }
    emit itemRemoved( indexPath );
}

void CraneFilteredDataModel::refreshView()
{
    int size_of_map = DownloadInfo::DownloadInfoMap().size();
    emit itemsChanged( bb::cascades::DataModelChangeType::AddRemove, QSharedPointer<bb::cascades::DataModel::IndexMapper>(
            new MyIndexMapper( size_of_map, size_of_map, false ))
            );
}

void CraneFilteredDataModel::progressHandler( QString url, QDateTime date_time )
{
    QVariantList indexPath;
    int index_of_download = DownloadInfo::DownloadInfoMap().keys().indexOf( date_time );
    indexPath.append( index_of_download );

    Q_UNUSED( url );
    emit itemUpdated( indexPath );
}

MyIndexMapper::MyIndexMapper( int index, int count, bool deleted ):
        m_index( index ), m_count( count ), m_deleted( deleted )
{

}

bool MyIndexMapper::newIndexPath( QVariantList *pNewIndexPath, int *replacementIndex, QVariantList const & oldIndexPath ) const
{
    if ( oldIndexPath[0].toInt() < m_index ) {
        pNewIndexPath->append( oldIndexPath );
        return true;
    } else if ( m_deleted && oldIndexPath[0].toInt() <= m_index + m_count ) {
        *replacementIndex = m_index;
        return false;
    } else {
        if ( m_deleted ){
            pNewIndexPath->append( oldIndexPath[0].toInt() - m_count );
        } else {
            pNewIndexPath->append( oldIndexPath[0].toInt() + m_count );
        }
        return true;
    }
}
