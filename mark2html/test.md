# Heading 1
## Heading 2
### Heading 3


Paragraph with *emphasis*, **strong**, and ***both***.

Paragraph with _underscores_ and __double underscores__.

Paragraph with mixed emphasis: **bold and *nested italic* text**.

---

## Links and Images

Inline link: [example](https://example.com)

Reference link: [ref][1]

[1]: https://example.org "Example Title"

Autolink: <https://example.net>

Image:
![alt text](https://picsum.photos/200)

---

## Code

Inline code: `fmt.Println("hello")`

Fenced code block:

```go
package main

func main() {
    println("hello")
}
```

Indented code block:

    line one
    line two

---

## Lists

Unordered list:
- item 1
- item 2
  - nested item
  - another nested

Ordered list:
1. first
2. second
   1. nested ordered
   2. nested ordered

Mixed:
- item
  1. nested ordered
  2. nested ordered

---

## Blockquotes

> simple quote

> nested quote
>> double nested

> quote with list
> - item 1
> - item 2

---

## Horizontal Rule

---

***

___

---

## Escaping

\*not italic\*  
\# not a heading  
\\ backslash

---

## Edge-ish Cases

Empty lines:

Paragraph one.


Paragraph two.

List tight vs loose:

- tight 1
- tight 2

- loose 1

- loose 2

---

## HTML Inline

<span>inline HTML</span>

<div>
block HTML
</div>

---

## Hard line breaks

Line with two spaces at end  
Next line

Line with backslash\
Next line

---

## Thematic weirdness

* * *

- - -

_ _ _

---

## Combination

> **bold inside quote**
>
> - list inside quote
> - with `code`

---

End.
