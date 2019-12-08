[![license](https://img.shields.io/github/license/matricali/cbrutekrag.svg)](https://matricali.mit-license.org/2014) [![GitHub contributors](https://img.shields.io/github/contributors/matricali/cbrutekrag.svg)](https://github.com/matricali/cbrutekrag/graphs/contributors) [![Build Status](https://travis-ci.org/matricali/cbrutekrag.svg?branch=master)](https://travis-ci.org/matricali/cbrutekrag) [![Latest stable release](https://img.shields.io/badge/dynamic/json.svg?label=stable&url=https%3A%2F%2Fapi.github.com%2Frepos%2Fmatricali%2Fcbrutekrag%2Freleases%2Flatest&query=%24.name&colorB=blue)](https://github.com/matricali/cbrutekrag/releases/latest)

# cbrutekrag
Penetration tests on SSH servers using dictionary attacks. Written in _C_.

> _brute krag_ means "brute force" in afrikáans

## Disclaimer
>This tool is for ethical testing purpose only.   
>cbrutekrag and its owners can't be held responsible for misuse by users.   
>Users have to act as permitted by local law rules.

## Requirements
* `gcc` compiler
* `libssh`

## Build
```bash
git clone --depth=1 https://github.com/matricali/cbrutekrag.git
cd cbrutekrag
make
make install
```
Then you can do
```bash
$ cbrutekrag -h
       _                _       _
      | |              | |     | |
  ___ | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _
 / __|| '_ \| '__| | | | __/ _ \ |/ / '__/ _` |/ _` |
| (__ | |_) | |  | |_| | ||  __/   <| | | (_| | (_| |
 \___||_.__/|_|   \__,_|\__\___|_|\_\_|  \__,_|\__, |
          OpenSSH Brute force tool 0.5.0        __/ |
      (c) Copyright 2014-2019 Jorge Matricali  |___/


usage: ./cbrutekrag [-h] [-v] [-D] [-p PORT] [-T TARGETS.lst] [-C combinations.lst]
		[-t THREADS] [-o OUTPUT.txt] [TARGETS...]

  -h                This help
  -v                Verbose mode
  -V                Verbose mode (sshlib)
  -s                Scan mode
  -D                Dry run
  -p <port>         Port (default: 22)
  -T <targets>      Targets file
  -C <combinations> Username and password file
  -t <threads>      Max threads
  -o <output>       Output log file
```

## Example usages
```bash

# Many targets, many pre-made combinations of user and password separated by space.
cbrutekrag -T targets.txt -C combinations.txt -o result.log
```
