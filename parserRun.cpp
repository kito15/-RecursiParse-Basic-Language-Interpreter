#include <iostream>
#include <fstream>
#include <map>
#include "val.h"
#include "parseRun.h"
using namespace std;
bool valAnalyser = false;
bool Factor(istream & in , int & line, Value & return_Value) {
  LexItem item = Parser::GetNextToken( in , line);
  bool FoundErr = false;
  const int line_num = item.GetLinenum();
  switch (item.GetToken()) {
  case ERR:
    item = Parser::GetNextToken( in , line);
    if (item.GetLinenum() == line_num)
      line++;
    Parser::PushBackToken(item);
    valAnalyser = false;
    return true;
  case IDENT:
    Parser::PushBackToken(item);
    if (Var(in,line,item))
      return true;
    return_Value = symbolTable[item.GetLexeme()];
    return false;
  case ICONST:
    valAnalyser = false;
    return_Value = Value(stoi(item.GetLexeme()));
    return false;
  case RCONST:
    valAnalyser = false;
    return_Value = Value(stof(item.GetLexeme()));
    return false;
  case SCONST:
    valAnalyser = false;
    return_Value = Value(item.GetLexeme());
    return false;
  case LPAREN:
    valAnalyser = false;
    if (Expr( in , line, return_Value)) {
      FoundErr = true;
    }
    item = Parser::GetNextToken( in , line);
    if (item.GetToken() == RPAREN)
      return FoundErr;
    else {
      Parser::PushBackToken(item);
      ParseError(line, "Missing Right Parenthesis");
      return true;
    }
  }
  return true;
}
bool Prog(istream & in , int & line) {
  LexItem item = Parser::GetNextToken( in , line);
  bool FoundErr = false;
  if (item.GetToken() != BEGIN) {
    ParseError(line, "No Begin Token");
    Parser::PushBackToken(item);
    FoundErr = true;
  }

  if (StmtList( in , line))
    FoundErr = true;

  return FoundErr;
}
bool Stmt(istream & in , int & line) {
  LexItem item = Parser::GetNextToken( in , line);
  Parser::PushBackToken(item);

  switch (item.GetToken()) {
  case PRINT:
    return PrintStmt( in , line);
  case IF:
    return IfStmt( in , line);
  case IDENT:
    return AssignStmt( in , line);
  default:
    const int currentLine = item.GetLinenum();
    while (currentLine == line) {
      item = Parser::GetNextToken( in , line);
    }
    Parser::PushBackToken(item);
    ParseError(currentLine, "Unrecognized Input");
    return true;
  }
  return false;
}
bool print_ = true;
bool Var(istream & in , int & line, LexItem & tok) {
  LexItem item = Parser::GetNextToken( in , line);
  tok = item;
  if (item.GetToken() == IDENT) {
    if (defVar.find(item.GetLexeme()) != defVar.end()) {
      defVar[item.GetLexeme()] = false;
      valAnalyser = false;
    } else {
      if (valAnalyser) {
        ParseError(line, "Undefined variable");
        valAnalyser = false;
        print_ = false;
        return true;
      }
      defVar[item.GetLexeme()] = true;
    }
  } else {
    ParseError(line, "Invalid Identifier Expression");
    valAnalyser = false;
    return true;
  }
  return false;
}
bool allow_val = true;
bool PrintStmt(istream & in , int & line) {
  const int line_num = line;
  valAnalyser = true;
  ValQue = new queue < Value > ;
  if (ExprList( in , line)) {
    ParseError(line_num, "Invalid Print Statement ");
    line++;
    while (!( * ValQue).empty()) {
      ValQue -> pop();
    }
    delete ValQue;
    return true;
  }
  if (print_ && allow_val) {
    while (print_ && !( * ValQue).empty()) {
      cout << ValQue -> front();
      ValQue -> pop();
    }
    cout << "\n";
  }
  allow_val = true;
  return false;
}
bool IfStmt(istream & in , int & line) {
  LexItem item = Parser::GetNextToken( in , line);
  bool FoundErr = false;
  valAnalyser = true;
  if (item.GetToken() != IF) {
    ParseError(line, "Missing If Statement Expression");
    FoundErr = true;
  }
  item = Parser::GetNextToken( in , line);
  if (item.GetToken() != LPAREN) {
    ParseError(line, "Missing Left Parenthesis");
    FoundErr = true;
  }
  Value expressionValue;
  if (Expr( in , line, expressionValue)) {
    FoundErr = true;
  }
  if (!expressionValue.IsInt()) {
    ParseError(line, "Run-Time: Non Integer If Statement Expression");
    FoundErr = true;
  } else {
    if (expressionValue.GetInt() == 0)
      allow_val = false;

  }
  item = Parser::GetNextToken( in , line);
  if (item.GetToken() != RPAREN) {
    ParseError(line, "Missing Right Parenthesis");
    FoundErr = true;
  }
  item = Parser::GetNextToken( in , line);
  if (item.GetToken() != THEN) {
    ParseError(line, "Missing THEN");
    FoundErr = true;
  }
  if (FoundErr)
    print_ = false;

  if (Stmt( in , line)) {
    ParseError(line, "Invalid If Statement Expression");
    FoundErr = true;
  }
  if (FoundErr)
    print_ = false;

  return FoundErr;
}
bool StmtList(istream & in , int & line) {
  LexItem item;
  bool FoundErr = false;

  while (true) {
    item = Parser::GetNextToken( in , line);
    if (item.GetToken() == DONE || item.GetToken() == END)
      return FoundErr;
    else
      Parser::PushBackToken(item);
    if (Stmt( in , line)) {
      FoundErr = true;
    }
  }
  return FoundErr;
}
bool AssignStmt(istream & in , int & line) {
  const int currentLine = line;
  bool FoundErr = false;
  LexItem tok;
  string identifier = "";

  if (Var( in , line, tok)) {
    ParseError(currentLine, "Invalid Assignment Statement Identifier");
    FoundErr = true;
  } else
    identifier = tok.GetLexeme();

  LexItem item = Parser::GetNextToken( in , line);
  if (item.GetToken() != EQ) {
    ParseError(currentLine, "Missing =");
    FoundErr = true;
  }
  Value val;
  if (Expr( in , line, val)) {
    ParseError(currentLine, "Invalid assignment statement ");
    FoundErr = true;
  }

  if (!FoundErr && identifier != "" && allow_val) {
    if (symbolTable.find(identifier) == symbolTable.end()) {
      symbolTable.insert(make_pair(identifier, val));
    } else {
      Value & actualType = symbolTable.find(identifier) -> second;
      if (actualType.IsInt() && val.IsInt() || actualType.IsReal() && val.IsReal() ||
        actualType.IsStr() && val.IsStr())
        symbolTable[identifier] = val;
      else if (actualType.IsInt() && val.IsReal())
        symbolTable[identifier] = Value((int) val.GetReal());
      else if (actualType.IsReal() && val.IsInt())
        symbolTable[identifier] = Value((float) val.GetInt());
      else {
        ParseError(currentLine, "Illegal type reassignment");
        FoundErr = true;
      }
    }
  }

  item = Parser::GetNextToken( in , line);
  if (item.GetToken() != SCOMA) {
    Parser::PushBackToken(item);
    ParseError(currentLine, "Missing semicolon");
    FoundErr = true;
  }
  if (FoundErr)
    print_ = false;
  allow_val = true;
  return FoundErr;
}

