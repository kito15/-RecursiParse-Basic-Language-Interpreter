Created a recursive-descent parser-based interpreter for the basic programming language.
  The interpreter analyzes the grammar of the input source code statement by statement before executing the statement if there are no syntactic or semantic errors.
  For each declared variable, the interpreter creates a symbol table.
  The interpreter evaluates expressions and assigns values and types to them.
  The outcomes of a failed parsing are a collection of error messages displayed by the parser functions, as well as any errors found by the lexical analyzer.
  Any problems caused by the process of parsing or interpreting the input program should prompt the interpretation process to halt and return.
