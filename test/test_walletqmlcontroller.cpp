// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <common/settings.h>
#include <common/types.h>
#include <interfaces/handler.h>
#include <interfaces/wallet.h>
#include <outputtype.h>
#include <qml/walletqmlcontroller.h>
#include <scheduler.h>
#include <test/mocks/mocknode.h>
#include <test/mocks/mockwallet.h>
#include <util/translation.h>
#include <wallet/walletutil.h>

#include <gmock/gmock.h>

#ifndef BITCOINQML_NO_TEST_MAIN
const TranslateFn G_TRANSLATION_FUN{nullptr};
#endif

namespace {
constexpr auto NOT_INITIALIZED_ERROR{"Wallets are still loading. Try again in a moment."};

std::unique_ptr<interfaces::Handler> MakeNoopHandler()
{
    return interfaces::MakeCleanupHandler([] {});
}

class FakeExternalSigner : public interfaces::ExternalSigner
{
public:
    explicit FakeExternalSigner(std::string name) : m_name(std::move(name)) {}
    std::string getName() override { return m_name; }

private:
    std::string m_name;
};

std::vector<std::unique_ptr<interfaces::ExternalSigner>> MakeSigners(std::initializer_list<const char*> names)
{
    std::vector<std::unique_ptr<interfaces::ExternalSigner>> signers;
    signers.reserve(names.size());
    for (const char* name : names) {
        signers.emplace_back(std::make_unique<FakeExternalSigner>(name));
    }
    return signers;
}

class FakeWalletLoader : public interfaces::WalletLoader
{
public:
    int create_wallet_calls{0};
    int migrate_wallet_calls{0};
    int handle_load_wallet_calls{0};
    int get_wallets_calls{0};
    int list_wallet_dir_calls{0};

    std::function<util::Result<std::unique_ptr<interfaces::Wallet>>(const std::string&, const SecureString&, uint64_t, std::vector<bilingual_str>&)>
        create_wallet_fn = [](const std::string&, const SecureString&, uint64_t, std::vector<bilingual_str>&) {
            return util::Error{Untranslated("Unexpected createWallet call")};
        };
    std::function<util::Result<interfaces::WalletMigrationResult>(const std::string&, const SecureString&)>
        migrate_wallet_fn = [](const std::string&, const SecureString&) {
            return util::Error{Untranslated("Unexpected migrateWallet call")};
        };
    std::function<std::vector<std::unique_ptr<interfaces::Wallet>>()> get_wallets_fn = [] {
        return std::vector<std::unique_ptr<interfaces::Wallet>>{};
    };
    std::vector<std::pair<std::string, std::string>> wallet_dir_entries;

    void registerRpcs() override {}
    bool verify() override { return true; }
    bool load() override { return true; }
    void start(CScheduler&) override {}
    void stop() override {}
    void setMockTime(int64_t) override {}
    void schedulerMockForward(std::chrono::seconds) override {}
    util::Result<std::unique_ptr<interfaces::Wallet>> createWallet(
        const std::string& name,
        const SecureString& passphrase,
        uint64_t wallet_creation_flags,
        std::vector<bilingual_str>& warnings) override
    {
        ++create_wallet_calls;
        return create_wallet_fn(name, passphrase, wallet_creation_flags, warnings);
    }
    util::Result<std::unique_ptr<interfaces::Wallet>> loadWallet(const std::string&, std::vector<bilingual_str>&) override
    {
        return util::Error{Untranslated("Unexpected loadWallet call")};
    }
    std::string getWalletDir() override { return {}; }
    util::Result<std::unique_ptr<interfaces::Wallet>> restoreWallet(const fs::path&, const std::string&, std::vector<bilingual_str>&, bool) override
    {
        return util::Error{Untranslated("Unexpected restoreWallet call")};
    }
    util::Result<interfaces::WalletMigrationResult> migrateWallet(const std::string& name, const SecureString& passphrase) override
    {
        ++migrate_wallet_calls;
        return migrate_wallet_fn(name, passphrase);
    }
    bool isEncrypted(const std::string&) override { return false; }
    std::vector<std::pair<std::string, std::string>> listWalletDir() override
    {
        ++list_wallet_dir_calls;
        return wallet_dir_entries;
    }
    std::vector<std::unique_ptr<interfaces::Wallet>> getWallets() override
    {
        ++get_wallets_calls;
        return get_wallets_fn();
    }
    std::unique_ptr<interfaces::Handler> handleLoadWallet(LoadWalletFn) override
    {
        ++handle_load_wallet_calls;
        return MakeNoopHandler();
    }
};

class FakeWallet : public StubWallet
{
public:
    struct State {
        int remove_calls{0};
    };

