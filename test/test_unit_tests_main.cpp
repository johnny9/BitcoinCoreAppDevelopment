// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>

#include <chainparams.h>
#include <test/qt_test_registry.h>
#include <util/translation.h>

const TranslateFn G_TRANSLATION_FUN{nullptr};

int RunBitcoinUriTests(int argc, char* argv[]);
int RunBlockClockDialTests(int argc, char* argv[]);
int RunDesktopTrayIconControllerTests(int argc, char* argv[]);
int RunDesktopWindowBehaviorModelTests(int argc, char* argv[]);
int RunNodeModelTests(int argc, char* argv[]);
int RunBanListModelTests(int argc, char* argv[]);
int RunDisplaySettingsTests(int argc, char* argv[]);
int RunWalletListModelTests(int argc, char* argv[]);
int RunWalletQmlModelTests(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    SelectParams(ChainType::REGTEST);

    int status = 0;
    status |= RunBitcoinUriTests(argc, argv);
    status |= RunBlockClockDialTests(argc, argv);
    status |= RunDesktopTrayIconControllerTests(argc, argv);
    status |= RunDesktopWindowBehaviorModelTests(argc, argv);
    status |= RunNodeModelTests(argc, argv);
    status |= RunBanListModelTests(argc, argv);
    status |= RunDisplaySettingsTests(argc, argv);
    status |= RunWalletListModelTests(argc, argv);
    status |= RunWalletQmlModelTests(argc, argv);

    for (const auto& test : qttestregistry::SortedEntries()) {
        status |= test.run(argc, argv);
    }

    return status;
}
