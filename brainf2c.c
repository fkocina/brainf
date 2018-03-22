/*
  Brainf: Compiler of Brainf*** code
  Copyright (C) 2015 Filip Kocina

  This file is part of Brainf.

  Brainf is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Brainf is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Brainf.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

int doIndent(int incIndent) { //for nice indented code and [] count check
  static int indent = 0;
  int i;
  if(incIndent != 0) indent += incIndent; //add/remove some indentation
  else for(i = 0; i < indent; i++) putchar(' '); //indent
  return indent;
}

void echo(const char *s) { //print indented string with NL
  doIndent(0);
  printf("%s\n", s);
}

void genPlus(int count) { //operator + or - (increment or decrement)
  if(!count) return;
  doIndent(0);
  if(count > 1) printf("*head += %i;\n", count); //add
  else if(count < -1) printf("*head -= %i;\n", -count); //subtract
  else if(count == 1) printf("++*head;\n"); //increment
  else if(count == -1) printf("--*head;\n"); //decrement
}

void genShift(int count) { //operator < or > (shift left or right)
  if(!count) return;
  doIndent(0);
  if(count > 1) printf("head += %i;\n", count); //move right
  else if(count < -1) printf("head -= %i;\n", -count); //move left
  else if(count == 1) printf("++head;\n"); //move right by one
  else if(count == -1) printf("--head;\n"); //move left by one
  if(count > 0) echo("if(head > end) enlarge(1);"); //enlarge at the end on need
  else echo("if(head < begin) enlarge(0);"); //enlarge at the beginning on need
}

void compile(const char *program) { //compile brainf*** program
  int plus = 0, shift = 0;
  while(*program) { //generate shift if another command encountered:
    if(shift && *program != '<' && *program != '>') {
      genShift(shift);
      shift = 0;
    }
    if(plus) if(*program == ',') plus = 0; //not needed if the input read
    else if(*program != '+' && *program != '-') { //generate plus (another cmd)
      genPlus(plus);
      plus = 0;
    }
    switch(*program++) { //brainf*** operations:
      case '+': plus++; break; //increment
      case '-': plus--; break; //decrement
      case '<': shift--; break; //shift left
      case '>': shift++; break; //shift right
      case '.': echo("putchar(*head);"); break; //print the current character
      case ',': //read a character
        echo("ret = getchar();");
        echo("if(ret == EOF) exit(0);"); //exit on EOF
        echo("*head = ret;");
        break;
      case '[': //beginning of a cycle
        echo("while(*head) {");
        doIndent(2);
        break;
      case ']': //end of a cycle
        doIndent(-2);
        echo("}");
        break;
    }
  }
  genShift(shift); //generate shift if needed
  genPlus(plus); //generate plus if needed
}

void printProlog() {
  printf("%s",
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <string.h>\n"
    "#include <signal.h>\n"
    "#include <unistd.h>\n"
    "#include <termios.h>\n\n"

    "#define SIZE 1024 //default tape size\n\n"

    "char *begin = NULL, *head, *end;\n"
    "struct termios old;\n\n"

    "void hup(int sig) { //on SIGHUP\n"
    "  exit(sig+128);\n"
    "}\n\n"

    "void term() {\n"
    "  tcsetattr(STDIN_FILENO, TCSANOW, &old); //restore changes\n"
    "  free(begin); //free tape memory\n"
    "}\n\n"

    "void usr1(int sig) { //print the tape on SIGUSR1\n"
    "  char *iter = begin, *endBZeros = end;\n"
    "  while(!*iter && iter < end) iter++; //avoid boundary zeros\n"
    "  while(!*endBZeros && endBZeros > iter) endBZeros--;\n"
    "  while(iter <= endBZeros) {\n"
    "    char c = *iter++;\n"
    "    switch(c) { //special chars:\n"
    "      case '\\\\': fprintf(stderr, \"\\\\\\\\\"); break;\n"
    "      case '\\a': fprintf(stderr, \"\\\\a\"); break;\n"
    "      case '\\b': fprintf(stderr, \"\\\\b\"); break;\n"
    "      case '\\f': fprintf(stderr, \"\\\\f\"); break;\n"
    "      case '\\n': fprintf(stderr, \"\\\\n\"); break;\n"
    "      case '\\r': fprintf(stderr, \"\\\\r\"); break;\n"
    "      case '\\t': fprintf(stderr, \"\\\\t\"); break;\n"
    "      case '\\v': fprintf(stderr, \"\\\\v\"); break;\n"
    "      default: //printable ASCII:\n"
    "         if(c >= 32 && c <= 126) fprintf(stderr, \"%c\", c);\n"
    "         else if(*iter >= '0' && *iter <= '7') //control chars\n"
    "           fprintf(stderr, \"\\\\%03o\", (int)(unsigned char)c);\n"
    "         else fprintf(stderr, \"\\\\%o\", (int)(unsigned char)c);\n"
    "    }\n"
    "  }\n"
    "  fprintf(stderr, \"\\n\");\n"
    "}\n\n"

    "void outOfMemory() {\n"
    "  fprintf(stderr, \"Not enough memory.\\n\");\n"
    "  exit(1);\n"
    "}\n\n"

    "void enlarge(int bAfter) { //reallocate memory\n"
    "  char *newTape;\n"
    "  size_t diff, oldSize, size;\n"
    "  if(begin) {\n"
    "    oldSize = end-begin+1;\n"
    "    if(bAfter) diff = head-end+SIZE; //new block ends SIZE after head\n"
    "    else diff = begin-head+SIZE; //new block begins SIZE before head\n"
    "    size = oldSize+diff;\n"
    "  }\n"
    "  else { //initial allocation\n"
    "    diff = SIZE;\n"
    "    oldSize = 0;\n"
    "    size = SIZE;\n"
    "  }\n"
    "  newTape = malloc(size);\n"
    "  if(!newTape) outOfMemory();\n\n"

    "  if(begin) memcpy(newTape+(bAfter? 0: diff), begin, oldSize);\n"
    "  bzero(newTape+(bAfter? oldSize: 0), diff); //the rest are zeros\n"
    "  free(begin);\n"
    "  head = newTape+(head-begin);\n"
    "  begin = newTape;\n"
    "  end = begin+size-1;\n"
    "  if(!bAfter) head += diff;\n"
    "}\n\n"

    "int main() {\n"
    "  struct termios new;\n"
    "  int ret;\n"
    "  tcgetattr(STDIN_FILENO, &old);\n"
    "  new = old;\n"
    "  new.c_lflag &= ~ICANON; //read chars one by one\n"
    "  tcsetattr(STDIN_FILENO, TCSANOW, &new);\n"
    "  signal(SIGUSR1, usr1);\n"
    "  signal(SIGHUP, hup);\n"
    "  signal(SIGINT, hup);\n"
    "  signal(SIGTERM, hup);\n"
    "  atexit(term);\n"
    "  enlarge(1);\n");
}

void printEpilog() {
  printf("}\n");
}

int main(int argc, char *argv[]) {
  printProlog();
  while(*++argv) {
    doIndent(2);
    compile(*argv);
    if(doIndent(-2)) {
      fprintf(stderr, "[] count mismatch.\n");
      exit(1);
    }
  }
  printEpilog();
  return 0;
}
