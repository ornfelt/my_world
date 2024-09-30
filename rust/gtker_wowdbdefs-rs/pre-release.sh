#!/usr/bin/env bash
set -e

export RUSTFLAGS="-D warnings"
export CARGO_INCREMENTAL=0

cargo test
cargo clippy