    explicit FakeWallet(std::string wallet_name, State* state)
        : m_wallet_name(std::move(wallet_name)), m_state(state) {}

    std::string getWalletName() override { return m_wallet_name; }
    void remove() override
    {
        if (m_state) {
            ++m_state->remove_calls;
        }
    }

private:
    std::string m_wallet_name;
    State* m_state;
};

void ExpectControllerInitialization(MockNode& node, FakeWalletLoader& loader)
{
    using ::testing::_;
    using ::testing::AtLeast;
    using ::testing::Invoke;
    using ::testing::Return;
    using ::testing::ReturnRef;

    ON_CALL(node, walletLoader()).WillByDefault(ReturnRef(loader));
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));

    EXPECT_CALL(node, walletLoader()).Times(AtLeast(3)).WillRepeatedly(ReturnRef(loader));
    EXPECT_CALL(node, getPersistentSetting(_)).Times(AtLeast(1)).WillRepeatedly(Return(common::SettingsValue{}));
    EXPECT_CALL(node, forceSetting(_, _)).Times(AtLeast(1));
    EXPECT_CALL(node, listExternalSigners()).Times(AtLeast(1)).WillRepeatedly(Invoke([] {
        return MakeSigners({});
    }));
}
} // namespace

class WalletQmlControllerTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void externalSignerCreationRequiresConfiguredPath();
    void externalSignerCreationRequiresExactlyOneSigner();
    void externalSignerSuggestionUsesSignerName();
    void createWalletBeforeInitializationReturnsFalseAndSetsError();
    void importWalletBeforeInitializationSetsLoadError();
    void migrateWalletBeforeInitializationSetsMigrationError();
    void selectWalletBeforeInitializationSetsLoadError();
    void initializedControllerPropagatesCreateErrors();
    void initializedControllerForwardsMigrationPassphrase();
    void initializedControllerClosesSelectedWalletAndSelectsRemainingLoadedWallet();
    void initializedControllerClosesNonSelectedWalletWithoutChangingSelection();
    void initializedControllerEmitsOpenWalletsChanged();
    void initializedControllerUnloadWalletsClearsSelectionAndOpenWallets();
};

void WalletQmlControllerTests::externalSignerCreationRequiresConfiguredPath()
{
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    EXPECT_CALL(node, listExternalSigners())
        .WillOnce(Invoke([] { return MakeSigners({"Ledger Nano X"}); }));

    WalletQmlController controller(node);
    controller.refreshExternalSignerStatus();

    QVERIFY(!controller.canCreateExternalSignerWallet());
    QCOMPARE(controller.externalSignerName(), QString("Ledger Nano X"));
    QCOMPARE(controller.suggestedExternalSignerWalletName(), QString("Ledger_Nano_X"));
}

