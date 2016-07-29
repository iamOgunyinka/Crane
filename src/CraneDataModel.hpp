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
#include "DownloadInfo.hpp"

namespace CraneDM {

    using namespace bb::cascades;
    class CraneDataModel : public DataModel
    {
        Q_OBJECT

    public:
        Q_INVOKABLE virtual bool hasChildren( QVariantList const & indexPath );
        Q_INVOKABLE virtual int childCount( QVariantList const & indexPath );
        Q_INVOKABLE virtual QString itemType( QVariantList const & indexPath );
        Q_INVOKABLE virtual QVariant data( QVariantList const & indexPath );

        CraneDataModel( QObject *parent = NULL );
        virtual ~CraneDataModel();

        static QString  GetDownloadLogoForFileExtension( QString const & filename );
        static QMap<QString, QString> ExtensionLogoMap;
        static void InstallExtensions();
        static QString ConvertByte( qint64 );
        void changeView( Information::DownloadStatus view_type );
        static QVariantList countList( Information::DownloadStatus view_type );
    private:
        int i;
        Information::DownloadStatus view_type;

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
        void refreshView( QString url );
        void progressHandler( QString );
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
}
#endif /* CRANEDATAMODEL_HPP_ */
