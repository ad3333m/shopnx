# ShopNX

An NSP / NSZ / XCI / XCZ installer for Nintendo Switch with a cleaner look, built on
[CyberFoil](https://github.com/luketanti/CyberFoil) (GPL-3.0), which is built on Awoo Installer.

## Shops
Open **Shops** and add a shop profile (URL, optional username and password). One app handles:

- **CyberFoil / AeroFoil / Ownfoil shops**: the sectioned `/api/shop` API (CyberFoil Mode).
- **Tinfoil shops**: JSON indexes with `files` / `directories` (Tinfoil / DBI Mode).
- **DBI shops**: plain Apache / nginx / lighttpd folder listings. Every `.nsp/.nsz/.xci/.xcz`
  link is listed, and sub-folders are followed up to 4 levels deep.

ShopNX ships with no shops configured. Shops that only accept the genuine Tinfoil client
(HAUTH signing) are not supported.

Also installs from SD card, USB drive, USB (NS-USBloader), LAN and MTP.

## Install
Copy `switch/ShopNX/shopnx.nro` from `ShopNX.zip` (GitHub Releases) to your SD card.
Settings live in `sdmc:/switch/ShopNX/`.

## Build
CI builds in the `devkitpro/devkita64` container (`.github/workflows/build.yml`).
`python3 tools/make_art.py` regenerates the background, logo and icon.
