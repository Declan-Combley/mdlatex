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
    Number,
    UnknownBlock,
    MathBlock,
    EndBlock,

    // Special Characters
    Pound,
    Tilda,
    Hyphon,
    Star,
    Dot,
    Chevron,
    Equals,

    // Types
    Dotpoint,
    NumberedList,

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
    char error_text[255];
} Token;

const char printable_tokens[][15] = {
    "title",
    "header two",
    "header three",
    "text",
    "character",
    "number",
    "unknown block",
    "math block",
    "end block",
    "pound",
    "tilda",
    "hyphon",
    "star",
    "dot",
    "chevron",
    "equals",
    "dotpoint",
    "numbered list",
    "empty",
    "new line",
    "error",
};

TokenKind to_token_kind(char c)
{
    if (c == '#')        { return Pound;   }
    if (c == '=')        { return Equals;  }
    if (c == '*')        { return Star;    }
    if (c == '^')        { return Chevron; }
    if (c == '.')        { return Dot;     }
    if (c == '-')        { return Hyphon;  }
    if (c == '`')        { return Tilda;   }
    if (c == ' ')        { return Empty;   }
    if (c == '\n')       { return NewLine; }
    if (isdigit(c))      { return Number;  }
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
    if (tmp_tokens_kind[0] == Tilda && tmp_tokens_kind[1] == Tilda && tmp_tokens_kind[2] == Tilda) {
        Token tmp_block_token = {
            .kind = EndBlock,
            .pos = {
                .line_no = line_no + 1,
                .col = 0,
            },
        };

        strcpy(tmp_block_token.text, line);
        if (tmp_tokens_kind[3] != Character) {
            tmp_block_token.kind = EndBlock;
        } else {
            char parsed_line[strlen(line) - 4];
            for (size_t i = 3; i <= strlen(line); i++) {
                parsed_line[i - 3] = line[i];
            }
            parsed_line[strlen(line)] = '\0';
            if (strcmp(parsed_line, "math") == 0) {
                tmp_block_token.kind = MathBlock;
            } else {
                tmp_block_token.kind = UnknownBlock;
            }
        }

        return tmp_block_token;
    }

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
        for (size_t i = num_of_pounds; i < len; i++) {
            parsed_line[(i - num_of_pounds) - 1] = line[i];
        }
        strcpy(tmp_header_token.text, parsed_line);

        if (num_of_pounds > 3) {
            Token error = {
                .kind = Error,
                .pos = {
                    .line_no = line_no + 1,
                    .col = 0,
                },
            };
            strcpy(error.text, line);
            strcpy(error.error_text, "cannnot have a header number greater than three");
            return error;
        }

        tmp_header_token.kind = num_of_pounds - 1; // This only works because of the order of the enums
        if (tmp_tokens_kind[num_of_pounds] != Empty) {
            strcpy(tmp_header_token.error_text, "expected a space between the header type and header name");
            strcpy(tmp_header_token.text, line);
            tmp_header_token.kind = Error;
            return tmp_header_token;
        }

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
        strcpy(dotpoint.text, parsed_line);

        return dotpoint;
    } else if (tmp_tokens_kind[0] == Number && tmp_tokens_kind[1] == Dot && tmp_tokens_kind[2] == Empty) {
        Token numbered_list_token = {
            .kind = NumberedList,
            .pos = {
                .line_no = line_no + 1,
                .col = 0,
            },
        };
        char parsed_line[len - 3];
        for (size_t i = 3; i < len; i++) {
            parsed_line[i - 3] = line[i];
        }
        strcpy(numbered_list_token.text, parsed_line);

        return numbered_list_token;
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

size_t number_of_digits(size_t n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    if (n < 1000) return 3;
    if (n < 10000) return 4;
    if (n < 100000) return 5;
    if (n < 1000000) return 6;
    if (n < 10000000) return 7;
    if (n < 100000000) return 8;
    if (n < 1000000000) return 9;
    return 10;
}

char *get_string_between_delim(char *string, size_t *delim_index, char *delim)
{
    char *parsed_string = malloc((strlen(string) - (sizeof(char) * *delim_index)) * sizeof(char));
    *delim_index += strlen(delim);
    size_t parsed_string_index = 0;
    for (size_t i = *delim_index; i < strlen(string) - 1; i++) {
        for (size_t tmp_delim_index = 0; string[i] == delim[tmp_delim_index]; tmp_delim_index++) {
            if (tmp_delim_index == strlen(delim) - 1) {
                parsed_string[parsed_string_index] = '\0';
                *delim_index += parsed_string_index + strlen(delim) - 1;
                return parsed_string;
            }
        }
        parsed_string[parsed_string_index] = string[i];
        parsed_string_index++;
    }
    return NULL;
}

void error(Token token, char *message, char *input_file)
{
    fprintf(stderr, "%s:%ld:%ld: error: %s\n", input_file, token.pos.line_no, token.pos.col, message);
    fprintf(stderr, " %ld | %s\n ", token.pos.line_no, token.text);
    for (size_t i = 1; i <= number_of_digits(token.pos.line_no) + 1; i++) {
        putc(' ', stderr);
    }
    fputs("| ", stderr);
    for (size_t i = 1; i <= strlen(token.text); i++) {
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
        //printf("%s\n", printable_tokens[tokens[line_counter].kind]);
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
    fputs("\\usepackage{amsmath}\n", out);
    fprintf(out, "\\title{%s}\n", tokens[curr].text);
    fputs("\\author{Mdlatex}\n", out);
    fputs("\\begin{document}\n", out);
    fputs("\\maketitle\n", out);
    curr++;

    if ((tokens[curr].kind != HeaderTwo || tokens[curr].kind != HeaderThree) && tokens[curr].kind != NewLine) {
        char error_text[255];
        strcpy(error_text, "expected a header two, got ");
        strcat(error_text, printable_tokens[tokens[curr].kind]);
        error(tokens[curr], error_text, input_file);
    }

    curr++;
    while (curr < no_of_lines) {
        Token current = tokens[curr];

        if (current.kind == Error) {
            error(tokens[curr], tokens[curr].error_text, input_file);
        }

        if (current.kind == NewLine) {
            curr++;
            fputs("\\newline \\newline\n", out);
            continue;
        }

        if (current.kind == HeaderOne) {
            error(tokens[curr], "can not have more than one title", input_file);
        }

        if (current.kind == Text) {
            while (tokens[curr].kind == Text) {
                for (size_t i = 0; i < strlen(tokens[curr].text); i++) {
                    if (to_token_kind(tokens[curr].text[i]) == Star && to_token_kind(tokens[curr].text[i + 1]) == Star) {
                        char *bold_text = get_string_between_delim(tokens[curr].text, &i, "**");
                        if (bold_text == NULL) {
                            error(tokens[curr], "expected a closing **", input_file);
                        }
                        fprintf(out, "\\textbf{%s}", bold_text);
                        free(bold_text);
                    } else {
                        putc(tokens[curr].text[i], out);
                    }
                }
                putc('\n', out);
                curr++;
            }
            continue;
        }

        if (current.kind == HeaderTwo) {
            fprintf(out, "\\section{");
            for (size_t i = 0; i < strlen(tokens[curr].text); i++) {
                if (to_token_kind(tokens[curr].text[i]) == Star && to_token_kind(tokens[curr].text[i + 1]) == Star) {
                    char *bold_text = get_string_between_delim(tokens[curr].text, &i, "**");
                    if (bold_text == NULL) {
                        error(tokens[curr], "expected a closing **", input_file);
                    }
                    fprintf(out, "\\textbf{%s}", bold_text);
                    free(bold_text);
                } else {
                    putc(tokens[curr].text[i], out);
                }
            }
            fputs("}\n", out);
            curr++;
            continue;
        }

        if (current.kind == HeaderThree) {
            fprintf(out, "\\subsection{");
            for (size_t i = 0; i < strlen(tokens[curr].text); i++) {
                if (to_token_kind(tokens[curr].text[i]) == Star && to_token_kind(tokens[curr].text[i + 1]) == Star) {
                    char *bold_text = get_string_between_delim(tokens[curr].text, &i, "**");
                    if (bold_text == NULL) {
                        error(tokens[curr], "expected a closing **", input_file);
                    }
                    fprintf(out, "\\textbf{%s}", bold_text);
                    free(bold_text);
                } else {
                    putc(tokens[curr].text[i], out);
                }
            }
            fputs("}\n", out);
            curr++;
            continue;
        }

        if (current.kind == Dotpoint) {
            fputs("\\begin{itemize}\n", out);
            while (tokens[curr].kind == Dotpoint) {
                fputs("  \\item{", out);
                for (size_t i = 0; i < strlen(tokens[curr].text); i++) {
                    if (to_token_kind(tokens[curr].text[i]) == Star && to_token_kind(tokens[curr].text[i + 1]) == Star) {
                        char *bold_text = get_string_between_delim(tokens[curr].text, &i, "**");
                        if (bold_text == NULL) {
                            error(tokens[curr], "expected a closing **", input_file);
                        }
                        fprintf(out, "\\textbf{%s}", bold_text);
                        free(bold_text);
                    } else {
                        putc(tokens[curr].text[i], out);
                    }
                }
                fputs("}\n", out);
                curr++;
            }
            fputs("\\end{itemize}\n", out);
            curr++;
            continue;
        }

        if (current.kind == NumberedList) {
            fputs("\\begin{enumerate}\n", out);
            while (tokens[curr].kind == NumberedList) {
                fputs("  \\item{", out);
                for (size_t i = 0; i < strlen(tokens[curr].text); i++) {
                    if (to_token_kind(tokens[curr].text[i]) == Star && to_token_kind(tokens[curr].text[i + 1]) == Star) {
                        char *bold_text = get_string_between_delim(tokens[curr].text, &i, "**");
                        if (bold_text == NULL) {
                            error(tokens[curr], "expected a closing **", input_file);
                        }
                        fprintf(out, "\\textbf{%s}", bold_text);
                        free(bold_text);
                    } else {
                        putc(tokens[curr].text[i], out);
                    }
                }
                fputs("}\n", out);
                curr++;
            }
            fputs("\\end{enumerate}\n", out);
            continue;
        }

        if (current.kind == UnknownBlock) {
            error(current, "unknown block type", input_file);
        }

        if (current.kind == EndBlock) {
            error(current, "expected an opening block", input_file);
        }

        if (current.kind == MathBlock) {
            fputs("\\begin{gather*}\n", out);
            curr++;
            while (tokens[curr].kind != EndBlock) {
                fputs("  ", out);
                if (tokens[curr].kind != Text && tokens[curr].kind != Empty && tokens[curr].kind != NewLine) {
                    char error_text[255];
                    strcat(error_text, "expected a closing block before ");
                    strcat(error_text, printable_tokens[tokens[curr].kind]);
                    error(tokens[curr], error_text, input_file);
                }

                for (size_t string_index = 0; string_index < strlen(tokens[curr].text); string_index++) {
                    if (to_token_kind(tokens[curr].text[string_index]) == Chevron) {
                        if (string_index == strlen(tokens[curr].text) - 1) {
                            error(tokens[curr], "expected another number", input_file);
                        }
                        string_index++;
                        fputs("^{", out);
                        while (to_token_kind(tokens[curr].text[string_index ]) == Number) {
                            fputc(tokens[curr].text[string_index], out);
                            string_index++;
                        }
                        fputc('}', out);
                        string_index--;
                    } else {
                        if (to_token_kind(tokens[curr].text[string_index]) == Star) {
                            fputs("\\times ", out);
                            string_index++;
                        } else if (to_token_kind(tokens[curr].text[string_index]) == Empty) {
                            fputs("\\;", out);
                        } else {
                            fputc(tokens[curr].text[string_index], out);
                        }
                    }
                }
                fputs("\\\\", out);
                putc('\n', out);
                curr++;
            }
            fputs("\\end{gather*}\n", out);
            curr++;
            continue;
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
