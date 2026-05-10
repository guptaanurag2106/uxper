package main

import (
	"strings"
)

type HTMLTag struct {
	start string
	end   string
}

var kind2Html = map[NodeKind]HTMLTag{
	Document:      {"<!doctype html>", ""},
	H1:            {"<h1>", "</h1>"},
	H2:            {"<h2>", "</h2>"},
	H3:            {"<h3>", "</h3>"},
	H4:            {"<h4>", "</h4>"},
	H5:            {"<h5>", "</h5>"},
	H6:            {"<h6>", "</h6>"},
	Paragraph:     {"<p>", "</p>"},
	BlockQuote:    {"<blockquote>", "</blockquote>"},
	List:          {"<ul>", "</ul>"},
	ListItem:      {"<li>", "</li>"},
	CodeBlock:     {"<pre><code>", "</code></pre>"},
	Emphasis:      {"<em>", "</em>"},
	Strong:        {"<strong>", "</strong>"},
	StrikeThrough: {"<s>", "</s>"},
	Code:          {"<code>", "</code>"},
	HardBreak:     {"<br />", ""},
	SoftBreak:     {"\n", ""},
	ThematicBreak: {"<hr />", ""},
	Space:         {" ", ""},
}

// https://html.spec.whatwg.org/entities.json
var htmlEntities = map[rune]string{
	'&':  "&amp;",
	'<':  "&lt;",
	'>':  "&gt;",
	'"':  "&quot;",
	'\'': "&#39;",
	' ':  "&nbsp;",
	'©':  "&copy;",
	'®':  "&reg;",
	'™':  "&trade;",
	'€':  "&euro;",
	'£':  "&pound;",
	'¥':  "&yen;",
	'–':  "&ndash;",
	'—':  "&mdash;",
	'…':  "&hellip;",
	'«':  "&laquo;",
	'»':  "&raquo;",
}

func appendTexttoSb(sb *strings.Builder, data []rune) {
	for _, c := range data {
		if n, ok := htmlEntities[c]; ok {
			sb.WriteString(n)
		} else {
			sb.WriteRune(c)
		}
	}
}

func exportHTML(node *Node, data []rune, sb *strings.Builder) {
	if node == nil {
		return
	}

	switch node.kind {
	case Document,
		H1, H2, H3, H4, H5, H6,
		Paragraph,
		BlockQuote,
		List,
		ListItem,
		CodeBlock,
		Emphasis,
		Strong,
		StrikeThrough,
		Code,
		HardBreak,
		SoftBreak,
		ThematicBreak:
		sb.WriteString(kind2Html[node.kind].start)

		for _, child := range node.children {
			exportHTML(child, data, sb)
		}

		sb.WriteString(kind2Html[node.kind].end)

	case Text:
		strOrig := data[node.span.start:node.span.end]
		appendTexttoSb(sb, strOrig)

	case InlineLink, InlineImage:
		var url []rune
		var title []rune
		var text []rune

		for _, child := range node.children {
			textNode := child.children[0]
			switch child.kind {
			case InlineLinkDestination:
				url = data[textNode.span.start:textNode.span.end]
			case InlineLinkTitle:
				title = data[textNode.span.start:textNode.span.end]
			case InlineLinkText:
				text = data[textNode.span.start:textNode.span.end]
			default:
				panic("unreachable inlineLink.child.kind")
			}
		}

		if node.kind == InlineLink {
			sb.WriteString("<a href=\"")
			appendTexttoSb(sb, url)
			sb.WriteString("\" title=\"")
			appendTexttoSb(sb, title)
			sb.WriteString("\">")
			appendTexttoSb(sb, text)
			sb.WriteString("</a>")
		} else {
			sb.WriteString("<img src=\"")
			appendTexttoSb(sb, url)
			sb.WriteString("\" alt=\"")
			appendTexttoSb(sb, text)
			sb.WriteString("\" title=\"")
			appendTexttoSb(sb, title)
			sb.WriteString("\" />")
		}

	case Space:
		sb.WriteRune(' ')

	default:
		panic("unreachable NodeKind")
	}
}
