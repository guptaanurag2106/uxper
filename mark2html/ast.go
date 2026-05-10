package main

type Span struct {
	start int
	end   int
}

type NodeKind int

const (
	Document              NodeKind = iota
	H1                             // #
	H2                             // ##
	H3                             // ###
	H4                             // ####
	H5                             // #####
	H6                             // ######
	Paragraph                      // <p>
	BlockQuote                     // >
	List                           // <ul>
	ListItem                       // -, . 1. etc
	CodeBlock                      // ``` ```
	Emphasis                       // _ _
	Strong                         // * *
	StrikeThrough                  // ~ ~ (not commonmark)
	InlineLinkDestination          // [](...)
	InlineLinkTitle                // [...](<url> ...)
	InlineLinkText                 // [...]()
	InlineLink                     // abstract container, cannot be converted to html
	InlineImage                    // ![]()
	Code                           // ` `
	HardBreak                      // <br />
	SoftBreak                      // \n
	ThematicBreak                  //  ---, ___ etc
	Text                           // just characters
	Space                          // special case (no span) for < > between elements
)

type Node struct {
	kind     NodeKind
	span     Span
	children []*Node
}

func node(kind NodeKind, span Span, children ...*Node) *Node {
	return &Node{kind: kind, span: span, children: children}
}

func text(start, end int) *Node {
	return node(Text, Span{start, end})
}

func paragraph(start, end int, data []rune, code bool) *Node {
	pNode := Node{
		kind:     Paragraph,
		children: []*Node{},
	}
	i := start
	if !code {
		for ; i < end; i++ {
			if data[i] == '\n' || data[i] == '\r' {
				if i >= 2 && data[i-1] == ' ' && data[i-2] == ' ' {
					pNode.children = append(pNode.children, text(start, i-2))
					pNode.children = append(pNode.children, &Node{kind: HardBreak})
					start = i + 1
				} else {
					pNode.children = append(pNode.children, text(start, i))
					start = i + 1
					pNode.children = append(pNode.children, &Node{kind: SoftBreak})
				}
			}
		}
	}
	pNode.children = append(pNode.children, text(start, i))
	return &pNode
}

func getTrimIndex(sstart, send int, data []rune) (int, int) {
	if sstart != -1 && send > sstart {
		for sstart < send && (data[sstart] == ' ' || data[sstart] == '\r' || data[sstart] == '\n') {
			sstart++
		}
		for send > sstart && (data[send-1] == ' ' || data[send-1] == '\r' || data[send-1] == '\n') {
			send--
		}

		if send <= sstart {
			return -1, -1
		}
		return sstart, send
	}
	return -1, -1
}

func getEOL(start int, data []rune, end int) int {
	for start < end && data[start] != '\n' && data[start] != '\r' {
		start++
	}
	for start < end && (data[start] == '\n' || data[start] == '\r') {
		start++
	}
	return start
}

func constructAST(data []rune, start, end int) Node {
	document := Node{
		kind:     Document,
		span:     Span{0, end - start},
		children: []*Node{},
	}
	constructASTImpl(data, start, end, &document, false)

	return document
}

