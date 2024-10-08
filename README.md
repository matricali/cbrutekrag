[![license](https://img.shields.io/github/license/matricali/cbrutekrag.svg)](https://matricali.mit-license.org/2014)
[![GitHub contributors](https://img.shields.io/github/contributors/matricali/cbrutekrag.svg)](https://github.com/matricali/cbrutekrag/graphs/contributors) [![Build](https://github.com/matricali/cbrutekrag/actions/workflows/build.yml/badge.svg)](https://github.com/matricali/cbrutekrag/actions/workflows/build.yml)
[![Static Build](https://github.com/matricali/cbrutekrag/actions/workflows/static-build.yml/badge.svg)](https://github.com/matricali/cbrutekrag/actions/workflows/static-build.yml)
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

      https://github.com/matricali/cbrutekrag


usage: ./cbrutekrag [-h] [-v] [-aA] [-D] [-P] [-T TARGETS.lst] [-C credentials.lst]
                [-t THREADS] [-f OUTPUT FORMAT] [-o OUTPUT.txt] [-F SCAN OUTPUT FORMAT] [-O SCAN_OUTPUT.txt] [TARGETS...]

-h, --help                This help
-v, --verbose             Verbose mode
-V, --verbose-sshlib      Verbose mode (sshlib)
-s, --scan                Scan mode
-D, --dry-run             Dry run
-P, --progress            Progress bar
-T, --targets <file>      Targets file
-C, --credentials <file>  Username and password file
-t, --threads <threads>   Max threads
-o, --output <file>       Output log file
-F, --format <pattern>    Output log format
                          Available placeholders:
                          %DATETIME%, %HOSTNAME%
                          %PORT%, %USERNAME%, %PASSWORD%
-O, --scan-output <file>  Output log file for scanner
-F, --scan-format <pattern> Output log format for scanner
                          Available placeholders:
                          %DATETIME%, %HOSTNAME%
                          %PORT%, %BANNER%.
                          Default:
                          "%HOSTNAME%:%PORT%\t%BANNER%\n"
-a, --allow-non-openssh   Accepts non OpenSSH servers
-A, --allow-honeypots     Allow servers detected as honeypots
    --timeout <seconds>   Sets connection timeout (Default: 3)
    --check-http <host>   Tries to open a TCP Tunnel after successful login
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

Output format can be easily customizable using the command line option `-f`

Example: `./cbrutekrag -f "%HOSTNAME%:%PORT%|%USERNAME%|%PASSWORD%\n"`, which
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

### Customizable output format for scanner

Output format can be easily customizable using the command line option `-F`

Example: `./cbrutekrag -F "%HOSTNAME%\t%PORT%\t%BANNER%\n"`, which
produces an output like:

```
192.168.0.100 22  SSH-2.0-OpenSSH_6.0p1 Debian-4+deb7u2
192.168.0.105 22  SSH-2.0-OpenSSH_9.2p1 Debian-2+deb12u2
```

#### Default value

`%HOSTNAME%:%PORT%\t%BANNER%\n`

```
192.168.0.100:22  SSH-2.0-OpenSSH_9.2p1 Debian-2+deb12u2
```

#### Placeholders

|Placeholder|Description                       |Example            |
|-----------|----------------------------------|-------------------|
|%DATETIME% |Replaced by `Y/m/d HH:ii:ss` date |2024/04/01 12:46:27|
|%HOSTNAME% |Replaced by hostname or IPv4      |192.168.0.100      |
|%PORT%     |Replaced by connection port       |22                 |
|%BANNER%   |Replaced by server banner         |SSH-2.0-OpenSSH_9.2p1 Debian-2+deb12u2|
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
