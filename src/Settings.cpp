/*
 * Settings.cpp
 *
 *  Created on: Jun 16, 2016
 *      Author: adonai
 */

#include <src/Settings.hpp>
#include <iostream>

Settings::Settings( QObject *parent ): QObject( parent ),
    download_directory_len( 0 ), download_directory( NULL ), max_download( 2 ),
    max_thread( 1 ), app_notify( 0 ), pause_on_error( 0 ), file( NULL )
{
    file = fopen( "data/settings.db", "rb" );
    if( !file ){
        writeSettings();
    } else {
        readSettings();
    }
}

void Settings::readSettings()
{
    if( file ){
        fclose( file );
    }

    file = fopen( "data/settings.db", "rb" );
    if( file ){
        fread( &download_directory_len, sizeof( download_directory_len ), 1, file );
        std::cout << download_directory_len << std::endl;

        download_directory = new char[download_directory_len + 1];
        fread( download_directory, download_directory_len, 1, file );
        download_directory[download_directory_len] = '\0';

        fread( &max_download, sizeof( max_download ), 1, file );

        fread( &max_thread, sizeof( max_thread ), 1, file );

        fread( &app_notify, sizeof( app_notify ), 1, file );

        fread( &pause_on_error, sizeof( pause_on_error ), 1, file );
        fclose( file );
    } else {
        emit error( "Unable to read file" );
    }
}

void Settings::writeSettings()
{
    if( file ){
        fclose( file );
    }

    file = fopen( "data/settings.db", "wb" );

    if( download_directory == NULL ){
        char const *default_download_dir = "file:///accounts/1000/shared/downloads/";
        download_directory_len = strlen( default_download_dir );

        download_directory = new char[download_directory_len + 1];
        strcpy( download_directory, default_download_dir );
        download_directory[download_directory_len] = '\0';
    }

    fwrite( &download_directory_len, sizeof( download_directory_len ), 1, file );
    fwrite( download_directory, download_directory_len, 1, file );
    fwrite( &max_download, sizeof( max_download ), 1, file );
    fwrite( &max_thread, sizeof( max_thread ), 1, file );
    fwrite( &app_notify, sizeof( app_notify ), 1, file );
    fwrite( &pause_on_error, sizeof( pause_on_error ), 1, file );

    fclose( file );
}

Settings::~Settings()
{
    writeSettings();
    if( download_directory != NULL ) delete[] download_directory;
}


void Settings::setMaxDownload( int m )
{
    if( max_download != m ){
        max_download = m;
        emit maxDownloadChanged( m );
    }
}

void Settings::setMaxThread( int t )
{
    if( max_thread != t ){
        max_thread = t;
        emit maxThreadChanged( t );
    }
}

void Settings::setAppNotification( int n )
{
    if( app_notify != n ){
        app_notify = n;
        emit appNotificationChanged( n );
    }
}

void Settings::setPauseOnError( int e )
{
    if( pause_on_error != e ){
        pause_on_error = e;
        emit pauseOnErrorChanged( e );
    }
}

void Settings::setNewLocation( QString loc )
{
    if( strcmp( loc.toStdString().c_str(), download_directory ) != 0 ){
        delete []download_directory;

        download_directory_len = loc.size();
        download_directory = new char[download_directory_len + 1];
        strcpy( download_directory, loc.toStdString().c_str() );
        download_directory[download_directory_len] = '\0';

        emit downloadLocationChanged( loc );
    }
}
