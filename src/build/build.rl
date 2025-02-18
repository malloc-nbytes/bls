#!/usr/local/bin/earl

### Build script if using EARL

module Build

import "std/system.rl"; as sys

fn usage() {
    println("Usage:");
    println("  ", argv()[0], " -- <build|install <destdir>>");
    exit(1);
}

with A = argv() in
if len(A) <= 1
    || !("build", "install").contains(A[1])
    || A[1] == "install" && len(A) <= 2 {
    usage();
}

let name = "bls";

if argv()[1] == "build" {
    let files = sys::ls(".");
    if files.contains("./out") {$"rm -r ./out";}
    $f"cc -O2 -I../include/ -o {name} ../*.c";
    $f"mkdir ./out && mv ./{name} ./out";
}
else {
    $f"sudo cp ./out/{name} " + argv()[2];
}
