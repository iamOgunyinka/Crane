/*
 * CraneDataModel.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: adonai
 */

#include <src/CraneDataModel.hpp>
#include "DownloadInfo.hpp"

CraneDataModel::CraneDataModel( QObject *parent ): DataModel( parent )
{
}

CraneDataModel::~CraneDataModel()
{
}

bool CraneDataModel::hasChildren( QVariantList const & indexPath )
{
    if( indexPath.size() == 0 || indexPath.size() == 1 ){
        return true;
    }
    return false;
}

int CraneDataModel::childCount( QVariantList const & indexPath )
{
    if( indexPath.size() == 0 ){
        return 1;
    } else {
        if( indexPath.size() == 1 ){
            return DownloadInfo::DownloadInfoMap().size();
        }
        return 0;
    }
}

QString CraneDataModel::itemType( QVariantList const & indexPath )
{
    if( indexPath.size() == 0 ){
        return QString( "header" );
    } else {
        if( indexPath.size() == 1 ){
            return QString( "item" );
        }
        return QString();
    }
}

QVariant CraneDataModel::data( QVariantList const & indexPath )
{
    if( indexPath.size() == 1 ){
        return QVariant();
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
