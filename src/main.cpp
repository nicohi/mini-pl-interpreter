#include <iostream>

using namespace std;

static void interpret(string l) {
  cout << "Interpreting line: \"" + l + "\"" << std::endl;
}

static void repl() {
  string line;
  for (;;) {
    if (cin.eof())
      break;
    cout << "> ";
    cin >> line;
    interpret(line);
  }
}

int main(int argc, char *argv[]) {
  repl();
  return 0;
}
