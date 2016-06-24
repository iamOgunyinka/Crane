/*
 * CraneDataModel.hpp
 *
 *  Created on: Jun 24, 2016
 *      Author: adonai
 */

#ifndef CRANEDATAMODEL_HPP_
#define CRANEDATAMODEL_HPP_

#include <QObject>
#include <bb/cascades/DataModel>

using namespace bb::cascades;
class CraneDataModel : DataModel
{
    Q_OBJECT
public:
    Q_INVOKABLE virtual bool hasChildren( QVariantList const & indexPath );
    Q_INVOKABLE virtual int childCount( QVariantList const & indexPath );
    Q_INVOKABLE virtual QString itemType( QVariantList const & indexPath );
    Q_INVOKABLE virtual QVariant data( QVariantList const & indexPath );
    Q_INVOKABLE virtual void removeItem( QVariantList const & indexPath );
    CraneDataModel();
    virtual ~CraneDataModel();
};

#endif /* CRANEDATAMODEL_HPP_ */