void WalletQmlControllerTests::externalSignerCreationRequiresExactlyOneSigner()
{
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    ON_CALL(node, getPersistentSetting(std::string{"signer"}))
        .WillByDefault(Return(common::SettingsValue{std::string{"/usr/bin/hwi"}}));
    EXPECT_CALL(node, listExternalSigners())
        .WillOnce(Invoke([] { return MakeSigners({"Signer A", "Signer B"}); }));

    WalletQmlController controller(node);
    controller.refreshExternalSignerStatus();

    QVERIFY(!controller.canCreateExternalSignerWallet());
    QVERIFY(controller.externalSignerName().isEmpty());
    QCOMPARE(controller.externalSignerError(), QString("More than one external signer was found. Connect only one device."));
}

void WalletQmlControllerTests::externalSignerSuggestionUsesSignerName()
{
    using ::testing::_;
    using ::testing::Invoke;
    using ::testing::NiceMock;
    using ::testing::Return;

    NiceMock<MockNode> node;
    ON_CALL(node, getPersistentSetting(_)).WillByDefault(Return(common::SettingsValue{}));
    ON_CALL(node, getPersistentSetting(std::string{"signer"}))
        .WillByDefault(Return(common::SettingsValue{std::string{"/usr/bin/hwi"}}));
    EXPECT_CALL(node, listExternalSigners())
        .WillOnce(Invoke([] { return MakeSigners({"Coldcard Mk4"}); }));

    WalletQmlController controller(node);
    controller.refreshExternalSignerStatus();

    QVERIFY(controller.canCreateExternalSignerWallet());
    QCOMPARE(controller.externalSignerName(), QString("Coldcard Mk4"));
    QCOMPARE(controller.suggestedExternalSignerWalletName(), QString("Coldcard_Mk4"));
}

void WalletQmlControllerTests::createWalletBeforeInitializationReturnsFalseAndSetsError()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    WalletQmlController controller(node);

    QVERIFY(!controller.createSingleSigWallet("test_wallet", "secret"));
    QCOMPARE(controller.walletCreateError(), QString{NOT_INITIALIZED_ERROR});
    QVERIFY(!controller.isWalletLoaded());
}

void WalletQmlControllerTests::importWalletBeforeInitializationSetsLoadError()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    WalletQmlController controller(node);

    controller.importWallet("/tmp/test_wallet.dat");
    QCOMPARE(controller.walletLoadError(), QString{NOT_INITIALIZED_ERROR});
}

void WalletQmlControllerTests::migrateWalletBeforeInitializationSetsMigrationError()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    WalletQmlController controller(node);

    controller.migrateWallet("legacy_wallet", "secret");
    QCOMPARE(controller.walletMigrationError(), QString{NOT_INITIALIZED_ERROR});
}

void WalletQmlControllerTests::selectWalletBeforeInitializationSetsLoadError()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    WalletQmlController controller(node);

    controller.setSelectedWallet("test_wallet");
    QCOMPARE(controller.walletLoadError(), QString{NOT_INITIALIZED_ERROR});
}

void WalletQmlControllerTests::initializedControllerPropagatesCreateErrors()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    FakeWalletLoader loader;
    ExpectControllerInitialization(node, loader);

    WalletQmlController controller(node);
    controller.initialize();

    QVERIFY(controller.initialized());
    QVERIFY(controller.noWalletsFound());

    bool saw_expected_passphrase{false};
    bool saw_expected_flags{false};
    loader.create_wallet_fn = [&](const std::string&,
                                  const SecureString& passphrase,
                                  uint64_t wallet_creation_flags,
                                  std::vector<bilingual_str>&) {
        saw_expected_passphrase = (passphrase == SecureString{"secret"});
        saw_expected_flags = (wallet_creation_flags == wallet::WALLET_FLAG_DESCRIPTORS);
        return util::Result<std::unique_ptr<interfaces::Wallet>>{
            util::Error{Untranslated("Wallet creation failed.")}};
    };

    QVERIFY(!controller.createSingleSigWallet("test_wallet", "secret"));
    QVERIFY(saw_expected_passphrase);
    QVERIFY(saw_expected_flags);
    QCOMPARE(loader.create_wallet_calls, 1);
    QCOMPARE(controller.walletCreateError(), QString{"Wallet creation failed."});
    QVERIFY(!controller.isWalletLoaded());
}

