[![license](https://img.shields.io/github/license/matricali/cbrutekrag.svg)](https://matricali.mit-license.org/2014) [![GitHub contributors](https://img.shields.io/github/contributors/matricali/cbrutekrag.svg)](https://github.com/matricali/cbrutekrag/graphs/contributors) ![Build Status](https://github.com/matricali/cbrutekrag/actions/workflows/build.yml/badge.svg?event=push) ![Static Build Status](https://github.com/matricali/cbrutekrag/actions/workflows/static-build.yml/badge.svg?event=push) [![Latest stable release](https://img.shields.io/badge/dynamic/json.svg?label=stable&url=https%3A%2F%2Fapi.github.com%2Frepos%2Fmatricali%2Fcbrutekrag%2Freleases%2Flatest&query=%24.name&colorB=blue)](https://github.com/matricali/cbrutekrag/releases/latest)

# cbrutekrag
Penetration tests on SSH servers using dictionary attacks. Written in _C_.

> _brute krag_ means "brute force" in afrikáans

## Disclaimer
>This tool is for ethical testing purpose only.   
>cbrutekrag and its owners can't be held responsible for misuse by users.   
>Users have to act as permitted by local law rules.

## Requirements
**cbrutekrag** uses **libssh** - The SSH Library (http://www.libssh.org/)

## Build

Requirements:

* `make`
* `gcc` compiler
* `libssh-dev`

```bash
git clone --depth=1 https://github.com/matricali/cbrutekrag.git
cd cbrutekrag
make
make install
```

## Static build

Requirements:

* `cmake`
* `gcc` compiler
* `make`
* `libssl-dev`
* `libz-dev`

```bash
git clone --depth=1 https://github.com/matricali/cbrutekrag.git
cd cbrutekrag
bash static-build.sh
make install
```

## Run

```bash
$ cbrutekrag -h
       _                _       _
      | |              | |     | |
  ___ | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _
 / __|| '_ \| '__| | | | __/ _ \ |/ / '__/ _` |/ _` |
| (__ | |_) | |  | |_| | ||  __/   <| | | (_| | (_| |
 \___||_.__/|_|   \__,_|\__\___|_|\_\_|  \__,_|\__, |
          OpenSSH Brute force tool 0.5.0        __/ |
      (c) Copyright 2014-2022 Jorge Matricali  |___/


usage: ./cbrutekrag [-h] [-v] [-aA] [-D] [-P] [-T TARGETS.lst] [-C combinations.lst]
		[-t THREADS] [-o OUTPUT.txt] [TARGETS...]

  -h                This help
  -v                Verbose mode
  -V                Verbose mode (sshlib)
  -s                Scan mode
  -D                Dry run
  -P                Progress bar
  -T <targets>      Targets file
  -C <combinations> Username and password file
  -t <threads>      Max threads
  -o <output>       Output log file
  -a                Accepts non OpenSSH servers
  -A                Allow servers detected as honeypots.
```

## Example usages
```bash
cbrutekrag -T targets.txt -C combinations.txt -o result.log
cbrutekrag -s -t 8 -C combinations.txt -o result.log 192.168.1.0/24
```

### Supported targets syntax

* 192.168.0.1
* 10.0.0.0/8
* 192.168.100.0/24:2222
* 127.0.0.1:2222

### Combinations file format
```
root root
root password
root $BLANKPASS
$TARGET root
root $TARGET
```

#### Combinations file placeholders

|Placeholder|Purpose|As password| As username|
|------------|------|-----------|------------|
|$BLANKPASS|Blank password|✔️|-|
|$TARGET|Use hostname or IP as a password|✔️|✔️|
