/*
 * CraneDataModel.hpp
 *
 *  Created on: Jun 24, 2016
 *      Author: Joshua
 */

#ifndef CRANEDATAMODEL_HPP_
#define CRANEDATAMODEL_HPP_

#include <QObject>
#include <bb/cascades/DataModel>

using namespace bb::cascades;

enum DownloadViewType {
    ActiveDownloads = 0,
    CompletedDownloads,
    AllDownloads
};

class CraneDataModel : public DataModel
{
    Q_OBJECT

public:
    Q_INVOKABLE virtual bool hasChildren( QVariantList const & indexPath );
    Q_INVOKABLE virtual int childCount( QVariantList const & indexPath );
    Q_INVOKABLE virtual QString itemType( QVariantList const & indexPath );
    Q_INVOKABLE virtual QVariant data( QVariantList const & indexPath );
    CraneDataModel( QObject *parent = NULL );

    static QString  GetDownloadLogoForFileExtension( QString const & filename );
    static QMap<QString, QString> ExtensionLogoMap;
    static void InstallExtensions();
    static QString ConvertByte( qint64 );
    virtual ~CraneDataModel();
    void changeView( DownloadViewType view_type );
private:
    int i;
    DownloadViewType view_type;
};

class CraneFilteredDataModel : public DataModel
{
    Q_OBJECT
public:
    Q_INVOKABLE virtual bool hasChildren( QVariantList const & indexPath );
    Q_INVOKABLE virtual int childCount( QVariantList const & indexPath );
    Q_INVOKABLE virtual QString itemType( QVariantList const & indexPath );
    Q_INVOKABLE virtual QVariant data( QVariantList const & indexPath );
    Q_INVOKABLE virtual void removeItem( QVariantList const & indexPath );
    Q_INVOKABLE void changeView( int view );

    CraneFilteredDataModel( bb::cascades::DataModel *data_model, QObject *parent = NULL );
    virtual ~CraneFilteredDataModel();
public slots:
    void refreshView();
    void progressHandler( QString, QDateTime );
private:
    bb::cascades::DataModel *m_pDataModel;
};

class MyIndexMapper : public bb::cascades::DataModel::IndexMapper
{
public:
    MyIndexMapper( int index, int count, bool deleted );
    bool newIndexPath( QVariantList *pNewIndexPath, int *pReplacementIndex, QVariantList const & oldIndexPath ) const;
private:
    int m_index;
    int m_count;
    bool m_deleted;
};

#endif /* CRANEDATAMODEL_HPP_ */
