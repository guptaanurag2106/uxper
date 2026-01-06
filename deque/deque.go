// Implementing Deque
package main

import (
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/gammazero/deque"
)

// all operations are inner <op> outer
const (
	TokenAdd   = "add"
	TokenSub   = "sub"
	TokenMul   = "mul"
	TokenDiv   = "div"
	TokenOr    = "or"
	TokenAnd   = "and"
	TokenShl   = "shl"
	TokenShr   = "shr"
	TokenLt    = "lt"
	TokenGt    = "gt"
	TokenDrop  = "drop"
	TokenDup   = "dup"  // dup! pushes copy of rightmost value (duplicates)
	TokenOver  = "over" // gets the second value and pushes it
	TokenMove  = "move" // move from left to righ vice versa
	TokenJmp   = "jmp"
	TokenJmpIf = "jmpif"
	TokenExit  = "exit"
	TokenPrint = "print"
)

func run(prog []string) {
	var mem deque.Deque[int]
	mem.Grow(len(prog) / 2)

	var labels = make(map[string]int)
	var ip = 0

	for ip < len(prog) {
		var token = prog[ip]
		if token[len(token)-1] == ':' {
			labels[token[:len(token)-1]] = ip
		}
		ip++
	}
	var tracing = false
	if tracing {
		fmt.Printf("labels: %v\n", labels)
	}

	ip = 0
	for ip < len(prog) {
		var token = prog[ip]

		if tracing {
			fmt.Printf("%v <- mem: %v\n", token, mem)
		}

		var left = false

		if token[0] == '!' {
			left = true
			token = token[1:]
		} else if token[len(token)-1] == '!' {
			left = false
			token = token[:len(token)-1]
		} else if token[len(token)-1] == ':' {
			ip++
			continue
		} else {
			if token == "trace" {
				for i := range mem.Len() {
					if mem.At(mem.Len()-i-1) == 1 {
						fmt.Print("*")
					} else if mem.At(mem.Len()-i-1) == 0 {
						fmt.Print(" ")
					}
				}
				fmt.Println()
				ip++
				continue
			}
			fmt.Fprintf(os.Stderr, "Unkown token '%s' at position %d\n", token, ip)
			os.Exit(1)
		}

		var (
			pop  func() int
			push func(int)
		)

		if left {
			pop = mem.PopBack
			push = mem.PushBack
		} else {
			pop = mem.PopFront
			push = mem.PushFront
		}

		switch token {
		case TokenAdd:
			a := pop()
			b := pop()
			push(a + b)
			ip++
		case TokenSub:
			a := pop()
			b := pop()
			push(b - a)
			ip++
		case TokenMul:
			a := pop()
			b := pop()
			push(b * a)
			ip++
		case TokenDiv:
			a := pop()
			b := pop()
			push(b / a)
			ip++
		case TokenOr:
			a := pop()
			b := pop()
			push(a | b)
			ip++
		case TokenAnd:
			a := pop()
			b := pop()
			push(a & b)
			ip++
		case TokenShl:
			a := pop()
			b := pop()
			push(b << a)
			ip++
		case TokenShr:
			a := pop()
			b := pop()
			push(b >> a)
			ip++
		case TokenLt:
			a := pop()
			b := pop()
			if b < a {
				push(0)
			} else {
				push(1)
			}
			ip++
		case TokenGt:
			a := pop()
			b := pop()
			if b > a {
				push(0)
			} else {
				push(1)
			}
			ip++
		case TokenDrop:
			pop()
			ip++
		case TokenDup:
			a := pop()
			push(a)
			push(a)
			ip++
		case TokenOver:
			a := pop()
			b := pop()
			push(b)
			push(a)
			push(b)
			ip++
		case TokenMove:
			a := pop()
			if left {
				mem.PushFront(a)
			} else {
				mem.PushBack(a)
			}
			ip++
		case TokenJmp:
			ip = pop()
		case TokenJmpIf:
			pos := pop()
			cond := pop()
			if cond == 0 {
				ip = pos
			} else {
				ip++
			}
		case TokenExit:
			code := pop()
			os.Exit(code)
		case TokenPrint:
			fmt.Println(pop())
			ip++
		default:
			i, err := strconv.Atoi(token)
			if err != nil {
				pos, ok := labels[token]
				if ok {
					push(pos)
					ip++
				} else {
					fmt.Fprintf(os.Stderr, "Unkown token '%s' at position %d\n", token, ip)
					os.Exit(1)
				}
			} else {
				push(i)
				ip++
			}
		}
	}
}

func main() {
	args := os.Args
	if len(args) != 2 {
		fmt.Fprintf(os.Stderr, "Usage: %s <input_file_name>\n", args[0])
		os.Exit(1)
	}

	fileName := args[1]
	data, err := os.ReadFile(fileName)

	if err != nil {
		fmt.Fprintf(os.Stderr, "ERROR: Could not read %s: %s\n", fileName, err.Error())
		os.Exit(1)
	}

	var prog = []string{}

	for _, lines := range strings.Split(string(data), "\n") {
		for _, word := range strings.Split(string(lines), " ") {
			if len(word) != 0 {
				if word[0] == '#' {
					break
				}
				prog = append(prog, word)
			}
		}
	}
	run(prog)
}
