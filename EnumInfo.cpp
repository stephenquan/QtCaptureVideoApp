#include "EnumInfo.h"
#include <QDebug>

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

EnumInfo::EnumInfo(QObject* parent) :
    QObject(parent)
{
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

EnumInfo::EnumInfo(const QMetaEnum& metaEnum, QObject* parent) :
    QObject(parent),
    m_MetaEnum(metaEnum)
{
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

EnumInfo::EnumInfo(const QString& name, const QVariant& context, QObject* parent) :
    QObject(parent)
{
    setContext(context);
    setName(name);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

QString EnumInfo::stringify(int value) const
{
    return metaEnum().valueToKey(value);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

QVariant EnumInfo::parse(const QString& key) const
{
    return metaEnum().keyToValue(key.toUtf8().data());
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

QStringList EnumInfo::availableNames() const
{
    if (!m_Context.isValid() || m_Context.isNull())
    {
        return QStringList();
    }

    QObject* context = qvariant_cast<QObject*>(m_Context);
    if (!context)
    {
        return QStringList();
    }

    const QMetaObject* metaObject = context->metaObject();
    if (!metaObject)
    {
        return QStringList();
    }

    QStringList list;
    for (int index = 0; index < metaObject->enumeratorCount(); index++)
    {
        QMetaEnum metaEnum = metaObject->enumerator(index);
        list.append(metaEnum.name());
    }
    return list;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

void EnumInfo::setContext(const QVariant& context)
{
    m_Context = context;

    setMetaEnum(metaEnumFromContext(m_Context, m_Name));
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

void EnumInfo::setName(const QString& name)
{
    m_Name = name;

    setMetaEnum(metaEnumFromContext(m_Context, m_Name));
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

QMetaEnum EnumInfo::metaEnumFromContext(const QVariant& context, const QString& name)
{
    if (!context.isValid() || context.isNull())
    {
        return QMetaEnum();
    }

    QObject* _context = qvariant_cast<QObject*>(context);
    if (!_context)
    {
        return QMetaEnum();
    }

    const QMetaObject* metaObject = _context->metaObject();
    if (!metaObject)
    {
        return QMetaEnum();
    }

    int index = metaObject->indexOfEnumerator(name.toUtf8().data());
    return metaObject->enumerator(index);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------

void EnumInfo::setMetaEnum(const QMetaEnum& metaEnum)
{
    m_MetaEnum = metaEnum;

    emit enumInfoChanged();
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
