package main

import (
	"fmt"
	"io/fs"
	"math"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"sync"
)

type flags struct {
	caseInsensitive bool
	lineNum         bool
	maxDepth        int
	matchBinary     bool
}

const NumChars = 256 // TODO: just ascii
const MaxConcurrency = 10

type preProcessed struct {
	badSuffix  [NumChars]int
	goodSuffix []int
}

func preProcess(pattern *string, flag flags) (p preProcessed) {
	if flag.caseInsensitive {
		*pattern = strings.ToLower(*pattern)
	}

	p = preProcessed{goodSuffix: buildGoodSuffix(*pattern)}
	for i := range NumChars {
		p.badSuffix[i] = -1
	}
	for i := 0; i < len(*pattern); i++ {
		p.badSuffix[(*pattern)[i]] = i
	}
	return
}

func buildGoodSuffix(pattern string) []int {
	m := len(pattern)
	shift := make([]int, m+1)
	bpos := make([]int, m+1)

	i := m
	j := m + 1
	bpos[i] = j
	for i > 0 {
		for j <= m && pattern[i-1] != pattern[j-1] {
			if shift[j] == 0 {
				shift[j] = j - i
			}
			j = bpos[j]
		}
		i--
		j--
		bpos[i] = j
	}

	j = bpos[0]
	for i := range m {
		if shift[i] == 0 {
			shift[i] = j
		}
		if i == j {
			j = bpos[j]
		}
	}

	return shift
}

func toLower(c byte, i bool) byte {
	if i && c >= 'A' && c <= 'Z' {
		return c + 'a' - 'A'
	}
	return c
}

func searchInternal(pattern, path string, flag flags, preProcessed *preProcessed) {
	// fmt.Println("searching", path)
	content, err := os.ReadFile(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s: %s\n", path, err.Error())
		return
	}

	m, n := len(pattern), len(content)
	if m > n {
		// fmt.Printf("%s: No match, file shorter than pattern", path)
		return
	}

	ins := flag.caseInsensitive
	ks := []int{}
	nullByteP := false // if nullByte present consider binary file
	for s := 0; s <= n-m; {
		j := m - 1
		if content[s] == 0 && !flag.matchBinary {
			nullByteP = true
		}
		for j >= 0 && pattern[j] == toLower(content[s+j], ins) {
			j--
		}

		if j < 0 {
			if nullByteP {
				fmt.Fprintf(os.Stderr, "gogrep: %s: binary file matches\n", path)
				return
			}
			ks = append(ks, s)
			shift := max(preProcessed.goodSuffix[0], 1)
			s += shift
			continue
		}

		badChar := toLower(content[s+j], ins)
		badCharShift := max(j-preProcessed.badSuffix[badChar], 1)

		goodSuffixShift := max(preProcessed.goodSuffix[j+1], 1)

		if badCharShift > goodSuffixShift {
			s += badCharShift
		} else {
			s += goodSuffixShift
		}
	}

	if len(ks) == 0 {
		return
	}

	lineNo, lastLineNo := 1, -1
	lineNoI := 0
	kIndex := 0

	for i, c := range content {
		if kIndex >= len(ks) {
			return
		}
		if c == '\n' {
			lineNoI = i + 1
			lineNo++
		}
		if i == ks[kIndex] {
			if lastLineNo != lineNo {
				x := lineNoI
				for x < n {
					if content[x] == '\n' {
						break
					}
					x++
				}
				if flag.lineNum {
					fmt.Printf("%s:%d:%s\n", path, lineNo, content[lineNoI:x])
				} else {
					fmt.Printf("%s:%s\n", path, content[lineNoI:x])
				}
				lastLineNo = lineNo
			}
			kIndex++
		}
	}
}

func search(pattern, path string, flag flags, preProcessed *preProcessed, wg *sync.WaitGroup, sem chan struct{}) {
	info, err := os.Stat(path)
	if err != nil {
		fmt.Fprintf(os.Stderr, "%s: %s\n", path, err.Error())
		return
	}

	if info.IsDir() {
		if flag.maxDepth != 0 {
			fs.WalkDir(os.DirFS(path), ".", func(path1 string, d fs.DirEntry, err error) error {
				if err != nil {
					fmt.Fprintf(os.Stderr, "%s: %s\n", path1, err.Error())
					return err
				}

				if path1 == "." || path1 == ".." {
					return nil
				}

				if d.IsDir() {
					level := strings.Count(path1, string(os.PathSeparator)) + 1
					if level >= flag.maxDepth {
						return fs.SkipDir
					}
					return nil
				}

				wg.Add(1)
				sem <- struct{}{}
				go func() {
					defer wg.Done()
					defer func() { <-sem }()
					searchInternal(pattern, filepath.Join(path, path1), flag, preProcessed)
				}()
				return nil
			})
		}
	} else {
		searchInternal(pattern, path, flag, preProcessed)
	}
}

func printUsage(bin string) {
	fmt.Fprintf(os.Stderr, "Usage: %s PATTERN [OPTIONS] PATH...\n", bin)
	fmt.Fprintf(os.Stderr, "Options:\n")
	fmt.Fprintf(os.Stderr, "  -n      show line numbers\n")
	fmt.Fprintf(os.Stderr, "  -i      case-insensitive match (ASCII)\n")
	fmt.Fprintf(os.Stderr, "  -a      treat binary files as text\n")
	fmt.Fprintf(os.Stderr, "  -D=N    max directory depth (0=only file paths, 1=current dir files)\n")
}

func main() {
	args := os.Args

	if len(args) < 3 {
		printUsage(args[0])
		os.Exit(1)
	}

	paths := []string{}
	pattern := args[1]
	flag := flags{false, false, math.MaxInt, false}

	r, _ := regexp.Compile("^-D=([0-9]+)$")
	for _, v := range args[2:] {
		switch v {
		case "-n":
			flag.lineNum = true
		case "-i":
			flag.caseInsensitive = true
		case "-a":
			flag.matchBinary = true
		default:
			if r.MatchString(v) {
				d, err := strconv.Atoi(r.FindStringSubmatch(v)[1])
				if err == nil {
					flag.maxDepth = d
					continue
				}
			}
			paths = append(paths, v)

		}
	}

	if len(paths) == 0 {
		printUsage(args[0])
		fmt.Fprintf(os.Stderr, "No path provided\n")
		os.Exit(1)
	}

	if len(pattern) == 0 {
		printUsage(args[0])
		fmt.Fprintf(os.Stderr, "Empty pattern provided\n")
		os.Exit(1)
	}

	preProcessed := preProcess(&pattern, flag)

	var wg sync.WaitGroup

	sem := make(chan struct{}, MaxConcurrency)
	for _, path := range paths {
		search(pattern, path, flag, &preProcessed, &wg, sem)
	}

	wg.Wait()
}
