#!/usr/bin/env bash
set -euo pipefail

N="${1:-500000000}"
CSV="${2:-resultados.csv}"
BUILD_DIR="${3:-build}"
CC="${CC:-gcc}"
CFLAGS="${CFLAGS:--O2 -fopenmp}"
LDFLAGS="${LDFLAGS:--lm}"
SEQ_CFLAGS="${SEQ_CFLAGS:--O2}"
SEQ_LDFLAGS="${SEQ_LDFLAGS:--lm -lgomp}"
NICE_VALUE="${NICE_VALUE:--20}"
THREADS=4
CHUNK=10

mkdir -p "$BUILD_DIR"

programas=(
    "naive,static,omp_primos_static.c,omp_primos_static"
    "naive,dynamic,omp_primos_dynamic.c,omp_primos_dynamic"
    "naive,guided,omp_primos_guided.c,omp_primos_guided"
    "bag_of_tasks,static,bot_static.c,bot_static"
    "bag_of_tasks,dynamic,bot_dynamic.c,bot_dynamic"
    "bag_of_tasks,guided,bot_guided.c,bot_guided"
)

sequencial_fonte="omp_primos_static.c"
sequencial_binario="primos_sequencial"

echo "programa,tipo,politica,n,threads,chunk,total_primos,tempo_segundos" > "$CSV"

for item in "${programas[@]}"; do
    IFS=',' read -r tipo politica fonte binario <<< "$item"
    saida="$BUILD_DIR/$binario"

    echo "Compilando $fonte..."
    "$CC" $CFLAGS "$fonte" $LDFLAGS -o "$saida"
done

echo "Compilando $sequencial_fonte sem paralelizar..."
"$CC" $SEQ_CFLAGS "$sequencial_fonte" $SEQ_LDFLAGS -o "$BUILD_DIR/$sequencial_binario"

echo "Executando $sequencial_binario com n=$N e nice=$NICE_VALUE..."
output="$(nice -n "$NICE_VALUE" "$BUILD_DIR/$sequencial_binario" "$N")"

total="$(printf '%s\n' "$output" | awk '/Quant/ {print $NF}')"
tempo="$(printf '%s\n' "$output" | awk '/Tempo/ {print $NF}')"

printf '%s,%s,%s,%s,%s,%s,%s,%s\n' \
    "$sequencial_binario" "sequencial" "sem_openmp" "$N" "1" "0" "$total" "$tempo" >> "$CSV"

for item in "${programas[@]}"; do
    IFS=',' read -r tipo politica fonte binario <<< "$item"
    saida="$BUILD_DIR/$binario"

    echo "Executando $binario com n=$N e nice=$NICE_VALUE..."
    output="$(nice -n "$NICE_VALUE" "$saida" "$N")"

    total="$(printf '%s\n' "$output" | awk '/Quant/ {print $NF}')"
    tempo="$(printf '%s\n' "$output" | awk '/Tempo/ {print $NF}')"

    printf '%s,%s,%s,%s,%s,%s,%s,%s\n' \
        "$binario" "$tipo" "$politica" "$N" "$THREADS" "$CHUNK" "$total" "$tempo" >> "$CSV"
done

echo "Resultados salvos em $CSV"
