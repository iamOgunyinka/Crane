/*
 * Settings.hpp
 *
 *  Created on: Jun 16, 2016
 *      Author: Joshua
 */

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

#include <QObject>
#include <QString>

namespace CraneDM {
    class Settings: public QObject
    {
        Q_OBJECT
        Q_PROPERTY( QString location READ location WRITE setNewLocation NOTIFY downloadLocationChanged )
        Q_PROPERTY( int max_download READ maxDownload WRITE setMaxDownload NOTIFY maxDownloadChanged )
        Q_PROPERTY( int max_thread READ maxThread WRITE setMaxThread NOTIFY maxThreadChanged )
        Q_PROPERTY( int app_notify READ appNotification WRITE setAppNotification NOTIFY appNotificationChanged )
        Q_PROPERTY( int autocopy_from_clipboard READ clipboardText WRITE useClipboardText NOTIFY clipboardUseChanged )
        Q_PROPERTY( QString app_theme READ appTheme WRITE setAppTheme NOTIFY appThemeChanged )
    public:

    private:
        quint16         max_download;
        quint16         max_thread;
        quint16         app_notify;
        quint16         autocopy_from_clipboard;

        QString         download_directory;
        QString         app_theme;
        QString const   filename;

        void writeSettings();
        void readSettings();
    public:
        Settings( QObject *parent = NULL );
        virtual ~Settings();

        int maxThread() const { return max_thread; }
        int maxDownload() const { return max_download; }
        int appNotification() const { return app_notify; }
        int clipboardText() const { return autocopy_from_clipboard; }
        QString location() const { return QString( download_directory ); }
        QString appTheme() const { return app_theme; }

        void setNewLocation( QString loc );
        void setMaxDownload( int m );
        void setMaxThread( int t );
        void setAppNotification( int m );
        void useClipboardText( int e );
        void setAppTheme( QString new_theme );

        signals:
        void error( QString message );
        void maxDownloadChanged( int );
        void maxThreadChanged( int );
        void appNotificationChanged( int );
        void pauseOnErrorChanged( int );
        void clipboardUseChanged( int );
        void downloadLocationChanged( QString );
        void appThemeChanged( QString );
    };
}
#endif /* SETTINGS_HPP_ */
