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
        QString url = DownloadInfo::DownloadInfoMap().keys().at( indexPath.at( 0 ).toInt() );
        QSharedPointer<Information> information = DownloadInfo::DownloadInfoMap().value( url );
        QVariantMap data_to_send;
        data_to_send[ "original_url" ] = url;
        data_to_send[ "file_name" ] = information->filename;
        data_to_send[ "file_size"] = QString::number( information->size_of_file_in_bytes );
        data_to_send[ "status" ] = Information::DownloadStatusToString( information->download_status );
        qint64 x = 0;
        for( int i = 0; i != information->threads.size(); ++i ){
            x += information->threads[i].bytes_written;
        }
        data_to_send[ "downloaded_size" ] = QString::number( x );
        return data_to_send;
    }
    return QVariant();
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
    qDebug() << "Refresh called with: " << url;
}