void WalletQmlControllerTests::initializedControllerForwardsMigrationPassphrase()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    FakeWalletLoader loader;
    loader.wallet_dir_entries = {{"legacy_wallet", "bdb"}};
    ExpectControllerInitialization(node, loader);

    WalletQmlController controller(node);
    controller.initialize();

    QSignalSpy failed_spy(&controller, &WalletQmlController::walletMigrationFailed);

    bool saw_expected_name{false};
    bool saw_expected_passphrase{false};
    loader.migrate_wallet_fn = [&](const std::string& name, const SecureString& passphrase) {
        saw_expected_name = (name == "legacy_wallet");
        saw_expected_passphrase = (passphrase == SecureString{"secret"});
        return util::Result<interfaces::WalletMigrationResult>{
            util::Error{Untranslated("Migration failed.")}};
    };

    controller.migrateWallet("legacy_wallet", "secret");
    QVERIFY(failed_spy.wait(5000));
    QVERIFY(saw_expected_name);
    QVERIFY(saw_expected_passphrase);
    QCOMPARE(loader.migrate_wallet_calls, 1);
    QCOMPARE(controller.walletMigrationError(), QString{"Migration failed."});
    QVERIFY(!controller.walletMigrationInProgress());
}

void WalletQmlControllerTests::initializedControllerClosesSelectedWalletAndSelectsRemainingLoadedWallet()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    FakeWalletLoader loader;
    FakeWallet::State alpha_state;
    FakeWallet::State beta_state;
    loader.get_wallets_fn = [&]() {
        std::vector<std::unique_ptr<interfaces::Wallet>> wallets;
        wallets.emplace_back(std::make_unique<FakeWallet>("alpha_wallet", &alpha_state));
        wallets.emplace_back(std::make_unique<FakeWallet>("beta_wallet", &beta_state));
        return wallets;
    };
    ExpectControllerInitialization(node, loader);

    WalletQmlController controller(node);
    controller.initialize();

    QCOMPARE(controller.selectedWallet()->name(), QString{"alpha_wallet"});
    QVERIFY(controller.isWalletLoaded());
    QVERIFY(controller.isWalletOpen("alpha_wallet"));
    QVERIFY(controller.isWalletOpen("beta_wallet"));

    QSignalSpy selected_spy(&controller, &WalletQmlController::selectedWalletChanged);
    QSignalSpy open_wallets_spy(&controller, &WalletQmlController::openWalletsChanged);

    controller.closeWallet("alpha_wallet");

    QCOMPARE(alpha_state.remove_calls, 1);
    QCOMPARE(beta_state.remove_calls, 0);
    QCOMPARE(selected_spy.count(), 1);
    QCOMPARE(open_wallets_spy.count(), 1);
    QCOMPARE(open_wallets_spy.at(0).at(0).toStringList(), QStringList({"beta_wallet"}));
    QCOMPARE(controller.selectedWallet()->name(), QString{"beta_wallet"});
    QVERIFY(controller.isWalletLoaded());
    QVERIFY(!controller.isWalletOpen("alpha_wallet"));
    QVERIFY(controller.isWalletOpen("beta_wallet"));
}

