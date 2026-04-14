// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <test/mocks/mocknode.h>
#include <qml/models/options_model.h>
#include <net_processing.h>
#include <common/args.h>
#include <util/translation.h>

#ifndef BITCOINQML_NO_TEST_MAIN
const TranslateFn G_TRANSLATION_FUN{nullptr};
#endif

class OptionsModelTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void proxyDisabledRemovesKey();
    void torDisabledRemovesKey();
    void proxyEnabledWritesAddress();
    void onboardWritesProxy();
    void onboardWritesOnboardedFlag();
    void proxyDirtySetWhenOnboarded();
    void proxyDirtyNotSetDuringOnboarding();
    void proxyDirtyResetWhenReverted();
    void proxyDirtyNotSetAfterOnboard();
};

// Convenience: set up a NiceMock whose getPersistentSetting returns null for
// all keys by default, but returns a given address for the specified key.
static common::SettingsValue MakeAddress(const std::string& addr)
{
    return common::SettingsValue{addr};
}

void OptionsModelTests::proxyDisabledRemovesKey()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    // Simulate a previously-saved proxy address so that m_proxy_enabled=true on construction.
    ON_CALL(node, getPersistentSetting(std::string{"proxy"}))
        .WillByDefault(Return(MakeAddress("127.0.0.1:9050")));

    OptionsQmlModel model(node, /*is_onboarded=*/true);
    QVERIFY(model.proxyEnabled());

    // When proxy is disabled, updateRwSetting must be called with a null (not
    // empty-string) SettingsValue so that the key is erased from settings.json.
    EXPECT_CALL(node, updateRwSetting(std::string{"proxy"},
        Truly([](const common::SettingsValue& v) { return v.isNull(); })));

    model.setProxyEnabled(false);
    QVERIFY(!model.proxyEnabled());
}

void OptionsModelTests::torDisabledRemovesKey()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    ON_CALL(node, getPersistentSetting(std::string{"onion"}))
        .WillByDefault(Return(MakeAddress("127.0.0.1:9150")));

    OptionsQmlModel model(node, /*is_onboarded=*/true);
    QVERIFY(model.torEnabled());

    EXPECT_CALL(node, updateRwSetting(std::string{"onion"},
        Truly([](const common::SettingsValue& v) { return v.isNull(); })));

    model.setTorEnabled(false);
    QVERIFY(!model.torEnabled());
}

void OptionsModelTests::proxyEnabledWritesAddress()
{
    using ::testing::_;
    using ::testing::Eq;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));

    // Construct with no saved proxy — m_proxy_enabled=false, m_proxy_address="".
    OptionsQmlModel model(node, /*is_onboarded=*/true);
    QVERIFY(!model.proxyEnabled());

    // Pre-load an address into the model (as QML does before toggling the switch).
    model.setProxyAddress("10.0.0.1:9050");

    // Enabling proxy must write the address string to settings.
    EXPECT_CALL(node, updateRwSetting(std::string{"proxy"},
        Truly([](const common::SettingsValue& v) {
            return v.isStr() && v.get_str() == "10.0.0.1:9050";
        })));

    model.setProxyEnabled(true);
    QVERIFY(model.proxyEnabled());
}

void OptionsModelTests::onboardWritesProxy()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    ON_CALL(node, resetSettings()).WillByDefault(Return());

    // Construct as not-yet-onboarded.
    OptionsQmlModel model(node, /*is_onboarded=*/false);
    model.setProxyEnabled(true);
    model.setProxyAddress("10.0.0.1:9050");

    // onboard() must write the proxy address to disk.
    EXPECT_CALL(node, updateRwSetting(std::string{"proxy"},
        Truly([](const common::SettingsValue& v) {
            return v.isStr() && v.get_str() == "10.0.0.1:9050";
        })));
    EXPECT_CALL(node, updateRwSetting(std::string{"qml_onboarded"},
        Truly([](const common::SettingsValue& v) {
            return SettingToBool(v, false);
        })));

    model.onboard();
}

void OptionsModelTests::onboardWritesOnboardedFlag()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::Truly;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    ON_CALL(node, resetSettings()).WillByDefault(Return());

    OptionsQmlModel model(node, /*is_onboarded=*/false);

    EXPECT_CALL(node, updateRwSetting(std::string{"qml_onboarded"},
        Truly([](const common::SettingsValue& v) {
            return SettingToBool(v, false);
        })));

    model.onboard();
}

void OptionsModelTests::proxyDirtySetWhenOnboarded()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));

    OptionsQmlModel model(node, /*is_onboarded=*/true);
    QVERIFY(!model.proxySettingsDirty());

    model.setProxyEnabled(true);
    QVERIFY(model.proxySettingsDirty());
}

void OptionsModelTests::proxyDirtyNotSetDuringOnboarding()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));

    // During onboarding the node has not started yet, so no restart is needed.
    OptionsQmlModel model(node, /*is_onboarded=*/false);
    model.setProxyEnabled(true);
    model.setProxyAddress("127.0.0.1:9050");
    QVERIFY(!model.proxySettingsDirty());
}

void OptionsModelTests::proxyDirtyResetWhenReverted()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));

    // Start onboarded with proxy disabled (no saved proxy).
    OptionsQmlModel model(node, /*is_onboarded=*/true);
    QVERIFY(!model.proxySettingsDirty());

    // Simulate what ProxySettings.qml does: set address before enabling.
    model.setProxyAddress("127.0.0.1:9050");
    // Address changed but proxy is still disabled — should NOT be dirty since
    // the address is irrelevant when proxy is off.
    QVERIFY(!model.proxySettingsDirty());

    // Enable proxy — now dirty (enabled differs from initial disabled).
    model.setProxyEnabled(true);
    QVERIFY(model.proxySettingsDirty());

    // Revert enable state — dirty should clear even though address is populated,
    // because the address is ignored when proxy is disabled.
    model.setProxyEnabled(false);
    QVERIFY(!model.proxySettingsDirty());
}

void OptionsModelTests::proxyDirtyNotSetAfterOnboard()
{
    using ::testing::_;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    ON_CALL(node, resetSettings()).WillByDefault(Return());

    // Configure proxy during onboarding.
    OptionsQmlModel model(node, /*is_onboarded=*/false);
    model.setProxyEnabled(true);
    model.setProxyAddress("127.0.0.1:9050");
    QVERIFY(!model.proxySettingsDirty());

    // After onboard() the node starts with those settings applied —
    // no restart is needed, so dirty must be false.
    model.onboard();
    QVERIFY(!model.proxySettingsDirty());
}

int RunOptionsModelTests(int argc, char* argv[])
{
    OptionsModelTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#ifndef BITCOINQML_NO_TEST_MAIN
QTEST_MAIN(OptionsModelTests)
#endif
#include "test_options_model.moc"
