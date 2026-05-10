package main

import (
	"fmt"
	"strings"
	"testing"
)

func TestConstructAST(t *testing.T) {
	tests := []struct {
		name string
		md   string
		want Node
	}{
		{
			name: "empty document",
			md:   "",
			want: doc(0),
		},
		{
			name: "single paragraph",
			md:   "hello world",
			want: doc(11, node(Paragraph, Span{}, text(0, 11))),
		},
		{
			name: "single h1 heading",
			md:   "#  hello",
			want: doc(8, node(H1, Span{}, text(3, 8))),
		},
		{
			name: "paragraph with h1 heading",
			md:   "para \n # hello",
			want: doc(14, node(Paragraph, Span{}, text(0, 4)), node(H1, Span{}, text(9, 14))),
		},
		{
			name: "two h1 heading",
			md:   "#  hello \n # hello2",
			want: doc(20, node(H1, Span{}, text(3, 8)), node(H1, Span{}, text(13, 19))),
		},
		{
			name: "single h3 and h6 headings",
			md:   "### h3\n###### h6",
			want: doc(16,
				node(H3, Span{0, 6}, text(4, 6)),
				node(H6, Span{7, 16}, text(14, 16)),
			),
		},
		{
			name: "single emphasis",
			md:   "*em*",
			want: doc(4, node(Paragraph, Span{}, node(Emphasis, Span{0, 4}, text(1, 3)))),
		},
		{
			name: "single strong",
			md:   "**bold**",
			want: doc(8, node(Paragraph, Span{}, node(Strong, Span{0, 8}, text(2, 6)))),
		},
		{
			name: "single strikethrough",
			md:   "~~old~~",
			want: doc(7, node(Paragraph, Span{}, node(StrikeThrough, Span{0, 7}, text(2, 5)))),
		},
		{
			name: "single inline code",
			md:   "`x`",
			want: doc(3, node(Paragraph, Span{}, node(Code, Span{0, 3}, text(1, 2)))),
		},
		{
			name: "single inline link",
			md:   "[site](https://x.test \"title\")",
			want: doc(30, node(Paragraph, Span{}, node(InlineLink, Span{0, 30},
				node(InlineLinkText, Span{}, text(1, 5)),
				node(InlineLinkDestination, Span{}, text(7, 21)),
				node(InlineLinkTitle, Span{}, text(23, 28)),
			))),
		},
		{
			name: "single inline image",
			md:   "![alt](img.png \"pic\")",
			want: doc(21, node(Paragraph, Span{0, 21}, node(InlineImage, Span{0, 21},
				node(InlineLinkText, Span{}, text(2, 5)),
				node(InlineLinkDestination, Span{}, text(7, 14)),
				node(InlineLinkTitle, Span{}, text(16, 19)),
			))),
		},
		{
			name: "soft and hard breaks",
			md:   "a\nb  \nc",
			want: doc(7, node(Paragraph, Span{0, 7},
				text(0, 1),
				node(SoftBreak, Span{}),
				text(2, 3),
				node(HardBreak, Span{}),
				text(6, 7),
			)),
		},
		{
			name: "single thematic break",
			md:   "---",
			want: doc(3, node(ThematicBreak, Span{})),
		},
		{
			name: "single blockquote",
			md:   "> quote",
			want: doc(7, node(BlockQuote, Span{}, node(Paragraph, Span{}, text(2, 7)))),
		},
		// {
		// 	name: "heading with emphasis",
		// 	md:   "# hi *em*",
		// 	want: doc(9, node(H1, Span{}, text(2, 5), node(Emphasis, Span{}, text(6, 8)))),
		// },
		// {
		// 	name: "paragraph with strong and code",
		// 	md:   "hello **bold** and `x`",
		// 	want: doc(22, node(Paragraph, Span{},
		// 		text(0, 6),
		// 		node(Strong, Span{}, text(8, 12)),
		// 		text(14, 19),
		// 		node(Code, Span{}, text(20, 21)),
		// 	)),
		// },
		{
			name: "h1 and paragraph",
			md:   "# title\nbody",
			want: doc(12,
				node(H1, Span{}, text(2, 7)),
				node(Paragraph, Span{}, text(8, 12)),
			),
		},
	}

	for i, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			mdR := []rune(tt.md)
			got := constructAST(mdR, 0, len(mdR))
			var sbg, sbw strings.Builder
			exportHTML(&tt.want, mdR, &sbw)
			exportHTML(&got, mdR, &sbg)
			fmt.Printf("Case %d: Got %s \n        Want %s\n\n", i, sbg.String(), sbw.String())
			assertNode(t, &got, &tt.want)
		})
	}
}

