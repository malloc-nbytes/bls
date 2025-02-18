#!/usr/local/bin/earl

module Build

with A = argv() in

let run, name, args = (
    len(A) > 1 && A[1] == "run",
    "bls",
    case len(A) > 2 of {
        true = A[2:];
        _ = [];
    },
);

$f"cc -Iinclude/ -o {name} *.c";

if run {
    $f"./{name}" + args.fold(|a, acc| { acc + " " + a; }, "");
}
