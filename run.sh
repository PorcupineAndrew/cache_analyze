#!/usr/bin/env bash
# **********************************************************************
# * Description   : run experiment script for Exp1
# * Last change   : 18:32:40 2020-04-03
# * Author        : Yihao Chen
# * Email         : chenyiha17@mails.tsinghua.edu.cn
# * License       : www.opensource.org/licenses/bsd-license.php
# **********************************************************************

echo -e "Exp1 script: run experiment..."

WORKING_DIR=`pwd`
SCRIPT_DIR=`dirname "$0"`
SCRIPT_DIR=`cd $SCRIPT_DIR; pwd`
DATA_DIR=`cd ${SCRIPT_DIR}/trace; pwd`
RESULT_DIR=${SCRIPT_DIR}/result

[ ! -d "$RESULT_DIR" ] && mkdir "$RESULT_DIR"
RESULT_DIR=`cd ${RESULT_DIR}; pwd`
cd $SCRIPT_DIR  && make clean && make

LIST_DATA="astar\nbzip2\nmcf\nperlbench"
LIST_ALGO="lru\nrand\ntree"
LIST_BLOCK="3\n5\n6"
LIST_WAY="DM\nFA\n2\n3"
LIST_ASSIGN="--write_assign\n"
LIST_WRITE="--write_back\n"

run_cache_arangement()
{
    while read -r data; do
        input_path=${DATA_DIR}/${data}.trace
        while read -r block; do
            while read -r way; do
                output_path=${RESULT_DIR}/${data}_${block}_${way}_lru_1_1.output
                [ -e "$output_path" ] && continue
                echo -e "run for $output_path"
                ./bin/cache_analyze \
                    --input_file        "$input_path" \
                    --output_file       "$output_path" \
                    --cache_size_bit    17 \
                    --block_size_bit    "$block" \
                    --way_size_bit      "$way"\
                    --replace_algo      lru \
                    --write_assign  --write_back &
            done < <(echo -e $LIST_WAY)
        done < <(echo -e $LIST_BLOCK)
    done < <(echo -e $LIST_DATA)
    wait
}

run_cache_replacement() {
    while read -r data; do
        input_path=${DATA_DIR}/${data}.trace
        while read -r algo; do
            output_path=${RESULT_DIR}/${data}_3_3_${algo}_1_1.output
            [ -e "$output_path" ] && continue
            echo -e "run for $output_path"
            ./bin/cache_analyze \
                --input_file        "$input_path" \
                --output_file       "$output_path" \
                --cache_size_bit    17 \
                --block_size_bit    3 \
                --way_size_bit      3 \
                --replace_algo      "$algo" \
                --write_assign  --write_back &
        done < <(echo -e $LIST_ALGO)
    done < <(echo -e $LIST_DATA)
    wait
}

run_cache_write_strategy() {
    while read -r data; do
        input_path=${DATA_DIR}/${data}.trace
        while read -r assign; do
            while read -r write; do
            output_path=${RESULT_DIR}/${data}_3_3_lru_$([ -z "$assign" ]; echo -e $?)_$([ -z "$write" ]; echo -e $?).output
            [ -e "$output_path" ] && continue
            echo -e "run for $output_path"
            ./bin/cache_analyze \
                --input_file        "$input_path" \
                --output_file       "$output_path" \
                --cache_size_bit    17 \
                --block_size_bit    3 \
                --way_size_bit      3 \
                --replace_algo      lru \
                "$assign"  "$write" &
            done < <(echo -e $LIST_WRITE)
        done < <(echo -e $LIST_ASSIGN)
    done < <(echo -e $LIST_DATA)
    wait
}

run_cache_arangement
run_cache_replacement
run_cache_write_strategy