void WalletQmlControllerTests::initializedControllerClosesNonSelectedWalletWithoutChangingSelection()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    FakeWalletLoader loader;
    FakeWallet::State alpha_state;
    FakeWallet::State beta_state;
    loader.get_wallets_fn = [&]() {
        std::vector<std::unique_ptr<interfaces::Wallet>> wallets;
        wallets.emplace_back(std::make_unique<FakeWallet>("alpha_wallet", &alpha_state));
        wallets.emplace_back(std::make_unique<FakeWallet>("beta_wallet", &beta_state));
        return wallets;
    };
    ExpectControllerInitialization(node, loader);

    WalletQmlController controller(node);
    controller.initialize();

    QSignalSpy selected_spy(&controller, &WalletQmlController::selectedWalletChanged);
    QSignalSpy open_wallets_spy(&controller, &WalletQmlController::openWalletsChanged);

    controller.closeWallet("beta_wallet");

    QCOMPARE(alpha_state.remove_calls, 0);
    QCOMPARE(beta_state.remove_calls, 1);
    QCOMPARE(selected_spy.count(), 0);
    QCOMPARE(open_wallets_spy.count(), 1);
    QCOMPARE(open_wallets_spy.at(0).at(0).toStringList(), QStringList({"alpha_wallet"}));
    QCOMPARE(controller.selectedWallet()->name(), QString{"alpha_wallet"});
    QVERIFY(controller.isWalletLoaded());
    QVERIFY(controller.isWalletOpen("alpha_wallet"));
    QVERIFY(!controller.isWalletOpen("beta_wallet"));
}

void WalletQmlControllerTests::initializedControllerEmitsOpenWalletsChanged()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    FakeWalletLoader loader;
    FakeWallet::State alpha_state;
    FakeWallet::State beta_state;
    loader.get_wallets_fn = [&]() {
        std::vector<std::unique_ptr<interfaces::Wallet>> wallets;
        wallets.emplace_back(std::make_unique<FakeWallet>("alpha_wallet", &alpha_state));
        wallets.emplace_back(std::make_unique<FakeWallet>("beta_wallet", &beta_state));
        return wallets;
    };
    ExpectControllerInitialization(node, loader);

    WalletQmlController controller(node);
    QSignalSpy open_wallets_spy(&controller, &WalletQmlController::openWalletsChanged);

    controller.initialize();

    QCOMPARE(open_wallets_spy.count(), 1);
    QCOMPARE(open_wallets_spy.at(0).at(0).toStringList(), QStringList({"alpha_wallet", "beta_wallet"}));
}

void WalletQmlControllerTests::initializedControllerUnloadWalletsClearsSelectionAndOpenWallets()
{
    using ::testing::StrictMock;

    StrictMock<MockNode> node;
    FakeWalletLoader loader;
    FakeWallet::State alpha_state;
    FakeWallet::State beta_state;
    loader.get_wallets_fn = [&]() {
        std::vector<std::unique_ptr<interfaces::Wallet>> wallets;
        wallets.emplace_back(std::make_unique<FakeWallet>("alpha_wallet", &alpha_state));
        wallets.emplace_back(std::make_unique<FakeWallet>("beta_wallet", &beta_state));
        return wallets;
    };
    ExpectControllerInitialization(node, loader);

    WalletQmlController controller(node);
    controller.initialize();

    QSignalSpy selected_spy(&controller, &WalletQmlController::selectedWalletChanged);
    QSignalSpy open_wallets_spy(&controller, &WalletQmlController::openWalletsChanged);

    controller.unloadWallets();

    QCOMPARE(selected_spy.count(), 1);
    QCOMPARE(open_wallets_spy.count(), 1);
    QCOMPARE(open_wallets_spy.at(0).at(0).toStringList(), QStringList{});
    QCOMPARE(controller.selectedWallet()->name(), QString{});
    QVERIFY(!controller.isWalletOpen("alpha_wallet"));
    QVERIFY(!controller.isWalletOpen("beta_wallet"));
    QCOMPARE(alpha_state.remove_calls, 0);
    QCOMPARE(beta_state.remove_calls, 0);
}

#ifdef BITCOINQML_NO_TEST_MAIN
#include <test/qt_test_registry.h>
BITCOINQML_REGISTER_QT_TEST(WalletQmlControllerTests)
#else
QTEST_MAIN(WalletQmlControllerTests)
#endif
#include "test_walletqmlcontroller.moc"
