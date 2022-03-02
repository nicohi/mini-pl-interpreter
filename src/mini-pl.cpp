#include "tokens.h"
#include <iostream>

using namespace std;

static void interpret(string l) {
  cout << "Interpreting line: \"" + l + "\"" << std::endl;
}

static void repl() {
  string line;
  for (;;) {
    cout << "> ";
    cin >> line;
    if (cin.eof())
      break;
    interpret(line);
  }
  cout << "\r";
}

static void runFile(string path) {
  cout << path;
  cout << std::endl;
}

static void printHelp() {
  cout << "Usage:\n";
  cout << "\tmini-pl \n";
  cout << "\tmini-pl -h\n";
  cout << "\tmini-pl --help\n";
  cout << "\tmini-pl [path]\n";
}

int main(int argc, char *argv[]) {
  if (argc == 1)
    repl();
  else if (argc == 2) {
    string arg2 = argv[1];
    if (arg2.compare("-h") == 0)
      goto end; // ↑_(ΦwΦ;)Ψ there is no help
    if (arg2.compare("--help") == 0)
      goto end; // (｀㊥益㊥)Ψ  O̶H̶ ̴L̶A̴W̴D̵
    runFile(arg2);
  } else
  end:
    printHelp();
  return 0;
}
