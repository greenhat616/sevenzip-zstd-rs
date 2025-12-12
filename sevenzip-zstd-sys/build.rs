use std::{env, path::PathBuf};

use glob::glob;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let sources = glob("deps/7-Zip-zstd/C/*.c")?.filter_map(Result::ok);
    cc::Build::new()
        .files(sources)
        .include("deps/7-Zip-zstd/C")
        .define("_REENTRANT", None)
        .define("_FILE_OFFSET_BITS", "64")
        .define("_LARGEFILE_SOURCE", None)
        .define("EXTERNAL_CODECS", None)
        .define("_7ZIP_LARGE_PAGES", None)
        .define("UNICODE", None)
        .define("_UNICODE", None)
        .try_compile("7zip")?;
    let include_dir = env::current_dir()?.join("deps/7-Zip-zstd/C");
    println!("cargo:include={}", include_dir.display());

    let bindings = bindgen::Builder::default()
        .header("unified.h")
        .default_macro_constant_type(bindgen::MacroTypeVariation::Signed)
        .default_enum_style(bindgen::EnumVariation::NewType {
            is_bitfield: false,
            is_global: false,
        })
        .clang_arg(format!("-I{}", include_dir.display()))
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()?;

    let bindings_out_path = PathBuf::from(env::var("OUT_DIR")?);
    bindings.write_to_file(bindings_out_path.join("bindings.rs"))?;

    println!("cargo:rerun-if-changed=deps/7-Zip-zstd/C/*.c");
    println!("cargo:rerun-if-changed=deps/7-Zip-zstd/C/*.h");
    println!("cargo:rerun-if-changed=unified.h");
    Ok(())
}
