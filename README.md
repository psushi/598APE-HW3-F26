# CS598APE HW3

Compute the sandpile identity on a Pacman-shaped grid. The program uses four
threads and saves `identity.png`.

## Build and run

With Docker running, from the repository root:

```bash
./dockerbuild.sh
./dockerrun.sh
```

Inside the container:

```bash
make all
./sandpile.exe 1001
```

## Benchmark

Two warmups and five measured runs:

```bash
python3 scripts/runtimes.py 1001
```

Use `401` for a shorter run.

## Compare with the original

Inside the container:

```bash
mkdir -p artifacts
git show 15fe837:sandpile.c > artifacts/original.c
gcc artifacts/original.c -I. -O3 -lm -g -Werror -o artifacts/original.exe
python3 scripts/runtimes.py 1001 --binary artifacts/original.exe --warmups 0 --runs 1
cp identity.png artifacts/original.png
python3 scripts/runtimes.py 1001 --reference artifacts/original.png
```

The original takes several minutes. Increase `--warmups` and `--runs` for
repeated baseline measurements.

## Profile

```bash
mkdir -p artifacts/profile
perf record -e cpu-clock:u -F 997 -o artifacts/profile/perf.data -- ./sandpile.exe 401
perf report --stdio -i artifacts/profile/perf.data
perf annotate --stdio --source -i artifacts/profile/perf.data
```
