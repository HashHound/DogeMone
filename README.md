# Dogemone

Copyright (c) 2014-2024 The Monero Project.   
Dogemone is a Privacy focused gpu mineable memecoin based on [Monero](README_original.md)

![Build-Linux](https://github.com/HashHound/Dogemone/workflows/Build-Linux/badge.svg)

## Production & Development

Active Branches:
- Stable: master(latest/release)
- Unstable: dogemone-v1.0(latest)
- Testing: N/A

To contribute to the Dogemone Project, please make all pull requests to testing branch.

For production, please use the _latest or tagged release_ of the _master_ branch.

## Resources
- Webpage: [dogemone.foundation](https://getdogemone.meme)
- Explorer: [explorer.dogemone.foundation](https://explorer.getdogemone.meme)
- Pool List: [miningpoolstats.stream/dogemone](https://miningpoolstats.stream/dogemone)
- GitHub: [github.com/HashHound/Dogemone](https://github.com/HashHound/dogemone)

## Social/Contact

- Bitcointalk []()
- Discord: [discord.gg/SmcFCPu](https://discord.gg/SmcFCPu)
- Reddit: [r/DogemoneProject/](https://www.reddit.com/r/DogemoneProject/)
- Twitter: [@DogemoneProject](https://x.com/DogemoneProject)
- Email: Dogemone@gmail.com

## Specifications & Emission

- Coin ticker: DOGM
- Total supply: 18,400,000 coins before tail emission
- Tail emission: ~158,000 coins each year (starting at year 8)
- Decimal places: 12
- PoW hash algorithm: Cuckaroo29s
- Block time: 15 seconds
- Difficulty Adjustment Algorithm: Monero DAA
- Genesis block: 2018-11-16 (November 16, 2018) at 09:06:03 (UTC)
- Premine: No
- Developer fee: 5%
- Founders reward: No
- Mainnet default P2P port: 49200
- Mainnet default RPC port: 53000

## Donation Address
dmzGbnVaU4yZ47Vbq235MLTjLuH1HfrznXq6VfPDYQLXW6d2tVi2aXnbzpNJXkGXMUP5m5kQoY2EG5ESpgp3gA8DAZLuSeEaZV

## Build on linux

install deps:

`sudo apt-get install build-essential cmake pkg-config libboost-all-dev libssl-dev libzmq3-dev libunbound-dev libsodium-dev libreadline6-dev libpgm-dev libnorm-dev`

clone repo:

`git clone --recursive https://github.com/HashHound/Dogemone`

build daemon and wallet:

`cd Dogemone && mkdir build && cd build && cmake .. && make daemon simplewallet`

or build everything:

`cd Dogemone && mkdir build && cd build && cmake .. && make`

## Build on Windows (using MinGW)

install deps:

`pacman -S mingw-w64-x86_64-toolchain make mingw-w64-x86_64-cmake mingw-w64-x86_64-boost mingw-w64-x86_64-openssl mingw-w64-x86_64-zeromq mingw-w64-x86_64-libsodium mingw-w64-x86_64-hidapi`

clone repo:

`git clone --recursive https://github.com/HashHound/Dogemone`

build daemon and wallet:

`cd Dogemone && mkdir build && cd build`

`cmake .. -G "MinGW Makefiles" -D STATIC=ON -D ARCH="x86-64" -D BUILD_64=ON -D CMAKE_BUILD_TYPE=Release -D BUILD_TAG="win-x64"`

`mingw32-make`
