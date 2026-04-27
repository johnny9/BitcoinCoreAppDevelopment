// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtQuickTest/quicktest.h>

#include <QQmlContext>
#include <QQmlEngine>
#include <QStringList>

// Minimal mock of OptionsQmlModel for QML unit tests.
// Exposes only the properties used by display-settings QML components.
class MockOptionsModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int displayUnit READ displayUnit WRITE setDisplayUnit NOTIFY displayUnitChanged)
    Q_PROPERTY(QString displayUnitLabel READ displayUnitLabel NOTIFY displayUnitChanged)
    Q_PROPERTY(QString languageSummary READ languageSummary NOTIFY languageChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT)

public:
    int displayUnit() const { return m_displayUnit; }
    void setDisplayUnit(int u) {
        if (u != m_displayUnit) { m_displayUnit = u; Q_EMIT displayUnitChanged(u); }
    }
    QString displayUnitLabel() const { return m_displayUnit == 1 ? "sat" : "BTC"; }
    Q_INVOKABLE QString displayUnitLabelForAmount(qint64 satoshi) const {
        if (m_displayUnit != 1) return QString("₿");
        return (qAbs(satoshi) == 1) ? QString("sat") : QString("sats");
    }
    QString language() const { return m_language; }
    void setLanguage(const QString& l) {
        if (l != m_language) { m_language = l; Q_EMIT languageChanged(); }
    }
    QString languageSummary() const { return m_language.isEmpty() ? "System default" : m_language; }
    QStringList availableLanguages() const { return {"", "de", "es", "fr"}; }
    Q_INVOKABLE QString languageLabel(const QString& tag) const {
        if (tag.isEmpty()) return "System default";
        if (tag == "de") return "Deutsch \u2014 German";
        if (tag == "es") return "Espa\u00f1ol \u2014 Spanish";
        if (tag == "fr") return "Fran\u00e7ais \u2014 French";
        return tag;
    }

Q_SIGNALS:
    void displayUnitChanged(int unit);
    void languageChanged();

private:
    int m_displayUnit{0};
    QString m_language;
};

class QmlTestsSetup : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        engine->addImportPath(QStringLiteral(BITCOINQML_QML_TEST_MOCKS_DIR));
        engine->addImportPath(QStringLiteral(BITCOINQML_QML_SOURCE_DIR));
        // Provide a mock optionsModel so display-settings QML tests can run.
        static MockOptionsModel mockOptions;
        engine->rootContext()->setContextProperty("optionsModel", &mockOptions);
    }
};

QUICK_TEST_MAIN_WITH_SETUP(bitcoinqml_qmltests, QmlTestsSetup)

#include "qml_tests_main.moc"
