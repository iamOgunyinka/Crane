/*
 * ApplicationData.hpp
 *
 *  Created on: Jun 26, 2016
 *      Author: adonai
 */

#ifndef APPLICATIONDATA_HPP_
#define APPLICATIONDATA_HPP_

#include <QObject>
#include "DownloadInfo.hpp"
#include <bb/system/Clipboard>
class ApplicationData : public QObject
{
    Q_OBJECT
public:
    ApplicationData( QObject *parent = NULL );
    virtual ~ApplicationData();

    static QSharedPointer<DownloadInfo>    m_pDownloadInfo;
};

class ApplicationClipBoard : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QString text READ clipboardText WRITE setClipboardText NOTIFY clipboardTextChanged );

public:
    ApplicationClipBoard( QObject *parent = NULL );
    ~ApplicationClipBoard();
private:
    QString text;
    bb::system::Clipboard clip_board;
public:
    Q_INVOKABLE QString clipboardText();
    Q_INVOKABLE void setClipboardText( QString );
signals:
    void clipboardTextChanged( QString );
};
#endif /* APPLICATIONDATA_HPP_ */
