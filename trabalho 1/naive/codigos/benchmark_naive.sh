#!/usr/bin/env bash
set -u
set -o pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR"
BIN_DIR="$SCRIPT_DIR/bin"
LOG_DIR="$SCRIPT_DIR/logs"

NP="${NP:-4}"
LIMIT="${LIMIT:-500000000}"
RESULT_CSV="${1:-$SCRIPT_DIR/resultados_execucao.csv}"

mkdir -p "$BIN_DIR" "$LOG_DIR"

TIMESTAMP="$(date '+%Y-%m-%d %H:%M:%S')"

# CSV: fonte -> tempo de execucao (e status) para cada programa.
echo "data_hora,fonte,executavel,np,n,tempo_execucao,status,log_arquivo" > "$RESULT_CSV"

shopt -s nullglob
sources=("$SRC_DIR"/*.c)

if [ ${#sources[@]} -eq 0 ]; then
    echo "Nenhum arquivo .c encontrado em: $SRC_DIR"
    exit 1
fi

for src_path in "${sources[@]}"; do
    src_file="$(basename -- "$src_path")"
    exe_name="${src_file%.c}"
    exe_path="$BIN_DIR/$exe_name"

    compile_log="$LOG_DIR/${exe_name}_compile.log"
    run_log="$LOG_DIR/${exe_name}_run.log"

    printf 'Compilando %s...\n' "$src_file"
    if ! mpicc "$src_path" -lm -o "$exe_path" > "$compile_log" 2>&1; then
        echo "Falha na compilacao: $src_file"
        echo "$TIMESTAMP,$src_file,$exe_name,$NP,$LIMIT,,compile_error,$compile_log" >> "$RESULT_CSV"
        continue
    fi

    printf 'Executando %s com mpirun -np %s %s...\n' "$exe_name" "$NP" "$LIMIT"
    run_output="$(mpirun -np "$NP" "$exe_path" "$LIMIT" 2>&1)"
    run_status=$?
    printf '%s\n' "$run_output" > "$run_log"

    if [ $run_status -ne 0 ]; then
        echo "Falha na execucao: $exe_name"
        echo "$TIMESTAMP,$src_file,$exe_name,$NP,$LIMIT,,run_error,$run_log" >> "$RESULT_CSV"
        continue
    fi

    tempo="$(printf '%s\n' "$run_output" | awk -F':' '/Tempo de execucao/{val=$2} END{gsub(/^[[:space:]]+|[[:space:]]+$/, "", val); print val}')"

    if [ -z "$tempo" ]; then
        echo "Tempo nao encontrado na saida de: $exe_name"
        echo "$TIMESTAMP,$src_file,$exe_name,$NP,$LIMIT,,no_time_found,$run_log" >> "$RESULT_CSV"
    else
        echo "$TIMESTAMP,$src_file,$exe_name,$NP,$LIMIT,$tempo,ok,$run_log" >> "$RESULT_CSV"
    fi
done

echo
echo "Execucoes finalizadas. Resultado salvo em: $RESULT_CSV"
echo "Logs de compilacao/execucao em: $LOG_DIR"
