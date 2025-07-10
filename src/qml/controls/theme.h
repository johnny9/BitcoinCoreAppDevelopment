#ifndef BITCOIN_QML_CONTROLS_THEME_H
#define BITCOIN_QML_CONTROLS_THEME_H

#include <QObject>
#include <QColor>
#include <QUrl>
#include <QSettings>

// Helper macro to declare simple constant QColor properties
#define DECLARE_COLOR_PROPERTY(name) \
    Q_PROPERTY(QColor name READ name CONSTANT) \
public: \
    QColor name() const { return m_##name; } \
private: \
    QColor m_##name;

// Helper macro to declare simple constant QUrl properties
#define DECLARE_URL_PROPERTY(name) \
    Q_PROPERTY(QUrl name READ name CONSTANT) \
public: \
    QUrl name() const { return m_##name; } \
private: \
    QUrl m_##name;


class ColorSet : public QObject
{
    Q_OBJECT
    // clang-format off
    DECLARE_COLOR_PROPERTY(white)
    DECLARE_COLOR_PROPERTY(background)
    DECLARE_COLOR_PROPERTY(orange)
    DECLARE_COLOR_PROPERTY(orangeLight1)
    DECLARE_COLOR_PROPERTY(orangeLight2)
    DECLARE_COLOR_PROPERTY(red)
    DECLARE_COLOR_PROPERTY(green)
    DECLARE_COLOR_PROPERTY(blue)
    DECLARE_COLOR_PROPERTY(amber)
    DECLARE_COLOR_PROPERTY(purple)
    DECLARE_COLOR_PROPERTY(transparent)
    DECLARE_COLOR_PROPERTY(neutral0)
    DECLARE_COLOR_PROPERTY(neutral1)
    DECLARE_COLOR_PROPERTY(neutral2)
    DECLARE_COLOR_PROPERTY(neutral3)
    DECLARE_COLOR_PROPERTY(neutral4)
    DECLARE_COLOR_PROPERTY(neutral5)
    DECLARE_COLOR_PROPERTY(neutral6)
    DECLARE_COLOR_PROPERTY(neutral7)
    DECLARE_COLOR_PROPERTY(neutral8)
    DECLARE_COLOR_PROPERTY(neutral9)
    // clang-format on

    Q_PROPERTY(QVariantList confirmationColors READ confirmationColors CONSTANT)
public:
    explicit ColorSet(QObject* parent = nullptr);
    void setValues(const QList<QPair<QString, QColor>>& colors, const QVariantList& confirmation);

    QVariantList confirmationColors() const { return m_confirmation_colors; }

private:
    QVariantList m_confirmation_colors;
};

class ImageSet : public QObject
{
    Q_OBJECT
    // clang-format off
    DECLARE_URL_PROPERTY(blocktime)
    DECLARE_URL_PROPERTY(network)
    DECLARE_URL_PROPERTY(storage)
    DECLARE_URL_PROPERTY(pending)
    DECLARE_URL_PROPERTY(tooltipArrow)
    // clang-format on
public:
    explicit ImageSet(QObject* parent = nullptr);
    void setValues(const QList<QPair<QString, QUrl>>& urls);
};

class Theme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool dark READ dark WRITE setDark NOTIFY darkChanged)
    Q_PROPERTY(qreal blockclocksize READ blockclocksize WRITE setBlockclocksize NOTIFY blockclocksizeChanged)
    Q_PROPERTY(ColorSet* color READ color NOTIFY darkChanged)
    Q_PROPERTY(ImageSet* image READ image NOTIFY darkChanged)
public:
    static Theme& instance();

    static Theme* instancePtr() { return &instance(); }

    bool dark() const { return m_dark; }
    void setDark(bool value);

    qreal blockclocksize() const { return m_blockclocksize; }
    void setBlockclocksize(qreal value);

    Q_INVOKABLE void toggleDark();

    ColorSet* color();
    ImageSet* image();

Q_SIGNALS:
    void darkChanged();
    void blockclocksizeChanged();

private:
    explicit Theme(QObject* parent = nullptr);
    void loadSettings();
    void saveSettings();

    bool m_dark{true};
    qreal m_blockclocksize{5.0 / 12.0};

    ColorSet m_darkColorSet;
    ColorSet m_lightColorSet;

    ImageSet m_darkImageSet;
    ImageSet m_lightImageSet;
};

#endif // BITCOIN_QML_CONTROLS_THEME_H
