#include "theme.h"

#include <QVariantList>
#include <QMetaProperty>


static QList<QPair<QString, QColor>> makeColorPairs(std::initializer_list<std::pair<const char*, const char*>> list)
{
    QList<QPair<QString, QColor>> out;
    for (const auto& p : list) {
        out.append({QString::fromLatin1(p.first), QColor(QString::fromLatin1(p.second))});
    }
    return out;
}

static QList<QPair<QString, QUrl>> makeUrlPairs(std::initializer_list<std::pair<const char*, const char*>> list)
{
    QList<QPair<QString, QUrl>> out;
    for (const auto& p : list) {
        out.append({QString::fromLatin1(p.first), QUrl(QString::fromLatin1(p.second))});
    }
    return out;
}


ColorSet::ColorSet(QObject* parent) : QObject(parent) {}

void ColorSet::setValues(const QList<QPair<QString, QColor>>& colors, const QVariantList& confirmation)
{
    for (const auto& p : colors) {
        const QByteArray key = p.first.toLatin1();

        int idx = metaObject()->indexOfProperty(key.constData());
        if (idx >= 0) {
            auto prop = metaObject()->property(idx);
            prop.write(this, p.second);
        }
    }
    m_confirmation_colors = confirmation;
}


ImageSet::ImageSet(QObject* parent) : QObject(parent) {}

void ImageSet::setValues(const QList<QPair<QString, QUrl>>& urls)
{
    for (const auto& p : urls) {
        const QByteArray key = p.first.toLatin1();
        int idx = metaObject()->indexOfProperty(key.constData());
        if (idx >= 0) {
            metaObject()->property(idx).write(this, p.second);
        }
    }
}


Theme& Theme::instance()
{
    static Theme s_instance;
    return s_instance;
}

Theme::Theme(QObject* parent)
    : QObject(parent)
    , m_darkColorSet(this)
    , m_lightColorSet(this)
    , m_darkImageSet(this)
    , m_lightImageSet(this)
{
    
    m_darkColorSet.setValues(makeColorPairs({
                                           {"white", "#FFFFFF"},
                                           {"background", "black"},
                                           {"orange", "#F89B2A"},
                                           {"orangeLight1", "#FFAD4A"},
                                           {"orangeLight2", "#FFBF72"},
                                           {"red", "#EC6363"},
                                           {"green", "#36B46B"},
                                           {"blue", "#3CA3DE"},
                                           {"amber", "#C9B500"},
                                           {"purple", "#C075DC"},
                                           {"transparent", "#00000000"},
                                           {"neutral0", "#000000"},
                                           {"neutral1", "#1A1A1A"},
                                           {"neutral2", "#2D2D2D"},
                                           {"neutral3", "#444444"},
                                           {"neutral4", "#5C5C5C"},
                                           {"neutral5", "#787878"},
                                           {"neutral6", "#949494"},
                                           {"neutral7", "#B0B0B0"},
                                           {"neutral8", "#CCCCCC"},
                                           {"neutral9", "#FFFFFF"},
                                       }),
                                       {QVariantList{QColor("#FF1C1C"), QColor("#ED6E46"), QColor("#EE8847"), QColor("#EFA148"), QColor("#F0BB49"), QColor("#F1D54A")}});

    m_lightColorSet.setValues(makeColorPairs({
                                           {"white", "#FFFFFF"},
                                           {"background", "white"},
                                           {"orange", "#F7931A"},
                                           {"orangeLight1", "#FFAD4A"},
                                           {"orangeLight2", "#FFBF72"},
                                           {"red", "#EB5757"},
                                           {"green", "#27AE60"},
                                           {"blue", "#2D9CDB"},
                                           {"amber", "#C9B500"},
                                           {"purple", "#BB6BD9"},
                                           {"transparent", "#00000000"},
                                           {"neutral0", "#FFFFFF"},
                                           {"neutral1", "#F8F8F8"},
                                           {"neutral2", "#F4F4F4"},
                                           {"neutral3", "#EDEDED"},
                                           {"neutral4", "#DEDEDE"},
                                           {"neutral5", "#BBBBBB"},
                                           {"neutral6", "#999999"},
                                           {"neutral7", "#777777"},
                                           {"neutral8", "#404040"},
                                           {"neutral9", "#000000"},
                                       }),
                                       {QVariantList{QColor("#FF1C1C"), QColor("#ED6E46"), QColor("#EE8847"), QColor("#EFA148"), QColor("#F0BB49"), QColor("#F1D54A")}});

    m_darkImageSet.setValues(makeUrlPairs({
                                         {"blocktime", "image://images/blocktime-dark"},
                                         {"network", "image://images/network-dark"},
                                         {"storage", "image://images/storage-dark"},
                                         {"pending", "image://images/pending-dark"},
                                         {"tooltipArrow", "qrc:/icons/tooltip-arrow-dark"},
                                     }));

    m_lightImageSet.setValues(makeUrlPairs({
                                         {"blocktime", "image://images/blocktime-light"},
                                         {"network", "image://images/network-light"},
                                         {"storage", "image://images/storage-light"},
                                         {"pending", "image://images/pending-light"},
                                         {"tooltipArrow", "qrc:/icons/tooltip-arrow-light"},
                                     }));

    loadSettings();
}

void Theme::loadSettings()
{
    QSettings s;
    m_dark = s.value("theme/dark", true).toBool();
    m_blockclocksize = s.value("theme/blockclocksize", 5.0 / 12.0).toReal();
}

void Theme::saveSettings()
{
    QSettings s;
    s.setValue("theme/dark", m_dark);
    s.setValue("theme/blockclocksize", m_blockclocksize);
}

void Theme::setDark(bool value)
{
    if (m_dark == value) return;
    m_dark = value;
    Q_EMIT darkChanged();
    saveSettings();
}

void Theme::setBlockclocksize(qreal value)
{
    if (qFuzzyCompare(m_blockclocksize, value)) return;
    m_blockclocksize = value;
    Q_EMIT blockclocksizeChanged();
    saveSettings();
}

void Theme::toggleDark()
{
    setDark(!m_dark);
}

ColorSet* Theme::color()
{
    return m_dark ? &m_darkColorSet : &m_lightColorSet;
}

ImageSet* Theme::image()
{
    return m_dark ? &m_darkImageSet : &m_lightImageSet;
}
