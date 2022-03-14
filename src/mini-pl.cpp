#include "compiler.h"
#include "interpreter.h"
#include <cerrno>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

std::string read_file(string path) {
  std::ifstream in(path, std::ios::in | std::ios::binary);
  if (in) {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return (contents);
  }
  throw(errno);
}

static int runFile(string path) {
  string source;
  try {
    source = read_file(path);
  } catch (int e) {
    cerr << "Failed to read file: " << path << endl;
    return errno;
  }
  Interpreter::interpret(source);
  return errno;
}

static int runScanner(string path) {
  string source;
  try {
    source = read_file(path);
  } catch (int e) {
    cerr << "Failed to read file: " << path << endl;
    return errno;
  }
  Compiler::runScanner(source);
  return errno;
}

static int runParser(string path) {
  string source;
  try {
    source = read_file(path);
  } catch (int e) {
    cerr << "Failed to read file: " << path << endl;
    return errno;
  }
  Compiler::runParser(source);
  return errno;
}

static void repl() {
  string line;
  for (;;) {
    cout << "> ";
    cin >> line;
    if (cin.eof())
      break;
    Interpreter::interpret(line);
  }
  cout << "\r";
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
  else if (argc >= 2) {
    string arg1 = argv[1];
    if (arg1.compare("-h") == 0)
      goto end; // ↑_(ΦwΦ;)Ψ there is no help
    if (arg1.compare("--help") == 0)
      goto end; // (｀㊥益㊥)Ψ
    if (arg1.compare("-s") == 0) {
      string arg2 = argv[2];
      runScanner(arg2);
    } else if (arg1.compare("-p") == 0) {
      string arg2 = argv[2];
      runParser(arg2);
    } else
      return runFile(arg1);
  } else
  end:
    printHelp();
  return 0;
}
