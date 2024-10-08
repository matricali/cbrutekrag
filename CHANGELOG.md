# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Hostname can be used as a username or password dynamically ($TARGET) (#28)
- Output log format are now customizable (#30)
- Windows support
- Timeout can be specified using --timeout <seconds> argument. Default: 3
- Separated and customizable scanner output log -F <format> -O <file> (#27)
- --check-http for check TCP Tunnel after successful login
- A separated thread to monitoring the progress of processes
- Estimated time remaining and rate indicators
- Targets and credentials can be loaded from multiple files

### Changed

- Bruteforce process now uses pthread instead of fork
- Short option for bruteforce output log format changed from -F to -f
- The current state is no longer reported by the worker threads. Is reported by
  monitoring thread instead

### Fixed

- Added missing wrapper for FD_SET in static build
- ssh_get_error shouldn't be used after ssh_free
- Buffer overflow in banner grabber
- Division by zero when progress bar has no elements
- Update total work to do after targets filtering
- ssh_free releases the memory allocated for banner
- Flush write buffer to logs immediately
- Progress bar on smaller terminal sizes (#11)

### Removed

- Honeypot detection

## [0.5.0] - 2020-03-07
### Added
- Support for custom port (argument: -p <PORT>) both on scan and bruteforce phases.
- Dry-run (argument: -D)
- Added the initial basis to support different ports on different targets
- Now is possible to specify the port on targets list (ex: 10.10.1.10:2222) (see #5)
- Shows time elapsed on each phase.
- Increase the maximum file descriptor number that can be opened by this process.
- manpages (`man cbrutekrag`)
- Debug bracktrace symbols
- Ignore as default non OpenSSH servers (argument flag -a to accept)
- Detects and skip NON SSH servers (tcpwrapped).
- Ignoring servers that don't support password authentication.

### Changed
- Separate Cbrutekrag verbosity from SSHLIB verbosity. (arguments: -v and -V respectively).
- The default maximum number of threads is calculated automatically.
- Allow servers detected as honeypot (argument flag -A)
- Improved detection of non-eligible servers.

### Removed
- Removed port option (-p <port>) in favor of new targets syntax (191.168.0.0/24:2222)

### Fixed
- Wait until all forks finished her work.
- Ignore SIGPIPE
- Fixed false positives in servers which login are interactive.

## [0.4.0] - 2018-09-02
### Added
- Multithreaded port scan, discard targets from batch if the port is closed (argument: -s).
- Honeypot detection (?).
- Support for target list as arguments. It can be combined with targets file.
- Targets can be a CIDR IPv4 block.

### Fixed
- Initialize hostnames wordlist.
- Aborts bruteforce phase if there is no targets after scan or honeypot detection phases.

## [0.3.0] - 2018-08-26
### Added
- Compatibility with libssh-dev < 0.6.0.

### Changed
- Improved logging.
- Improved help (-h).

### Fixed
- Fixed a segmentation fault when it does not had an open output file.
- Update progress bar at the end to complete 100%.

## [0.2.1] - 2018-01-02
### Added
- Support for empty password ($BLANKPASS in dictionary).

### Changed
- Improved fork model.

## [0.1.3] - 2017-12-29
### Added
- Multithread.
- Progress bar.


[Unreleased]: https://github.com/matricali/cbrutekrag/compare/0.5...HEAD
[0.5.0]: https://github.com/matricali/cbrutekrag/compare/0.4...0.5
[0.4.0]: https://github.com/matricali/cbrutekrag/compare/0.3...0.4
[0.3.0]: https://github.com/matricali/cbrutekrag/compare/0.2.6...0.3
[0.2.1]: https://github.com/matricali/cbrutekrag/compare/0.1...0.2
[0.1.3]: https://github.com/matricali/cbrutekrag/releases/tag/0.1
