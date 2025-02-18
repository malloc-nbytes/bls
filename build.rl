module Build

with A = argv() in

let run, name = (
    len(A) > 1 && A[1] == "run",
    "main",
);

$f"cc -o {name} {name}.c";

if run {
    $f"./{name}";
}
