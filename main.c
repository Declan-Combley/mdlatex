#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

typedef enum TokenKind {
    // Headers
    HeaderOne,
    HeaderTwo,
    HeaderThree,

    // Token Types
    Text,
    Character,
    Pound,
    Tilda,
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
    char text[255];
} Token;

const char printable_tokens[14][11] = {
    "header one",
    "header two",
    "headerThree",
    "text",
    "character",
    "pound",
    "tilda",
    "empty",
    "new line",
    "error",
};

TokenKind to_token_kind(char c)
{
    if (c == '#')        { return Pound;   }
    if (c == '`')        { return Tilda;   }
    if (c == ' ')        { return Empty;   }
    if (c == '\n')       { return NewLine; }
    return Character;
}

Token tokenize_line(char *line, int line_no)
{
    TokenKind tmp_tokens_kind[strlen(line)];
    for (size_t i = 0; i < strlen(line); i++) {
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

        printf("number of pounds: %ld\n", num_of_pounds);

        if (num_of_pounds == 1) {
            strcpy(line, strtok(line, "# "));
        } else if (num_of_pounds == 2) {
            strcpy(line, strtok(line, "## "));
        } else if (num_of_pounds == 3) {
            strcpy(line, strtok(line, "### "));
        } else {
            assert(false && "Unreachable");                
        }
        
        strcpy(tmp_header_token.text, line);
        
        assert(num_of_pounds != 0 && "Unreachable");
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

    int no_of_lines = 0;
    for (char c = getc(f); c != EOF; c = getc(f)) {
        if (c == '\n') {
            no_of_lines++;
        }
    }

    rewind(f);

    Token tokens[no_of_lines];
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
        strcpy(error_text, "expected a header one or two, got ");
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
        
        if (current.kind == Text) {
            while (tokens[curr].kind == Text) {
                fputs(tokens[curr].text, out);
                fputc('\n', out);
                curr++;
            }
        }
        
        if (current.kind == HeaderTwo || current.kind == HeaderThree) {
            fputs("\\section{", out);
            fputs(current.text, out);
            fputs("}\n", out);
            curr++;
            while (tokens[curr].kind == Text) {
                fputs(tokens[curr].text, out);
                fputc('\n', out);
                curr++;
            }
            fputs("\\end{", out);
            fputs(current.text, out);
            fputs("}\n", out);
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
