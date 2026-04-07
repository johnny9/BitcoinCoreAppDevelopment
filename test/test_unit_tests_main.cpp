// Copyright (c) 2026 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QGuiApplication>

#include <util/translation.h>

const TranslateFn G_TRANSLATION_FUN{nullptr};

int RunBitcoinAmountTests(int argc, char* argv[]);
int RunPeerListModelTests(int argc, char* argv[]);
int RunPeerStatsUtilTests(int argc, char* argv[]);
int RunQmlBitcoinUnitsTests(int argc, char* argv[]);
int RunImageProviderTests(int argc, char* argv[]);
int RunNetworkStyleTests(int argc, char* argv[]);
int RunQmlInitExecutorApiTests(int argc, char* argv[]);
int RunOptionsModelTests(int argc, char* argv[]);
int RunWalletQmlControllerTests(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    int status = 0;
    status |= RunBitcoinAmountTests(argc, argv);
    status |= RunPeerListModelTests(argc, argv);
    status |= RunPeerStatsUtilTests(argc, argv);
    status |= RunQmlBitcoinUnitsTests(argc, argv);
    status |= RunImageProviderTests(argc, argv);
    status |= RunNetworkStyleTests(argc, argv);
    status |= RunQmlInitExecutorApiTests(argc, argv);
    status |= RunOptionsModelTests(argc, argv);
    status |= RunWalletQmlControllerTests(argc, argv);

    return status;
}
