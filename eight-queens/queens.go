package main

import (
	"fmt"
	"os"
	"strconv"
	"time"
)

func queens(SIZE, nQueen, cols, diags, r_diags uint64) uint64 {
	if nQueen == SIZE {
		return 1
	}
	var nSolutions uint64
	for i := uint64(0); i < SIZE; i++ {
		col := uint64(1 << i)
		diag := uint64(1 << (nQueen + i))
		r_diag := uint64(1 << (SIZE - 1 - nQueen + i))

		if col&cols == 0 && diag&diags == 0 && r_diag&r_diags == 0 {
			nSolutions += queens(SIZE, nQueen+1, cols|col, diags|diag, r_diags|r_diag)
		}
	}
	return nSolutions
}

func main() {
	if len(os.Args) != 2 {
		fmt.Println("Usage: ./", os.Args[0], "<queen size>")
		return
	}
	// ParseUint(s string, base int, bitSize int)
	SIZE, err := strconv.ParseUint(os.Args[1], 10, 64)
	if err != nil {
		fmt.Println("Usage: ./", os.Args[0], "<queen size>")
		return
	}

	start_time := time.Now()
	// nSolution := queens(strconv.Atoi(os.Args[1]), 0, 0, 0, 0)
	nSolution := queens(SIZE, 0, 0, 0, 0)
	fmt.Println("Solution num = ", nSolution)
	fmt.Println("elapsed time = ", time.Since(start_time))
}
