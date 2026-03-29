//go:build helper

package main

import (
	"fmt"
	"math/rand/v2"
	"os"
)

var tlds = [20]string{
	".com",
	".org",
	".tv",
	".net",
	".io",
	".gov",
	".edu",
	".co",
	".us",
	".uk",
	".de",
	".fr",
	".jp",
	".cn",
	".ru",
	".br",
	".in",
	".au",
	".it",
	".nl",
}

const (
	N        = 10_00_000
	fileName = "urls.txt"
)

func main() {
	f, err := os.Create(fileName)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error opening file %v: %v\n", fileName, err.Error())
		os.Exit(1)
	}

	defer f.Close()

	for _ = range N {
		var length = 5 + rand.IntN(10)
		main := ""
		for _ = range length {
			main += string(rune(97 + rand.IntN(25)))
		}
		url := "www." + main + tlds[rand.IntN(len(tlds))] + "\n"
		f.WriteString(url)
	}
	f.Sync()

	fmt.Printf("Wrote %v urls to %v\n", N, fileName)
}
