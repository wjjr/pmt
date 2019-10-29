#!/bin/bash
trap 'rm -rf "$TMP_DIR"; trap - INT; kill -s INT "$$"' INT

PMT='./bin/pmt'

for CMP in make gcc mktemp mkdir rm shuf head basename time diff grep agrep; do
    hash $CMP 2>/dev/null || {
        echo "$CMP: command not found" >&2
        exit 127
    }
done

if ! [ -x $PMT ]; then
    make -s clean && make -s || exit $?
    if ! [ -x $PMT ]; then
        echo "$PMT: No such file or directory" >&2
        exit 127
    fi
fi

TEST_RUNTIME=${1-true}
ONLY_MATCHING=${2-true}
EDIT_DISTANCE=${3-0}
DATA_DIR=./data
RESULTS_DIR=./data/results
TMP_DIR=$(mktemp -d -p /dev/shm -t pmt.tmp.XXXXXXXXXX)

if [ "$EDIT_DISTANCE" -eq 0 ]; then
    ALGORITHMS="grep $($PMT -h | awk '/Supported algorithms/{a=1;next}(a==1){print $1}')"
    PATTERN_LENGTHS='2 4 8 16 32 64'
else
    ALGORITHMS="agrep $($PMT -h | awk '/\[approximate\]/{print $1}')"
    PATTERN_LENGTHS='4 8 16 32'
fi

function get_pattern() {
    _LC_ALL=$LC_ALL
    LC_ALL=C

    PATTERN_LENGTH=$1
    FILE=$2
    PATTERN=''

    while ! [ ${#PATTERN} -eq "$PATTERN_LENGTH" ]; do
        PATTERN=$(shuf -n1 <(head -n5000 "$FILE"))
        START=$(shuf -n1 -i 0-$((${#PATTERN} - PATTERN_LENGTH)) 2>/dev/null)
        PATTERN="${PATTERN:$START:$PATTERN_LENGTH}"
    done

    echo "$PATTERN"

    LC_ALL=$_LC_ALL
}

function runtime() {
    if $TEST_RUNTIME; then
        sync
        echo 3 | sudo tee /proc/sys/vm/drop_caches >/dev/null
    fi

    command time -f "%e" "${@:2}"
}

if $TEST_RUNTIME; then
    STEPS='1 2 3'
    FLAGS='-c'
else
    STEPS='1'

    if $ONLY_MATCHING; then
        FLAGS='-bo'
    else
        FLAGS='-b'
    fi
fi

for FILE in "${DATA_DIR}/"*; do
    if [ -d "$FILE" ] || ! [ -s "$FILE" ] || ! [ -r "$FILE" ]; then
        continue
    elif ! [ -f "$FILE" ]; then
        echo "$(basename "$0"): $FILE: No such file or directory" >&2
        continue
    fi

    echo -e "--\nFile: \"${FILE}\""

    declare -A CSV=([i]='I' [file]="File,$FILE" [pat]='Pattern' [len]='Length')
    for ALGORITHM in $ALGORITHMS; do
        CSV[algorithm_$ALGORITHM]=$ALGORITHM
    done

    for PATTERN_LENGTH in $PATTERN_LENGTHS; do
        if [ "$PATTERN_LENGTH" -le "$EDIT_DISTANCE" ]; then
            continue
        fi

        PATTERN=$(get_pattern "$PATTERN_LENGTH" "$FILE")
        echo "$PATTERN_LENGTH : $FILE : $([ "$EDIT_DISTANCE" -gt 0 ] && echo "Edit=$EDIT_DISTANCE ")\"$PATTERN\""

        for I in $STEPS; do
            echo -n .

            CSV[i]+=",$I"
            CSV[pat]+=","$([ $I -eq 1 ] && echo "\"${PATTERN//\"/\\\"}\"")
            CSV[len]+=","$([ $I -eq 1 ] && echo "$PATTERN_LENGTH")

            for ALGORITHM in $ALGORITHMS; do
                echo -n .

                OUT_FILE="${TMP_DIR}/${ALGORITHM}.${PATTERN_LENGTH}.I${I}"
                TIME_FILE="${OUT_FILE}.time"

                if [ "uk" = "$ALGORITHM" ] || [ "wm" = "$ALGORITHM" ]; then
                    ! $TEST_RUNTIME && $ONLY_MATCHING && continue
                fi

                if [ "grep" = "$ALGORITHM" ]; then
                    GREP_OUT_FILE=$OUT_FILE
                    runtime "$ALGORITHM" grep -F $FLAGS "$PATTERN" "$FILE" >"$OUT_FILE" 2>"$TIME_FILE"
                elif [ "agrep" = "$ALGORITHM" ]; then
                    GREP_OUT_FILE=$OUT_FILE
                    runtime "$ALGORITHM" agrep -"$EDIT_DISTANCE" "$(printf '%q' "$PATTERN")" "$FILE" >"$OUT_FILE" 2>"$TIME_FILE"
                else
                    runtime "$ALGORITHM" "$PMT" -a "$ALGORITHM" $FLAGS "$PATTERN" "$FILE" >"$OUT_FILE" 2>"$TIME_FILE"
                fi

                if $TEST_RUNTIME; then
                    CSV[algorithm_$ALGORITHM]+=",$(cat "$TIME_FILE")"
                else
                    diff -u --color=always "$GREP_OUT_FILE" "$OUT_FILE" | less -Fr

                    if [ "${PIPESTATUS[0]}" != "0" ]; then
                        DIFF_ERROR=1
                    fi
                fi
            done
        done

        echo
    done

    if $TEST_RUNTIME; then
        mkdir -p "$RESULTS_DIR"
        RESULTS_FILE="${RESULTS_DIR}/$(basename "$FILE")-$(date -Is).csv"

        {
            echo "${CSV[i]}"
            echo "${CSV[file]}"
            echo "${CSV[pat]}"
            echo "${CSV[len]}"

            for ALGORITHM in $ALGORITHMS; do
                echo "${CSV[algorithm_$ALGORITHM]}"
            done
        } >>"$RESULTS_FILE"

        echo "Exported CSV: $RESULTS_FILE"
    fi
done

if $TEST_RUNTIME || [ ! "$DIFF_ERROR" = "1" ]; then
    rm -rf "$TMP_DIR"
else
    echo "$TMP_DIR"
    rm -rfI "$TMP_DIR"
fi

exit 0
