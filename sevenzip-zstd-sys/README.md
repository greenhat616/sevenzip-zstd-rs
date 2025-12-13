# sevenzip-zstd-sys

[![Crates.io](https://img.shields.io/crates/v/sevenzip-zstd-sys.svg)](https://crates.io/crates/sevenzip-zstd-sys)
[![Documentation](https://docs.rs/sevenzip-zstd-sys/badge.svg)](https://docs.rs/sevenzip-zstd-sys)

Raw Rust bindings to the [7-Zip-zstd](https://github.com/mcmilk/7-Zip-zstd) library.

This crate provides unsafe low-level FFI bindings to 7-Zip-zstd, which is an enhanced version of 7-Zip with support for additional compression codecs including:

- **Zstandard** - Real-time compression algorithm with high compression ratios
- **Brotli** - Generic-purpose lossless compression with excellent density
- **LZ4** - Extremely fast compression/decompression
- **LZ5** - Better ratio than LZ4 at cost of speed
- **Lizard** - Efficient compression with fast decompression
- **Fast LZMA2** - Faster LZMA2 implementation with parallel buffering

## Usage

This is a low-level sys crate providing raw FFI bindings. Most users should use the higher-level `sevenzip-zstd` crate instead.

Add this to your `Cargo.toml`:

```toml
[dependencies]
sevenzip-zstd-sys = "0.1"
```

## Building

This crate builds 7-Zip-zstd from source using the bundled C/C++ code. The build process requires:

- A C/C++ compiler (MSVC on Windows, GCC/Clang on Unix-like systems)
- Standard build tools (make, etc.)

## License

This crate contains bindings to 7-Zip-zstd, which is licensed under the GNU Lesser General Public License v2.1 or later (LGPL-2.1-or-later).

The Rust bindings in this crate are also licensed under LGPL-2.1-or-later to maintain consistency with the upstream library.

See the LICENSE file for details.

## Upstream

The bundled 7-Zip-zstd library is from: https://github.com/mcmilk/7-Zip-zstd

7-Zip-zstd is maintained by Tino Reichardt and is based on the original 7-Zip by Igor Pavlov.

## Safety

This crate provides unsafe FFI bindings. Users are responsible for ensuring correct usage according to the upstream library's API contract.
