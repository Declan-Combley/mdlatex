#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#define STRING_CAP 1000

typedef enum TokenKind {
    // Headers
    HeaderOne,
    HeaderTwo,
    HeaderThree,

    // Token Types
    Text,
    Character,

    // Special Characters
    Pound,
    Tilda,
    Hyphon,
    Star,

    // Types
    Dotpoint,

    // Weird Characters
    Empty,
    NewLine,
    
    Error,
} TokenKind;

typedef struct Position {
    size_t line_no;
    size_t col;
} Position;

typedef struct Token {
    TokenKind kind;
    Position pos;
    char text[STRING_CAP];
} Token;

const char printable_tokens[16][15] = {
    "title",
    "header two",
    "header three",
    "text",
    "character",
    "pound",
    "tilda",
    "hyphon",
    "star",
    "dotpoint",
    "empty",
    "new line",
    "error",
};

TokenKind to_token_kind(char c)
{
    if (c == '#')        { return Pound;   }
    if (c == '-')        { return Hyphon;  }
    if (c == '`')        { return Tilda;   }
    if (c == ' ')        { return Empty;   }
    if (c == '\n')       { return NewLine; }
    return Character;
}

Token tokenize_line(char *line, int line_no)
{
    size_t len = strlen(line);
    TokenKind tmp_tokens_kind[len];
    for (size_t i = 0; i < len; i++) {
        tmp_tokens_kind[i] = to_token_kind(line[i]);
    }

    if (tmp_tokens_kind[0] == NewLine) {
        Token new_line = {
            .kind = NewLine,
            .pos = {
                .line_no = line_no + 1,
                .col = 0,
            },
        };
        
        return new_line;
    }
    
    line[strcspn(line, "\n")] = 0;
    if (tmp_tokens_kind[0] == Pound) {
        Token tmp_header_token = {
            .kind = Error,
            .pos = {
                .line_no = line_no + 1,
                .col = 0,
            },
        };
        strcpy(tmp_header_token.text, line);
        size_t num_of_pounds = 1;
        size_t i = 1;
        while (tmp_tokens_kind[i] == Pound) {
            num_of_pounds++;
            i++;
        }

        char parsed_line[len - num_of_pounds];
        for (size_t i = 1; i < len; i++) {
            parsed_line[i - num_of_pounds] = line[i];
        }
        strcpy(line, parsed_line);
        strcpy(tmp_header_token.text, line);
        
        if (num_of_pounds > 3) {
            Token error = {
                .kind = Error,
                .pos = {
                    .line_no = line_no + 1,
                    .col = 0,
                },
            };
            strcpy(error.text, line);
            return error;
        }
        
        tmp_header_token.kind = num_of_pounds - 1;
        
        return tmp_header_token;
    } else if (tmp_tokens_kind[0] == Hyphon) {
        Token dotpoint = {
            .kind = Dotpoint,
            .pos = {
                .line_no = line_no + 1,
                .col = 0,
            },
        };
        
        char parsed_line[len - 2];
        for (size_t i = 2; i < len; i++) {
            parsed_line[i - 2] = line[i];
        }
        strcpy(line, parsed_line);
        strcpy(dotpoint.text, line);
        
        return dotpoint;
    } else {
        Token text_token = {
            .kind = Text,
            .pos = {
                .line_no = line_no + 1,
                .col = 0,
            },
        };
        strcpy(text_token.text, line);
        return text_token;
    }

    Token error = {
        .kind = Error,
        .pos = {
            .line_no = line_no + 1,
            .col = 0,
        },
    };
    
    return error;
}

size_t no_of_digits(size_t n)
{
    size_t no_of_digits = 0;
    do  {
        n /= 2;
        no_of_digits++;
    } while (n != 0);

    return no_of_digits;
}

void error(Token token, char *message, char *input_file)
{
    fprintf(stderr, "%s:%ld:%ld: error: %s\n", input_file, token.pos.line_no, token.pos.col, message);
    fprintf(stderr, "  %ld | %s\n", token.pos.line_no, token.text);
    fputs("  ", stderr);
    for (size_t i = 0; i <= no_of_digits(token.pos.line_no) - 1; i++) {
        putc(' ', stderr);
    }
    fputs("| ", stderr);
    for (size_t i = 0; i < strlen(token.text); i++) {
        putc('~', stderr);
    }        
    putc('\n', stderr);
    exit(1);
}

