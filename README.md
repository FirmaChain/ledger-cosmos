# Ledger FirmaChain app

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

This repository contains the FirmaChain app for Ledger hardware wallets. It allows users to manage FirmaChain (FCT) accounts and sign FirmaChain transactions on Ledger devices.

The app is based on the [Cosmos Ledger app](https://github.com/cosmos/ledger-cosmos) by Zondax, adapted for the FirmaChain network.

## Supported devices

- Ledger Nano S
- Ledger Nano S Plus
- Ledger Nano X
- Ledger Stax
- Ledger Flex

## Features

- Display, sign and broadcast FirmaChain transactions
- Support for both JSON (legacy Amino) and Protobuf (SIGN_MODE_DIRECT) sign modes
- Derivation path: `m/44'/7777777'/0'/0/0` (coin type `7777777`)
- Bech32 address prefix: `firma`
- Base denomination: `ufct` (displayed as `FCT`)

## ATTENTION

- **Do not use in production until this app is officially published on Ledger Live.**
- **Do not use a Ledger device with funds for development purposes.**
- **Have a separate and marked device that is used ONLY for development and testing.**

## Companion wallet

The recommended companion wallet for this app is [Firma Station](https://github.com/FirmaChain/firma-station), which connects to the Ledger device over USB (WebHID).

## Development

### Preconditions

- Checkout submodules:

    ```sh
    git submodule update --init --recursive
    ```

- Install Docker CE (https://docs.docker.com/install/)
- We officially support Ubuntu. Install the following packages:

    ```sh
    sudo apt update && apt-get -y install build-essential git wget cmake \
      libssl-dev libgmp-dev autoconf libtool
    ```

- Install Python 3
- This project requires the Ledger Secure SDK (tracked as a git submodule)

### Build

Using the [Ledger VS Code extension](https://marketplace.visualstudio.com/items?itemName=LedgerHQ.ledger-dev-tools) is the easiest way to build and load the app:

1. Open this repository in VS Code
2. Open the Ledger extension from the sidebar
3. Select the target device (e.g., Nano S Plus)
4. Run **Update container**
5. Run **Build (full)**
6. Run **Load app on device** to install on a physical Ledger

Alternatively, build from the command line inside the Ledger dev container:

```sh
make
```

### Loading onto a device

Loading is device-specific. Use the target matching your device:

| Device         | Command        |
|----------------|----------------|
| Nano S         | `make loadS`   |
| Nano S Plus    | `make loadS2`  |
| Stax           | `make loadST`  |
| Flex           | `make loadFL`  |

> **Warning**: The existing app on the device will be deleted before uploading.

Nano X cannot be side-loaded; it can only receive apps signed by Ledger through Ledger Live.

## Running tests

- C/C++ unit tests:

    ```sh
    make cpp_test
    ```

- Zemu integration tests:

    ```sh
    make zemu_install
    make zemu_test
    ```

## APDU specifications

- [APDU Protocol](docs/APDUSPEC.md)
- [Transaction format](docs/TXSPEC.md)

## License

Apache 2.0 — see [LICENSE](LICENSE).
