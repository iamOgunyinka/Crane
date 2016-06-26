/*
 * ApplicationData.cpp
 *
 *  Created on: Jun 26, 2016
 *      Author: adonai
 */

#include <src/ApplicationData.hpp>

ApplicationData::ApplicationData( QObject *parent ): QObject( parent )
{
    // TODO Auto-generated constructor stub

}

DownloadInfo ApplicationData::application_housekeeping_info( "data/download_info.json" );

ApplicationData::~ApplicationData()
{
    // TODO Auto-generated destructor stub
}