func constructASTImpl(data []rune, start, end int, node *Node, inContainer bool) {
	if end <= start {
		return
	}

	sstart := -1
	send := -1
	var tagStart int

	var c rune
	i := start

	for i < end {
		c = data[i]
		switch c {
		case '\n', '\r':
			// if sstart != -1 {
			// 	sstart, send = getTrimIndex(sstart, i, data)
			// 	if sstart != -1 && send != -1 {
			// 		if inContainer {
			// 			node.children = append(node.children, text(sstart, send))
			// 			sstart = -1
			// 		} else {
			// 			if send < i-1 {
			// 				fmt.Println("asdfasf", string(data[sstart:send]))
			// 				node.children = append(node.children, &Node{kind: HardBreak})
			// 			}
			// 			// node.children = append(node.children, paragraph(sstart, send))
			// 		}
			// 	}
			// }
			i++
		case ' ', '\t':
			i++
		case '#':
			hcount := 0
			for i < end && data[i] == '#' {
				hcount++
				i++
			}

			if i < end && data[i] == ' ' && hcount <= 6 {
				tagStart = i
				i = getEOL(i, data, end)
				hNode := Node{
					kind:     htags[hcount],
					children: []*Node{},
				}
				if sstart != -1 {
					sstart, send = getTrimIndex(sstart, tagStart-hcount-1, data)
					if sstart != -1 && send != -1 {
						node.children = append(node.children, paragraph(sstart, send, data, false))
					}
					sstart = -1
				}
				constructASTImpl(data, tagStart, i, &hNode, true)
				node.children = append(node.children, &hNode)
			} else {
				if sstart == -1 {
					sstart = i
				}
			}
		case '*', '_':
			if i < end-1 {
				if data[i+1] == c {
					if i < end-2 && data[i+2] == c {
						node.children = append(node.children, &Node{kind: ThematicBreak})
						i = getEOL(i, data, end)
					} else {
						i += 2
						tagStart = i
						for i < end && data[i] != '*' {
							i++
						}
						iEOL := getEOL(i, data, end)
						if i >= iEOL {
							i = iEOL
							if sstart != -1 {
								sstart = tagStart - 3
							}
						} else {
							sNode := Node{
								kind:     Strong,
								children: []*Node{},
							}
							pNode := Node{
								kind:     Paragraph,
								children: []*Node{&sNode},
							}
							if sstart != -1 {
								sstart, send = getTrimIndex(sstart, tagStart-3, data)
								if sstart != -1 && send != -1 {
									node.children = append(node.children, paragraph(sstart, send, data, false))
								}
								sstart = -1
							}
							constructASTImpl(data, tagStart, i, &sNode, true)
							node.children = append(node.children, &pNode)
							i += 2 // skip the 2 stars
						}
					}
				} else {
					i += 1
					tagStart = i
					for i < end && data[i] != c {
						i++
					}
					iEOL := getEOL(i, data, end)
					if i >= iEOL {
						i = iEOL
						if sstart != -1 {
							sstart = tagStart - 2
						}
					} else {
						sNode := Node{
							kind:     Emphasis,
							children: []*Node{},
						}
						pNode := Node{
							kind:     Paragraph,
							children: []*Node{&sNode},
						}
						if sstart != -1 {
							sstart, send = getTrimIndex(sstart, tagStart-2, data)
							if sstart != -1 && send != -1 {
								node.children = append(node.children, paragraph(sstart, send, data, false))
							}
							sstart = -1
						}
						constructASTImpl(data, tagStart, i, &sNode, true)
						node.children = append(node.children, &pNode)
						i++ // skip the star
					}
				}
			} else {
				if sstart == -1 {
					sstart = i
				}
				i++
			}
		case '-':
			if i < end-2 && data[i+1] == '-' && data[i+2] == '-' {
				i = getEOL(i, data, end)
				node.children = append(node.children, &Node{kind: ThematicBreak})
			} else {
				if sstart == -1 {
					sstart = i
				}
				i++
			}
		case '~':
			if i < end-1 && data[i+1] == '~' {
				i += 2
				tagStart = i
				for i < end && data[i] != c {
					i++
				}
				iEOL := getEOL(i, data, end)
				if i >= iEOL {
					i = iEOL
					if sstart != -1 {
						sstart = tagStart - 3
					}
				} else {
					sNode := Node{
						kind:     StrikeThrough,
						children: []*Node{},
					}
					pNode := Node{
						kind:     Paragraph,
						children: []*Node{&sNode},
					}
					if sstart != -1 {
						sstart, send = getTrimIndex(sstart, tagStart-2, data)
						if sstart != -1 && send != -1 {
							node.children = append(node.children, paragraph(sstart, send, data, false))
						}
						sstart = -1
					}
					constructASTImpl(data, tagStart, i, &sNode, true)
					node.children = append(node.children, &pNode)
					i += 2 // skip the ~~
				}
			} else {
				if sstart == -1 {
					sstart = i
				}
				i++
			}
		case '`':
			i++
			tagStart = i
			for i < end && data[i] == c {
				i++
			}

			for i < end && data[i] != c {
				i++
			}
			iEOL := getEOL(i, data, end)
			if i >= iEOL {
				i = iEOL
				if sstart != -1 {
					sstart = tagStart - 3
				}
			} else {
				sNode := Node{
					kind:     Code,
					children: []*Node{},
				}
				pNode := Node{
					kind:     Paragraph,
					children: []*Node{&sNode},
				}
				if sstart != -1 {
					sstart, send = getTrimIndex(sstart, tagStart-2, data)
					if sstart != -1 && send != -1 {
						node.children = append(node.children, paragraph(sstart, send, data, false))
					}
					sstart = -1
				}
				constructASTImpl(data, tagStart, i, &sNode, true)
				node.children = append(node.children, &pNode)
				for i < end && data[i] == c {
					i += 1 // skip the `
				}
			}
		case '>':
			i++
			bNode := Node{
				kind:     BlockQuote,
				children: []*Node{},
			}
			constructASTImpl(data, i, end, &bNode, false)
			node.children = append(node.children, &bNode)
			i = getEOL(i, data, end)
		case '[', '!':
			if c == '!' {
				if i >= end-1 || data[i+1] != '[' {
					if sstart == -1 {
						sstart = i
						i++
					}
					continue
				} else {
					i++
				}
			}
			iEOL := getEOL(i, data, end)
			temp := i
			for ; temp < iEOL; temp++ {
				if data[temp] == ']' && temp < iEOL-1 && data[temp+1] == '(' {
					break
				}
			}
			if temp == iEOL {
				if sstart == -1 {
					sstart = i + 1
				}
				i++
			} else {
				// parse [text](url "title")
				contentStart := temp + 2 // after ](
				parenEnd := contentStart
				urlEnd := contentStart
				titleStart, titleEnd := -1, -1

				for parenEnd < iEOL {
					ch := data[parenEnd]
					if ch == ' ' || ch == '\t' {
						if urlEnd == contentStart {
							urlEnd = parenEnd
						}
						t := parenEnd + 1
						for t < iEOL && (data[t] == ' ' || data[t] == '\t') {
							t++
						}
						if t < iEOL {
							q := data[t]
							if q == '"' || q == '\'' {
								titleStart = t + 1
								titleEnd = titleStart
								t++
								for t < iEOL && data[t] != q {
									if data[t] == '\\' && t+1 < iEOL {
										titleEnd += 2
										t += 2
									} else {
										titleEnd++
										t++
									}
								}
								parenEnd = t + 1
								for parenEnd < iEOL && data[parenEnd] != ')' {
									parenEnd++
								}
								break
							}
							parenEnd = t
							continue
						}
					}
					if ch == ')' {
						if urlEnd == contentStart {
							urlEnd = parenEnd
						}
						break
					}
					parenEnd++
				}

				if parenEnd >= iEOL || data[parenEnd] != ')' {
					if sstart == -1 {
						sstart = i + 1
					}
					i++
				} else {
					nodeKind := InlineLink
					if c == '!' {
						nodeKind = InlineImage
					}
					linkNode := Node{
						kind: nodeKind,
						span: Span{i, parenEnd + 1},
						children: []*Node{
							{kind: InlineLinkText, children: []*Node{text(i+1, temp)}},
							{kind: InlineLinkDestination, children: []*Node{text(contentStart, urlEnd)}},
						},
					}
					if titleStart != -1 && titleEnd != -1 {
						linkNode.children = append(linkNode.children,
							&Node{kind: InlineLinkTitle, children: []*Node{text(titleStart, titleEnd)}})
					}
					paraNode := Node{
						kind:     Paragraph,
						span:     Span{i, parenEnd + 1},
						children: []*Node{&linkNode},
					}
					node.children = append(node.children, &paraNode)
					i = parenEnd + 1
				}
			}

		default:
			if sstart == -1 {
				sstart = i
			}
			i++
		}
	}

	sstart, send = getTrimIndex(sstart, end, data)
	if sstart != -1 && send != -1 {
		if inContainer {
			node.children = append(node.children, text(sstart, send))
		} else {
			node.children = append(node.children, paragraph(sstart, send, data, false))
		}
	}
}

// https://spec.commonmark.org/0.31.2/#ascii-punctuation-character
func isPunctuation(c rune) bool {
	return (c >= 0x21 && c <= 0x2F) || (c >= 0x3A && c <= 0x40) ||
		(c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7E)
}

var htags = map[int]NodeKind{
	1: H1,
	2: H2,
	3: H3,
	4: H4,
	5: H5,
	6: H6,
}