int main(int argc, char **argv)
{
    (void) argc;
    assert(argv != NULL);

    char *program = *argv++;
    char *input_file = *argv++;
    if (input_file == NULL) {
        fprintf(stderr, "error: expected an input file\n");
        fprintf(stderr, "usage: %s example.md\n", program);
        exit(1);
    }

    if (strstr(input_file, ".md") == 0) {
        fprintf(stderr, "error: expected a .md file\n");
        fprintf(stderr, "usage: %s example.md\n", program);
        exit(1);
    }

    FILE *f = fopen(input_file, "r");
    if (f == NULL) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(errno);
    }

    int line_len = 0;
    int no_of_lines = 0;
    for (char c = getc(f); c != EOF; c = getc(f)) {
        line_len++;
        if (line_len >= STRING_CAP) {
            fprintf(stderr, "%s:%d:%d error: string is to long, consider adding a new line\n", input_file, no_of_lines + 1, 0);
            exit(1);
        }
        if (c == '\n') {
            no_of_lines++;
            line_len = 0;
        }
    }

    rewind(f);

    Token tokens[no_of_lines + 1];
    int line_counter = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, f)) != -1) {
        tokens[line_counter] = tokenize_line(line, line_counter);
        printf("%s\n", printable_tokens[tokens[line_counter].kind]);
        line_counter++;
    }

    if (fclose(f) != 0) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(errno);
    }

    char *output_file_path = strdup(input_file);
    strtok(output_file_path, ".");
    strcat(output_file_path, ".tex");

    FILE *out = fopen(output_file_path, "w");
    if (out == NULL) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(errno);
    }
    
    int curr = 0;
    if (tokens[curr].kind != HeaderOne) {
        error(tokens[0], "expected a header one title at the beginning of the .md file", input_file);
    }
    
    fputs("\\documentclass[12pt]{article}\n", out); 
    fputs("\\title{", out);
    fputs(tokens[curr].text, out);
    fputs("}\n", out);
    fputs("\\author{Mdlatex}\n", out);
    fputs("\\begin{document}\n", out); 
    fputs("\\maketitle\n", out); 
    curr++;

    if ((tokens[curr].kind != HeaderTwo || tokens[curr].kind != HeaderThree) && tokens[curr].kind != NewLine) {
        char error_text[255];
        printf("%s\n", printable_tokens[tokens[curr].kind]);
        strcpy(error_text, "expected a header two, got ");
        strcat(error_text, printable_tokens[tokens[curr].kind]);
        error(tokens[curr], error_text, input_file);
    }

    curr++;
    while (curr < no_of_lines) {
        Token current = tokens[curr];

        if (current.kind == Error) {
            error(tokens[curr], "unknown error", input_file);
        }
        
        if (current.kind == NewLine) {
            curr++;
            continue;
        }

        if (current.kind == HeaderOne) {
            error(tokens[curr], "can not have more than one title", input_file);
        }
        
        if (current.kind == Text) {
            while (tokens[curr].kind == Text) {
                fputs(tokens[curr].text, out);
                fputc('\n', out);
                curr++;
            }
        }
        
        if (current.kind == HeaderTwo) {
            fputs("\\section{", out);
            fputs(current.text, out);
            fputs("}\n", out);
            curr++;
            while (tokens[curr].kind == Text) {
                fputs(tokens[curr].text, out);
                fputc('\n', out);
                curr++;
            }
        }
        
        if (current.kind == HeaderThree) {
            fputs("\\subsection{", out);
            fputs(current.text, out);
            fputs("}\n", out);
            curr++;
            while (tokens[curr].kind == Text) {
                fputs(tokens[curr].text, out);
                fputc('\n', out);
                curr++;
            }
        }

        if (current.kind == Dotpoint) {
            fputs("\\begin{itemize}\n", out);
            while (tokens[curr].kind == Dotpoint) {
                fputs("  \\item ", out);
                fputs(tokens[curr].text, out);
                fputc('\n', out);
                curr++;
            }
            fputs("\\end{itemize}\n", out);
        }
        curr++;
    }

    fputs("\\end{document}\n", out);
    if (fclose(out) != 0) {
        fprintf(stderr, "error: %s\n", strerror(errno));
        exit(errno);
    } 
    
    return 0;
}
