# pkgmgr

pkgmgr is a simple package manager for Unix systems.

Packages are distributed through static repositories over HTTP(S). A repository consists only of files and does not require any server-side package management software.

## Getting Started

Configure one or more repositories in `/var/pkgmgr/repos.csv`, then synchronize them:

```sh
pkgmgr sync
```

Search for or inspect packages:

```sh
pkgmgr info test
```

Install a package:

```sh
pkgmgr install test
```

Remove a package:

```sh
pkgmgr remove test
```

## Repository Format

A repository has the following structure:

```text
repo/
  index.csv
  pkgs/
    test1/
      meta.csv
      content.tar.zst
```

`index.csv` contains the package names and metadata paths. Package metadata and package contents are stored separately.

## Building

pkgmgr uses [cbld](https://github.com/drwxor/cbld) to build the project.

```sh
cbld build
```

## Status

pkgmgr is experimental and under development. Its repository and package formats are subject to change.

## Contributing

Contributions, bug reports, and ideas are welcome.

## License

See `LICENSE`.
