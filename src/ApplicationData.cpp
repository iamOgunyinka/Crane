/*
 * ApplicationData.cpp
 *
 *  Created on: Jun 26, 2016
 *      Author: Joshua
 */

#include <src/ApplicationData.hpp>

ApplicationData::ApplicationData( QObject *parent ): QObject( parent )
{

}

QSharedPointer<DownloadInfo>    ApplicationData::m_pDownloadInfo ( new DownloadInfo( "data/download_info.json" ) );

ApplicationData::~ApplicationData()
{
    // TODO Auto-generated destructor stub
}

