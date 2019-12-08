# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Support for custom port (argument: -p <PORT>) both on scan and bruteforce phases.
- Dry-run (argument: -D)
- Added the initial basis to support different ports on different targets
- Now is possible to specify the port on targets list (ex: 10.10.1.10:2222) (see #5)
- Shows time elapsed on each phase.
- Increase the maximum file descriptor number that can be opened by this process.

### Changed
- Separate Cbrutekrag verbosity from SSHLIB verbosity. (arguments: -v and -V respectively).
- The default maximum number of threads is calculated automatically.

### Removed
- Removed port option (-p <port>) in favor of new targets syntax (191.168.0.0/24:2222)

### Fixed
- Wait until all forks finished her work.

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


[Unreleased]: https://github.com/matricali/cbrutekrag/compare/0.4...HEAD
[0.4.0]: https://github.com/matricali/cbrutekrag/compare/0.3...0.4
[0.3.0]: https://github.com/matricali/cbrutekrag/compare/0.2.6...0.3
[0.2.1]: https://github.com/matricali/cbrutekrag/compare/0.1...0.2
[0.1.3]: https://github.com/matricali/cbrutekrag/releases/tag/0.1
