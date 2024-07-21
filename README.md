[![license](https://img.shields.io/github/license/matricali/cbrutekrag.svg)](https://matricali.mit-license.org/2014)
[![GitHub contributors](https://img.shields.io/github/contributors/matricali/cbrutekrag.svg)](https://github.com/matricali/cbrutekrag/graphs/contributors) ![Build Status](https://github.com/matricali/cbrutekrag/actions/workflows/build.yml/badge.svg?event=push)
![Static Build Status](https://github.com/matricali/cbrutekrag/actions/workflows/static-build.yml/badge.svg?event=push)
[![Windows Build](https://github.com/matricali/cbrutekrag/actions/workflows/windows-build.yml/badge.svg?branch=master)](https://github.com/matricali/cbrutekrag/actions/workflows/windows-build.yml)
[![Latest stable release](https://img.shields.io/badge/dynamic/json.svg?label=stable&url=https%3A%2F%2Fapi.github.com%2Frepos%2Fmatricali%2Fcbrutekrag%2Freleases%2Flatest&query=%24.name&colorB=blue)](https://github.com/matricali/cbrutekrag/releases/latest)

# cbrutekrag
Penetration tests on SSH servers using dictionary attacks. Written in _C_.

> _brute krag_ means "brute force" in afrikáans

## Disclaimer
>This tool is for ethical testing purpose only.
>cbrutekrag and its owners can't be held responsible for misuse by users.
>Users have to act as permitted by local law rules.

## Run

```bash
$ cbrutekrag -h
       _                _       _
      | |              | |     | |
  ___ | |__  _ __ _   _| |_ ___| | ___ __ __ _  __ _
 / __|| '_ \| '__| | | | __/ _ \ |/ / '__/ _` |/ _` |
| (__ | |_) | |  | |_| | ||  __/   <| | | (_| | (_| |
 \___||_.__/|_|   \__,_|\__\___|_|\_\_|  \__,_|\__, |
          OpenSSH Brute force tool 0.6.0        __/ |
      (c) Copyright 2014-2024 Jorge Matricali  |___/


usage: ./cbrutekrag [-h] [-v] [-aA] [-D] [-P] [-T TARGETS.lst] [-C credentials.lst]
                [-t THREADS] [-F OUTPUT FORMAT] [-o OUTPUT.txt] [TARGETS...]

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
  -F <format>       Output log format
                    Available placeholders:
                    %DATETIME%, %HOSTNAME%
                    %PORT%, %USERNAME%, %PASSWORD%
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

### Customizable output format

Output format can be easily customizable using the command line option `-F`

Example: `./cbrutekrag -F "%HOSTNAME%:%PORT%|%USERNAME%|%PASSWORD%\n"`, which
produces an output like:

```
192.168.0.100:22|root|toor
192.168.0.105:22|ubnt|ubnt
```

#### Default value

`%DATETIME%\t%HOSTNAME%:%PORT%\t%USERNAME%\t%PASSWORD%\n`

```
2024/04/01 13:05:13     192.168.0.100:22     root    admin
```

#### Placeholders

|Placeholder|Description                       |Example            |
|-----------|----------------------------------|-------------------|
|%DATETIME% |Replaced by `Y/m/d HH:ii:ss` date |2024/04/01 12:46:27|
|%HOSTNAME% |Replaced by hostname or IPv4      |192.168.0.100      |
|%PORT%     |Replaced by connection port       |22                 |
|%USERNAME% |Replaced by username used         |root               |
|%PASSWORD% |Replaced by password used         |admin              |
|\n         |Replaced by LF                    |                   |
|\t         |Replaced by TAB                   |                   |


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
