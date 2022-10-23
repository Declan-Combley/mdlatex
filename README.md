# Mdlatex

A simple way of converting your markdown files into pdfs offline using [latex](https://www.latex-project.org/).

## Goals

- [x] Simple conversion between markdown and latex
- [ ] Allow for highliting, bolding, underlining and italics
- [x] Dotpoints, Numbered Questions
- [ ] Blocks such as math, code, etc..

## Quickstart

```shell
$ make && ./ml example.md && make tex
```

### Usage

To run the code all you will need is the [pdflatex](https://www.tug.org/applications/pdftex/) compiler installed, and a C compiler of your choice

Build out the binary
```shell
$ make
```

And run it with a markdown file to produce the output tex file
```shell
$ ./ml example.md
```

And finally run that output tex file through [pdflatex](https://www/tug.org/applications/pdftex), the output file should be named after the input file.
```shell
$ pdflatex output.tex
```
