parallel -j7 "python3 analysis_scripts/parse_results.py 4 {1} {2}" ::: $(seq 10 10 500) ::: {1..8}
