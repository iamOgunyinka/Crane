/*
 * ApplicationData.cpp
 *
 *  Created on: Jun 26, 2016
 *      Author: Joshua
 */

#include <src/ApplicationData.hpp>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeTargetReply>
#include <bb/system/InvokeRequest>

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

AppInvoker::AppInvoker( QObject *parent ): QObject( parent )
{

}

AppInvoker::~AppInvoker()
{

}

void AppInvoker::open( QString const & file_path )
{
    bb::system::InvokeManager invoke_manager;
    bb::system::InvokeRequest invoke_request;

    invoke_request.setUri( QUrl::fromUserInput( QString( "file://" ) + file_path ) );
    invoke_request.setAction( "bb.action.OPEN" );

    bb::system::InvokeTargetReply *invoker_reply = invoke_manager.invoke( invoke_request );
    if( invoker_reply ){
        QObject::connect( invoker_reply, SIGNAL( finished() ), this, SLOT( onInvocationCompleted() ) );
    } else {
        emit error( "Unable to open file" );
    }
    invoker_reply->deleteLater();
}

void AppInvoker::onInvocationCompleted()
{
    bb::system::InvokeTargetReply *reply = qobject_cast<bb::system::InvokeTargetReply*>( sender() );

    switch( reply->error() ){
        case bb::system::InvokeReplyError::BadRequest:
            emit error( "Bad request." );
            break;
        case bb::system::InvokeReplyError::Internal:
            emit error( "Internal error" );
            break;
        default:
            break;
    }
    reply->deleteLater();
}

void AppInvoker::onUrlShared( QString url )
{
    emit sharedUrl( url );
}
