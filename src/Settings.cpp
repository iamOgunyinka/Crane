/*
 * Settings.cpp
 *
 *  Created on: Jun 16, 2016
 *      Author: adonai
 */

#include <src/Settings.hpp>
#include <iostream>
#include <bb/data/JsonDataAccess.hpp>
#include <QDir>
#include <QFile>
#include <stdio.h>

namespace CraneDM {
    Settings::Settings( QObject *parent ): QObject( parent ),
        max_download( 2 ), max_thread( 1 ), app_notify( 0 ),
        autocopy_from_clipboard( 0 ),
        download_directory( "file:///accounts/1000/shared/downloads/" ),
        app_theme( "bright" ), filename( "data/app_settings.dat" )
    {
        if( !QFile::exists( filename ) ){
            writeSettings();
        } else {
            readSettings();
        }
    }

    void Settings::readSettings()
    {
        {
            char const *filename = "data/settings.db";
            if( QFile::exists( filename ) ){
                FILE *old_settings_file = fopen( filename, "rb" );
                if( old_settings_file ){
                    char *download_directory_old = NULL;
                    int download_directory_len = 0;
                    int max_download_old = 0;
                    int max_thread_old = 0;
                    int app_notify_old = 0;
                    int autocopy_from_clipboard_old = 0;

                    fread( &download_directory_len, sizeof( download_directory_len ), 1, old_settings_file );
                    download_directory_old = new char[download_directory_len + 1];
                    fread( download_directory_old, download_directory_len, 1, old_settings_file );
                    download_directory_old[download_directory_len] = '\0';
                    fread( &max_download_old, sizeof( max_download_old ), 1, old_settings_file );
                    fread( &max_thread_old, sizeof( max_thread_old ), 1, old_settings_file );
                    fread( &app_notify_old, sizeof( app_notify_old ), 1, old_settings_file );
                    fread( &autocopy_from_clipboard_old, sizeof( autocopy_from_clipboard_old ), 1, old_settings_file );
                    fclose( old_settings_file );

                    max_download = max_download_old;
                    max_thread = max_thread_old;
                    app_notify = app_notify_old;
                    autocopy_from_clipboard = autocopy_from_clipboard_old;
                    download_directory = QString( download_directory_old );

                    delete[] download_directory_old;
                    download_directory_old = NULL;
                    old_settings_file = NULL;
                } else {
                    qDebug() << "Unable to read file";
                }

                QFile::remove( filename );
                return;
            }
        }
        QFile file( filename );
        if( !file.exists() ){
            QDir dir;
            dir.mkpath( "data/" );
            if( !file.open( QIODevice::ReadWrite | QIODevice::Text ) ){
                qDebug() << "Unable to open file.";
                return;
            }
            QTextStream textS( &file );
            textS << "{}";
            file.close();
        }

        bb::data::JsonDataAccess jda;
        QVariant data = jda.load( filename );
        if( jda.hasError() ){
            qDebug() << "Json: " << jda.error().errorMessage();
            return;
        }

        QVariantMap root_map = data.toMap();
        download_directory = root_map["download_directory"].toString();
        app_theme = root_map["app_theme"].toString();
        max_download = root_map["max_download"].toUInt();
        max_thread = root_map["max_thread"].toUInt();
        app_notify = root_map["app_notify"].toUInt();
        autocopy_from_clipboard = root_map["autocopy_from_clipboard"].toUInt();
    }

    void Settings::writeSettings()
    {
        QVariantMap root_map;
        root_map["download_directory"] = download_directory;
        root_map["app_theme"] = app_theme;
        root_map["max_download"] = max_download;
        root_map["max_thread"] = max_thread;
        root_map["app_notify"] = app_notify;
        root_map["autocopy_from_clipboard"] = autocopy_from_clipboard;

        QFile file( filename );
        bb::data::JsonDataAccess jda;
        jda.save( root_map, filename );

        if( jda.hasError() ){
            qDebug() << "We have an error: " << jda.error().errorMessage();
        }
    }

    Settings::~Settings()
    {
    }


    void Settings::setMaxDownload( int m )
    {
        if( max_download != m ){
            max_download = m;
            emit maxDownloadChanged( m );
            writeSettings();
        }
    }

    void Settings::setMaxThread( int t )
    {
        if( max_thread != t ){
            max_thread = t;
            emit maxThreadChanged( t );
            writeSettings();
        }
    }

    void Settings::setAppNotification( int n )
    {
        if( app_notify != n ){
            app_notify = n;
            emit appNotificationChanged( n );
            writeSettings();
        }
    }

    void Settings::useClipboardText( int e )
    {
        if( autocopy_from_clipboard != e ){
            autocopy_from_clipboard = e;
            emit clipboardUseChanged( e );
            writeSettings();
        }
    }

    void Settings::setAppTheme( QString theme )
    {
        if( theme != app_theme ){
            app_theme = theme;
            emit appThemeChanged( theme );
            writeSettings();
        }
    }

    void Settings::setNewLocation( QString loc )
    {
        if( loc != download_directory ){
            download_directory = loc;
            emit downloadLocationChanged( loc );
            writeSettings();
        }
    }
}
