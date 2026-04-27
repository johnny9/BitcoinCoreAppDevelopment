// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtTest/QtTest>

#include <QSet>

#include <test/mocks/mocknode.h>
#include <qml/models/banlistmodel.h>
#include <util/translation.h>

#include <netbase.h>

#include <map>
#include <stdexcept>
#include <utility>

namespace {
CSubNet ParseSubnet(const std::string& subnet)
{
    CSubNet parsed = LookupSubNet(subnet);
    if (!parsed.IsValid()) {
        throw std::runtime_error("failed to parse subnet test fixture");
    }
    return parsed;
}

CBanEntry MakeBanEntry(int64_t ban_until)
{
    CBanEntry entry;
    entry.nBanUntil = ban_until;
    return entry;
}
} // namespace

class BanListModelTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void refreshPopulatesRolesAndRows();
    void unbanAtTargetsSelectedSubnet();
    void unbanAtIgnoresInvalidRows();
};

void BanListModelTests::refreshPopulatesRolesAndRows()
{
    using ::testing::_;
    using ::testing::DoAll;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::SetArgReferee;

    banmap_t banned;
    const CSubNet subnet_a = ParseSubnet("10.0.0.0/8");
    const CSubNet subnet_b = ParseSubnet("127.0.0.1/32");
    banned.emplace(subnet_a, MakeBanEntry(1'900'000'000));
    banned.emplace(subnet_b, MakeBanEntry(2'000'000'000));

    NiceMock<MockNode> node;
    EXPECT_CALL(node, getBanned(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(banned), Return(true)));

    BanListModel model{node, nullptr};
    QSignalSpy count_spy(&model, &BanListModel::countChanged);

    model.refresh();

    QCOMPARE(model.count(), 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);
    QCOMPARE(count_spy.count(), 1);

    const auto roles = model.roleNames();
    QCOMPARE(roles.value(static_cast<int>(BanListModel::BanRoles::AddressRole)), QByteArray{"address"});
    QCOMPARE(roles.value(static_cast<int>(BanListModel::BanRoles::BanUntilRole)), QByteArray{"banUntil"});

    QSet<QString> addresses;
    for (int row = 0; row < model.rowCount(); ++row) {
        const QModelIndex index = model.index(row, 0);
        QVERIFY(index.isValid());

        const QString address = model.data(index, static_cast<int>(BanListModel::BanRoles::AddressRole)).toString();
        QVERIFY(!address.isEmpty());
        addresses.insert(address);

        const QString ban_until = model.data(index, static_cast<int>(BanListModel::BanRoles::BanUntilRole)).toString();
        QVERIFY(!ban_until.isEmpty());
    }

    QVERIFY(addresses.contains(QString::fromStdString(subnet_a.ToString())));
    QVERIFY(addresses.contains(QString::fromStdString(subnet_b.ToString())));
}

void BanListModelTests::unbanAtTargetsSelectedSubnet()
{
    using ::testing::_;
    using ::testing::DoAll;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::SetArgReferee;
    using ::testing::Truly;

    banmap_t banned;
    const CSubNet subnet = ParseSubnet("10.0.0.0/8");
    banned.emplace(subnet, MakeBanEntry(1'900'000'000));

    NiceMock<MockNode> node;
    EXPECT_CALL(node, getBanned(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(banned), Return(true)));

    BanListModel model{node, nullptr};
    model.refresh();

    EXPECT_CALL(node, unban(Truly([&](const CSubNet& value) {
        return value.ToString() == subnet.ToString();
    }))).Times(1).WillOnce(Return(true));
    model.unbanAt(0);
}

void BanListModelTests::unbanAtIgnoresInvalidRows()
{
    using ::testing::_;
    using ::testing::DoAll;
    using ::testing::NiceMock;
    using ::testing::Return;
    using ::testing::SetArgReferee;

    banmap_t banned;
    banned.emplace(ParseSubnet("10.0.0.0/8"), MakeBanEntry(1'900'000'000));

    NiceMock<MockNode> node;
    EXPECT_CALL(node, getBanned(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(banned), Return(true)));

    BanListModel model{node, nullptr};
    model.refresh();

    EXPECT_CALL(node, unban(_)).Times(0);
    model.unbanAt(-1);
    model.unbanAt(42);
}

int RunBanListModelTests(int argc, char* argv[])
{
    BanListModelTests tests;
    return QTest::qExec(&tests, argc, argv);
}

#ifndef BITCOINQML_NO_TEST_MAIN
QTEST_MAIN(BanListModelTests)
#endif
#include "test_banlistmodel.moc"