func doc(end int, children ...*Node) Node {
	return Node{kind: Document, span: Span{0, end}, children: children}
}

func assertNode(t *testing.T, got, want *Node) {
	t.Helper()

	if got.kind != want.kind || (want.kind == Text && got.span != want.span) || len(got.children) != len(want.children) {
		t.Fatalf("got kind=%v span=%v children=%d, want kind=%v span=%v children=%d",
			got.kind, got.span, len(got.children), want.kind, want.span, len(want.children))
	}

	for i := range want.children {
		assertNode(t, got.children[i], want.children[i])
	}
}

// textPos := 0
// nextText := func(s string) *Node {
// 	idx := strings.Index(dataText[textPos:], s)
// 	if idx < 0 {
// 		panic("missing dummy markdown text")
// 	}
// 	startByte := textPos + idx
// 	start := len([]rune(data[:startByte]))
// 	textPos = startByte + len(s)
// 	return &Node{kind: Text, span: Span{start, start + len([]rune(s))}}
// }
//
// emphasis := func(s string) *Node {
// 	return &Node{kind: Emphasis, children: []*Node{nextText(s)}}
// }
//
// strong := func(s string) *Node {
// 	return &Node{kind: Strong, children: []*Node{nextText(s)}}
// }
//
// strike := func(s string) *Node {
// 	return &Node{kind: StrikeThrough, children: []*Node{nextText(s)}}
// }
//
// code := func(s string) *Node {
// 	return &Node{kind: Code, children: []*Node{nextText(s)}}
// }
//
// softBreak := Node{
// 	kind: SoftBreak,
// }
//
// h1 := Node{
// 	kind: H1,
// 	children: []*Node{
// 		nextText("heading1 "),
// 		emphasis("italic"),
// 		nextText(" and "),
// 		strike("strike"),
// 	},
// }
//
// h2 := Node{
// 	kind:     H2,
// 	children: []*Node{nextText("heading2")},
// }
//
// para := Node{
// 	kind: Paragraph,
// 	children: []*Node{
// 		nextText("Paragraph with "),
// 		emphasis("emphasis"),
// 		nextText(", "),
// 		strong("bold"),
// 		nextText(", "),
// 		code("code"),
// 		nextText(", and"),
// 		{kind: SoftBreak},
// 		nextText("soft break then"),
// 		{kind: HardBreak},
// 		nextText("hard break."),
// 	},
// }
//
// list := Node{
// 	kind: List,
// 	children: []*Node{
// 		{kind: ListItem, children: []*Node{nextText("first item")}},
// 		{kind: ListItem, children: []*Node{nextText("second item with "), emphasis("italic")}},
// 	},
// }
//
// quotePara := nextText("quoted paragraph")
//
// linkText := Node{
// 	kind:     InlineLinkText,
// 	children: []*Node{nextText("text")},
// }
//
// linkDest := Node{
// 	kind:     InlineLinkDestination,
// 	children: []*Node{nextText("url")},
// }
//
// linkTitle := Node{
// 	kind:     InlineLinkTitle,
// 	children: []*Node{nextText("title")},
// }
//
// link := Node{
// 	kind:     InlineLink,
// 	children: []*Node{&linkDest, &linkTitle, &linkText},
// }
//
// imgLinkText := Node{
// 	kind:     InlineLinkText,
// 	children: []*Node{nextText("alt_text")},
// }
//
// imgLinkDest := Node{
// 	kind:     InlineLinkDestination,
// 	children: []*Node{nextText("image.jpg")},
// }
//
// imgLinkTitle := Node{
// 	kind:     InlineLinkTitle,
// 	children: []*Node{nextText("title")},
// }
//
// imgLink := Node{
// 	kind:     InlineImage,
// 	children: []*Node{&imgLinkDest, &imgLinkTitle, &imgLinkText},
// }
//
// quote := Node{
// 	kind: BlockQuote,
// 	children: []*Node{
// 		{kind: Paragraph, children: []*Node{
// 			quotePara, &softBreak, &link, &softBreak, &imgLink,
// 		}},
// 	},
// }
//
// document := Node{
// 	kind:     Document,
// 	span:     Span{0, len(data)},
// 	children: []*Node{&h1, &h2, &para, &list, {kind: ThematicBreak}, &quote},
// }
// return document
