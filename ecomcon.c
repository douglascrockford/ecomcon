/*  ecomcon.c
    Douglas Crockford
    2019-05-05

    Public Domain

    Ecomcon is a simple preprocessor. It enables comments conditionally. The
    comments can be enabled by supplying tag names on the command line. These
    comments are normally ignored, and will be removed by JSMin. But ecomcon
    will activate these comments, making them executable. They can be used for
    development time activities like debugging, testing, logging, or tracing. A
    conditional comment is a '//' style comment that starts at the left margin.
    The '//' is immediately followed by a <tag>. There must be no space between
    the slashes and the <tag>.

        //<tag> <stuff>

    If the <stuff> starts with a letter or digit, then a space must be placed
    between <tag> and <stuff>.

    The command line will contain a list of <tag> names.

    A <tag> may contain any short sequence of ASCII letters, digits, and
    underbar '_'. The active <tag> strings are declared in the command line.
    All <tag>s that are not declared in the command line are deleted.

    A command line can contain zero or more comments.

        -comment <comment>

    Sample method line:

        ecomcon debug log -comment "Devel Edition"

    That will activate the debug and log comments, and will also insert the
    comment

        // Devel Edition

    at the top of the output file.

    A program is read from stdin, and a modified program is written to stdout.
    Errors are written to stderr.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_NR_TAGS         1000
#define MAX_LINE_LENGTH     65536

static int  nr_tags;
static int  tag_lengths[MAX_NR_TAGS];
static char *tags[MAX_NR_TAGS];

static char line[MAX_LINE_LENGTH];
static int  line_length;
static int  line_nr;

static int the_character;

static void error(char* message) {
    fputs("ecomcon: ", stderr);
    if (line_nr == 0) {
        fprintf(stderr, "\necomcon: %s\n", message);
    } else {
        fprintf(stderr, "\necomcon: %d %s\n", line_nr, message);
    }
    exit(EXIT_FAILURE);
}


static int is_tag_char(int c) {
/*
    Return 1 if the character is a letter, digit, underscore.
*/
    return (
        (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9')
        || (c >= 'A' && c <= 'Z')
        || c == '_'
    );
}

 static void emit(int c) {
/*
    Send a character to stdout.
*/
    if (fputc(c, stdout) == EOF) {
        error("write error.");
    }
}


static void emit_line(int from) {
/*
    Send all or part of the current line to stdout.
*/
    int index;
    for (index = from; index < line_length; index += 1) {
        emit(line[index]);
    }
}


static int match(int length) {
    int tag_nr;
    int index;
    int ok;

    for (tag_nr = 0; tag_nr < nr_tags; tag_nr += 1) {
        if (tag_lengths[tag_nr] == length) {
            ok = 1;
            for (index = 0; index < length; index += 1) {
                if (tags[tag_nr][index] != line[2 + index]) {
                    ok = 0;
                    break;
                }
            }
            if (ok) {
                return 1;
            }
        }
    }
    return 0;
}

static int next() {
    the_character = fgetc(stdin);
    return the_character;
}

static int read_line() {
    int at = 0;
    int eof = 0;
    while (1) {
        line_nr += 1;
        if (the_character == EOF) {
            return eof = (at == 0);
        }
        if (the_character == '\n') {
            next();
            break;
        }
        if (the_character == '\r') {
            if (next() == '\n') {
                next();
            }
            break;
        }
        line[at] = the_character;
        at += 1;
        if (at >= MAX_LINE_LENGTH) {
            error("Line too long.");
        }
        next();
    }
    line_length = at;
    line[at] = 0;
    return 0;
}

static void process() {
    next();
    while (1) {
        line_length = 0;
        int supress = 0;
        int at = 0;
        if (read_line() != 0) {
            break;
        }
        if (line_length > 2 && line[0] == '/' && line[1] == '/') {
            int tag_length = 0;
            while (is_tag_char(line[tag_length + 2])) {
                tag_length += 1;
            }
            if (tag_length > 0) {
                if (match(tag_length) == 0) {
                    supress = 1;
                } else {
                    at += tag_length + 2;
                    if (line[at] == ' ') {
                        at += 1;
                    }
                }
            }
        }
        if (!supress) {
            emit_line(at);
            emit('\n');
        }
    }
}


extern int main(int argc, char *argv[]) {
    char *arg;
    int arg_nr;
    int at;
    int comment = 0;
    int the_character;
    line_nr = 0;
    nr_tags = 0;
    for (arg_nr = 1; arg_nr < argc; arg_nr += 1) {
        arg = argv[arg_nr];
        if (strcmp(arg, "-comment") == 0) {
            comment = 1;
        } else if (comment) {
            comment = 0;
            emit('/');
            emit('/');
            puts(arg);
            emit('\n');
        } else {
            at = 0;
            while (1) {
                the_character = arg[at];
                if (the_character == 0) {
                    break;
                }
                if (!is_tag_char(the_character)) {
                    error(arg);
                }
                at += 1;
            }
            if (at > 0) {
                tags[nr_tags] = arg;
                tag_lengths[nr_tags] = at;
                nr_tags += 1;
            }
        }
    }
    process();
    return EXIT_SUCCESS;
}
