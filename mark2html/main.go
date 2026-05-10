package main

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
)

func usage(progName string, outFile io.ReadWriter) {
	fmt.Fprintf(outFile, `Usage:
  %s <input.md> [output.html]

Arguments:
  input.md      Path to the Markdown file
  output.html   Optional output HTML file path
`, progName)
}

func main() {
	args := os.Args
	progName := args[0]
	if len(args) == 1 {
		fmt.Fprintln(os.Stderr, "Error: Missing input file")
		usage(progName, os.Stderr)
		os.Exit(1)
	}

	inFilePath := os.Args[1]
	outFilePath := ""

	if len(args) == 3 {
		outFilePath = os.Args[2]
	} else {
		ext := filepath.Ext(inFilePath)
		outFilePath = strings.TrimSuffix(inFilePath, ext) + ".html"
	}

	mdData, err := os.ReadFile(inFilePath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error reading %s: %s\n", inFilePath, err.Error())
		os.Exit(1)
	}
	mdDataR := []rune(string(mdData))

	document := constructAST(mdDataR, 0, len(mdDataR))

	var sb strings.Builder
	exportHTML(&document, mdDataR, &sb)

	outFileDir := filepath.Dir(outFilePath)
	err = os.MkdirAll(outFileDir, 0o755)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error creating folder %s: %s\n", outFileDir, err.Error())
	}
	os.WriteFile(outFilePath, []byte(sb.String()), 0o644)
}
