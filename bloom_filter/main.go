package main

import (
	"bufio"
	"fmt"
	"math"
	"math/big"
	"os"
	"strings"

	"github.com/spaolacci/murmur3"
)

const (
	e = 1e-6 // FPR
	N = 10_00_000
)

var M, K int

var bits big.Int

func hashLocations(s string) []int {
	h1, h2 := murmur3.Sum128([]byte(s))
	locs := make([]int, K)
	mod := uint64(M)
	for i := range K {
		k := (h1 + uint64(i)*h2) % mod
		locs[i] = int(k)
	}
	return locs
}

func setup(fileName string) {
	M = int(-2.08 * N * math.Log(e))
	K = int(-1 * math.Log(e) / math.Log(2))

	file, err := os.Open(fileName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening file %s: %v\n", fileName, err)
		os.Exit(1)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	scanner.Buffer(make([]byte, 1024), 1024*1024)

	urlsCount := 0
	for scanner.Scan() {
		url := strings.TrimSpace(scanner.Text())
		if url != "" {
			urlsCount++
			locs := hashLocations(url)
			for i := range K {
				bits.SetBit(&bits, locs[i], 1)
			}
		}
	}
	if err := scanner.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "Error reading file %s: %v\n", fileName, err)
		os.Exit(1)
	}

	fmt.Printf("Loaded %d urls\n", urlsCount)
}

func isProbablyPresent(url string) bool {
	url = strings.TrimSpace(url)
	locs := hashLocations(url)
	for i := range K {
		if bits.Bit(locs[i]) == 0 {
			return false
		}
	}
	return true
}

func main() {
	fileName := "urls.txt"
	setup(fileName)

	scanner := bufio.NewScanner(os.Stdin)
	scanner.Buffer(make([]byte, 1024), 1024*1024)
	fmt.Print("Query: > ")
	for scanner.Scan() {
		text := strings.TrimSpace(scanner.Text())
		if text == "" {
			continue
		}
		if text == "quit" {
			break
		}
		ans := isProbablyPresent(text)
		if ans {
			fmt.Printf("%v is probably present\n", text)
		} else {
			fmt.Printf("%v is not present\n", text)
		}
		fmt.Print("Query: > ")
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintf(os.Stderr, "Input error: %v\n", err)
	}
}
