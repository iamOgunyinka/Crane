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

ApplicationClipBoard::ApplicationClipBoard( QObject *parent ): QObject( parent ), text( "" )
{

}

ApplicationClipBoard::~ApplicationClipBoard()
{
}

QString ApplicationClipBoard::clipboardText()
{
    text.clear();
    QByteArray data = clip_board.value( "text/plain" );
    if( !data.isEmpty() ){
        text = data;
    }
    return text;
}


void ApplicationClipBoard::setClipboardText( QString text_ )
{
    clip_board.clear();
    clip_board.insert( QByteArray( "text/plain" ), text_.toUtf8() );
    emit clipboardTextChanged( text );
}