bool ExprList(istream & in , int & line) {
  LexItem item = Parser::GetNextToken( in , line);
  bool FoundErr = false;

  if (item.GetToken() != PRINT) {
    FoundErr = true;
  }

  while (true) {
    Value value;
    if (Expr( in , line, value)) {
      FoundErr = true;
    } else
      ValQue -> push(value);
    item = Parser::GetNextToken( in , line);

    if (item.GetToken() != COMA) {
      if (item.GetToken() == SCOMA)
        return FoundErr;
      Parser::PushBackToken(item);
      return true;
    }
  }
  return FoundErr;
}
bool Expr(istream & in , int & line, Value & return_Value) {
  bool FoundErr = false;
  Value accumulator;
  Value current;
  int operation = -1;

  while (true) {
    if (Term( in , line, current)) {
      return true;
    } else {
      if (accumulator.IsErr())
        accumulator = current;
      else if (operation == 0) {
        if (accumulator.IsStr() || current.IsStr()) {
          ParseError(line, "Invalid String Operation");
          FoundErr = true;
        } else
          accumulator = accumulator + current;
      } else if (operation == 1) {
        if (accumulator.IsStr() || current.IsStr()) {
          ParseError(line, "Invalid Arithmetic Operation");
          FoundErr = true;
        } else
          accumulator = accumulator - current;
      }
    }
    LexItem item = Parser::GetNextToken( in , line);
    if (item.GetToken() == ERR)
      FoundErr = true;
    else if (item.GetToken() == PLUS)
      operation = 0;
    else if (item.GetToken() == MINUS)
      operation = 1;
    else {
      Parser::PushBackToken(item);
      return_Value = accumulator;
      return FoundErr;
    }
  }
  return FoundErr;
}
bool Term(istream & in , int & line, Value & return_Value) {
  bool FoundErr = false;
  Value accumulator;
  Value current;
  int operation = -1;
  while (true) {
    if (Factor( in , line, current)) {
      return true;
    } else {
      if (accumulator.IsErr())
        accumulator = current;
      else if (operation == 0) {
        if (accumulator.IsStr() || current.IsStr()) {
          ParseError(line, "Invalid String Operation");
          FoundErr = true;
        } else
          accumulator = accumulator * current;
      } else if (operation == 1) {
        if (accumulator.IsStr() || current.IsStr()) {
          ParseError(line, "Invalid Arithmetic Operation");
          FoundErr = true;
        } else
          accumulator = accumulator / current;
      }
    }
    LexItem item = Parser::GetNextToken( in , line);
    if (item.GetToken() == ERR)
      FoundErr = true;
    else if (item.GetToken() == MULT)
      operation = 0;
    else if (item.GetToken() == DIV)
      operation = 1;
    else {
      Parser::PushBackToken(item);
      return_Value = accumulator;
      return FoundErr;
    }
  }
  return FoundErr;
}
int main(int argc, char ** argv) {
  if (argc >= 2) {
    fstream file(argv[1]);
    int line_num = 1;

    if (file.is_open()) {
      bool FoundErr = Prog(file, line_num);
      cout << "\n";
      if (FoundErr) {
        cerr << "Unsuccessful Interpretation" << endl;
        cout << "\n";
        cout << "Number of Errors: " << error_count << endl;
      } else
        cout << "Successful Execution" << endl;
    } else {
      cerr << "CANNOT OPEN FILE " << argv[1] << endl;
      exit(1);
    }
  } else {
    cerr << "ONLY ONE FILE NAME ALLOWED" << endl;
    exit(1);
  }
  return 0;
}
