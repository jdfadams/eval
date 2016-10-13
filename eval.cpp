// This program evaluates arithmetic expressions.
// It is a simple exercise in tokenizing, parsing, and evaluating a character string.
// Distinct arithmetic expressions should be separated by a ';'.
// For example, it will evaluate "sin(cos exp -2.123) * 3 - -1; 2 ^ -3 ^2/ 0.43e+1; e^pi", printing three doubles.

#include <cctype> // We need isalnum, isalpha, isprint, and isspace.
#include <cmath> // We need some standard math functions.
#include <cstdlib> // We need strtod.

#include <sstream> // We use stringstream in the function dtostr.

#include <iostream> // We use console input/output.
#include <string>
#include <vector>

using namespace std; // I will avoid typing std:: over and over.

// This function converts a double to a string.
string dtostr(const double &d)
{
 stringstream s;
 s << d;
 return s.str();
}

enum SymbolType {
 ASSIGN,
 CONSTANT,
 MATHFN,
 OPERATOR
};

enum MathFunctions {
 EXP,
 COS,
 LOG,
 SIN,
 TAN
};

// A symbol consists of a type and a value.
// If the type is CONSTANT, then the value is an index into an array (or vector) of constants.
// If the type is MATHFN, then the value is an element of MathFunctions.
// If the type is OPERATOR, then the value is the character representing the operator.
class Symbol {
 public:
  SymbolType type;
  unsigned int value;
  Symbol(const SymbolType &a, const unsigned int &b)
  {
   type = a;
   value = b;
  }
  ~Symbol() {}
};

// Variables have an associated name and value.
class Variable {
 public:
  string name;
  double value;
  Variable(const string &a, const double &b)
  {
   name = a;
   value = b;
  }
  ~Variable() {}
};

// This is where all of the tokenizing, parsing, and evaluation takes place.
class Evaluator {
 private:

  vector<double> constants; // This holds any numbers read from the input.

  vector<Symbol> symbols;
  // We generate a vector of symbols while parsing the input.
  // These are the instructions deduced from the input.
  // Execute these instructions to evaluate the input.

  vector<Variable> variables; // At the moment, these "variables" are simply named values (i.e., a constant associated with a string).

  const char *line; // This is the input to be tokenized and parsed.

  unsigned int pos; // This is the index of the current character in line.

 protected:

  // This function looks up the index of name in variables.
  // If name is found, then the returned value is in [0, variables.size()).
  // If name cannot be found, the returned value is variables.size().
  unsigned int find_variable(const string &name) const
  {
   unsigned int i;
   for (i = 0; i < variables.size(); i ++)
   {
    if (variables[i].name == name) break;
   }
   return i;
  }

  // This function formats error messages and displays where the error occurred.
  string error(const string &m, const unsigned int &l = 20) const
  {
   string message = m;
   message += '\n'; // Record the message m.

   // Record l characters of line, starting from where the error occurred.
   unsigned int i;
   for (i = pos; i < pos + l; i ++)
   {
    if (line[i] == 0) break; // Stop recording characters if there aren't any more.

    if (isprint(line[i])) // If a character is printable, record the character.
    {
     message += line[i];
    }
    else // If a character isn't printable, record its integer value.
    {
     message += '\\';
     message += static_cast<unsigned int>(line[i]);
    }
   }

   message += "\n^"; // Add a line break, and a little arrow pointing at where the error occurred.

   return message;
  }

  // This function extracts a double from line, starting at pos.
  // In order to work properly, this function should only be called if there actually is a double that can be extracted.
  // The double should be formatted in a way acceptable by C++ (i.e., -124.32e+23).
  double get_double()
  {
   char *endptr; // This will point to the first character after the extracted double.
   double x = strtod(&line[pos], &endptr); // Extract a double with strtod.
   pos += endptr - &line[pos]; // Move pos so that line[pos] is the first character after the extracted double.
   return x;
  }

  // This function extracts a name (as determined by the function issname defined below) from line, starting at pos.
  string get_name()
  {
   string name = "";
   do {
    name += line[pos];
    pos ++;
   } while (isname(line[pos]));
   return name;
  }

  // This function skips whitespace characters.
  void eatspace()
  {
   while (isspace(line[pos]))
   {
    pos ++;
   }
   return;
  }

  // After calling this function, line[pos] will be the first non-whitespace character after the current character.
  void nextch()
  {
   pos ++;
   eatspace();
   return;
  }

  // This function returns true if and only if ch is an alpha-numeric character or an underscore.
  bool isname(const char &ch)
  {
   return isalnum(ch) || ch == '_';
  }

  // The following functions, level1, ..., level_top, are where the parsing of line occurs.
  // Operators with higher precedence are parsed by a higher level.

  // This level parses the binary operators + and -.
  bool level1()
  {
   if (! level2())
   {
    return false;
   }
   while (line[pos] == '+' || line[pos] == '-') // We parse from left to right.
   {
    char op = line[pos];
    nextch();
    if (! level2())
    {
     return false;
    }
    if (op == '+')
    {
     symbols.push_back(Symbol(OPERATOR, '+'));
    }
    else
    {
     symbols.push_back(Symbol(OPERATOR, '-'));
    }
   }
   return true;
  }

  // This level parses the binary operators * and /.
  bool level2()
  {
   if (! level3())
   {
    return false;
   }
   while (line[pos] == '*' || line[pos] == '/') // We parse from left to right.
   {
    char op = line[pos];
    nextch();
    if (! level3())
    {
     return false;
    }
    if (op == '*')
    {
     symbols.push_back(Symbol(OPERATOR, '*'));
    }
    else
    {
     symbols.push_back(Symbol(OPERATOR, '/'));
    }
   }
   return true;
  }

  // This level parses the unary operator -.
  bool level3()
  {
   if (line[pos] == '-')
   {
    nextch();
    if (! level3())
    {
     return false;
    }
    symbols.push_back(Symbol(OPERATOR, 'n'));
   }
   else
   {
    if (! level4())
    {
     return false;
    }
   }
   return true;
  }

  // This level parses the power operator ^.
  bool level4()
  {
   if (! level_top())
   {
    return false;
   }
   while (line[pos] == '^')
   {
    nextch();
    if (! level3())
    {
     return false;
    }
    symbols.push_back(Symbol(OPERATOR, '^'));
   }
   return true;
  }

  // The main purpose of this level is to tokenize the input, but it also deals with parentheses.
  bool level_top()
  {
   if (isdigit(line[pos])) // We can extract a double.
   {
    constants.push_back(get_double());
    eatspace();
    symbols.push_back(Symbol(CONSTANT, constants.size() - 1));
   }
// Note that when we extract a double, we haven't required that the next character is an operator or whitespace.
// Consequently, if the input is 12asdf, then the double extracted will be 12, and the next token will be the name "asdf".
// This could be interpreted as a bug.

   else if (line[pos] == '(') // We should deal with parentheses.
   {
    nextch();
    if (! level1())
    {
     return false;
    }
    if (line[pos] != ')') // There should be a matching paretheses; if not, output an error.
    {
     cout << error("expected \')\'") << endl;
     return false;
    }
    nextch();
   }
   else if (isalpha(line[pos]) || line[pos] == '_') // We can extract a name.
   {
    string name = get_name(); // We expect that name will be either a predefined function or a variable name.
    eatspace();

    // If name is a function, we should parse the next token from level1.
    // In other words, if we see "cos 2^3*4", we should interpret this as (cos(2^3))*4.
    if (name == "cos")
    {
     if (! level3())
     {
      return false;
     }
     symbols.push_back(Symbol(MATHFN, COS));
    }
    else if (name == "exp")
    {
     if (! level3())
     {
      return false;
     }
     symbols.push_back(Symbol(MATHFN, EXP));
    }
    else if (name == "log")
    {
     if (! level3())
     {
      return false;
     }
     symbols.push_back(Symbol(MATHFN, LOG));
    }
    else if (name == "sin")
    {
     if (! level3())
     {
      return false;
     }
     symbols.push_back(Symbol(MATHFN, SIN));
    }
    else if (name == "tan")
    {
     if (! level3())
     {
      return false;
     }
     symbols.push_back(Symbol(MATHFN, TAN));
    }
    else // If we get here, then name wasn't a predefined function.
    {
     unsigned int i = find_variable(name); // Check whether we have the name of an existing variable.
     if (i != variables.size()) // If name is associated with a variable, get the variable's value.
     {
      constants.push_back(variables[i].value);
      symbols.push_back(Symbol(CONSTANT, constants.size() - 1));
     }
     else // There is no variable associated with name.
     {
      cout << error(string("unknown name: ") + name) << endl;
      return false;
     }
    }
   }
   else if (line[pos] == 0)
   {
    cout << error("unexpected end of input") << endl;
   }
   else // No other tokens than those listed above are permitted.
   {
    cout << error("unexpected character") << endl;
    return false;
   }
   return true;
  }

 public:

  // There isn't really anything to construct.
  Evaluator()
  {
   // For now, we will just have two "variables", which are, of course, famous mathematical constants.
   variables.push_back(Variable("e", 2.7182818284590452353603));
   variables.push_back(Variable("pi", 3.1415926535897932384626));
  }

  // Nor is there anything to destruct.
  ~Evaluator() {}

  // This function parses the character string str.
  bool parse(const char *str)
  {
   line = str;
   pos = 0;
   eatspace(); // Skip any initial whitespace.

   while (line[pos] != 0)
   {
    if (! level1())
    {
     return false;
    }

    // There are now two allowed cases:
    // 1) We have reached the end of the input string.
    // 2) There are multiple "sentences" separated by ';'.
    if (line[pos] != ';' && line[pos] != 0)
    {
     cout << error("expected \';\' or end of input") << endl;
     return false;
    }
    symbols.push_back(Symbol(OPERATOR, ';'));

    if (line[pos] == ';') nextch();
    // If line[pos] is ';', then we skip one character and go to the next non-whitespace character.
    // If line[pos] is not ';', then the parsing functions already called have already skipped to the next non-whitespace character.
   }
   return true;
  }

  // This function returns a string indicating the computer's understanding of the input, illustrating how parenthetical ambiguity is resolved.
  // For example, "2^-3^4" is understood as 2^(-(3^4)).
  string understanding() const
  {
   string expr = ""; // This holds the unambiguously parenthesized version of the input string.
   vector<string> s; // This is a stack of strings rather than numbers.

   // Now, we pass over all of the recorded instructions.
   unsigned int i;
   for (i = 0; i < symbols.size(); i ++)
   {
    if (symbols[i].type == CONSTANT) // If we find a double, push it (as a string) onto the back of s.
    {
     s.push_back(dtostr(constants[symbols[i].value])); // Recall that dtostr is the function we defined to convert a double to a string.
    }
    else if (symbols[i].type == MATHFN) // We have a math function, so we have to find the argument passed to it.
    {
     string b = s.back(); // This is the argument passed to the math function.
     s.pop_back();

     // Write the math function using characters.
     string temp;
     if (symbols[i].value == COS)
     {
      temp = "cos";
     }
     else if (symbols[i].value == EXP)
     {
      temp = "exp";
     }
     else if (symbols[i].value == LOG)
     {
      temp = "log";
     }
     else if (symbols[i].value == SIN)
     {
      temp = "sin";
     }
     else if (symbols[i].value == TAN)
     {
      temp = "tan";
     }
     else // This should never happen, provided we parsed everything correctly.
     {
      return "????";
     }

     s.push_back(temp + '(' + b + ')'); // This records the math function and its argument.
    }
    else if (symbols[i].type == OPERATOR) // We have an operator, either binary or unary.
    {
     string b = s.back(); // Any such operator requires at least one argument.
     s.pop_back();

     if (symbols[i].value == ';') // This "operator" is more of a place-holder, but it also indicates that the result of the computation should be displayed.
     {
      expr += b + ";\n";
     }
     else if (symbols[i].value == 'n') // This means the unary operator negation.
     {
      s.push_back("(-" + b + ')');
     }
     else // We have a binary operator.
     {
      string a = s.back(); // The argument b is second; get the first argument a.
      s.pop_back();

      // Combine a, the operator, and b.
      if (symbols[i].value == '+')
      {
       s.push_back('(' + a + '+' + b + ')');
      }
      else if (symbols[i].value == '-')
      {
       s.push_back('(' + a + '-' + b + ')');
      } 
      else if (symbols[i].value == '*')
      {
       s.push_back('(' + a + '*' + b + ')');
      }
      else if (symbols[i].value == '/')
      {
       s.push_back('(' + a + '/' + b + ')');
      }
      else if (symbols[i].value == '^')
      {
       s.push_back('(' + a + '^' + b + ')');
      }
      else // This should never happen, provided we parsed everything correctly.
      {
       return "????";
      }

     } // This is the end of the binary operators.

    } // This is the end of the trichotomy, CONSTANT, MATHFN, or OPERATOR.

   } // This is the end of the for loop passing over all the symbols.

   // If there is anything left on the stack, the input wasn't parsed correctly.
   // This should never happen.
   if (s.size() != 0)
   {
    cout << "unfinished" << endl;
    return "????";
   }

   return expr; // Return the string we build out of the input.
  }

  // This function is almost the same as the function understanding, but rather than formatting the computer's understanding as a string, we actually perform the indicated arithmetical operations.
  bool run()
  {
   vector<double> s;
   unsigned int i;
   for (i = 0; i < symbols.size(); i ++)
   {
    if (symbols[i].type == CONSTANT)
    {
     s.push_back(constants[symbols[i].value]);
    }
    else if (symbols[i].type == MATHFN)
    {
     double b = s.back();
     s.pop_back();
     if (symbols[i].value == COS)
     {
      s.push_back(cos(b));
     }
     else if (symbols[i].value == EXP)
     {
      s.push_back(exp(b));
     }
     else if (symbols[i].value == LOG)
     {
      s.push_back(log(b));
     }
     else if (symbols[i].value == SIN)
     {
      s.push_back(sin(b));
     }
     else if (symbols[i].value == TAN)
     {
      s.push_back(tan(b));
     }
     else // This should never happen.
     {
      cout << "unknown math function" << endl;
      return false;
     }
    }
    else if (symbols[i].type == OPERATOR)
    {
     double b = s.back();
     s.pop_back();
     if (symbols[i].value == ';')
     {
      cout << "Evaluation: " << b << endl;
     }
     else if (symbols[i].value == 'n')
     {
      s.push_back(-b);
     }
     else // We have a binary operator.
     {
      double a = s.back();
      s.pop_back();
      if (symbols[i].value == '+')
      {
       s.push_back(a + b);
      }
      else if (symbols[i].value == '-')
      {
       s.push_back(a - b);
      } 
      else if (symbols[i].value == '*')
      {
       s.push_back(a * b);
      }
      else if (symbols[i].value == '/')
      {
       s.push_back(a / b);
      }
      else if (symbols[i].value == '^')
      {
       s.push_back(pow(a, b));
      }
      else // This should never happen.
      {
       cout << "unknown operator " << symbols[i].value << endl;
       return false;
      }

     } // This is the end of the binary operators.

    } // This is the end of the trichotomy, CONSTANT, MATHFN, or OPERATOR.

   } // This is the end of the for loop passing over all the symbols.

   // If there is anything left on the stack, the input wasn't parsed correctly.
   // This should never happen.
   if (s.size() != 0)
   {
    cout << "unfinished" << endl;
    return false;
   }

   return true;
  }

  // This function clears the constants and symbols.
  // It makes sure that everything is set up to evaluate another expression.
// Note that at the moment, no functionality is needed to clear variables.
  void clear()
  {
   constants.clear();
   symbols.clear();
   return;
  }

  // This function evaluates the expressions in the character string str.
  bool eval(const char *str)
  {
   if (! parse(str))
   {
    return false;
   }

   // Output the computer's understanding of the input, writing parentheses to illustrate how any ambiguity has been resolved.
   // For example, the computer will understand "2^-3^4" as 2^(-(3^4)).
   cout << "Understanding:" << endl
        << understanding() << endl;

   if (! run())
   {
    return false;
   }
   return true;
  }
};

int main()
{
 Evaluator test;
 string input;

 // Evaluate the user's input until the user asks to quit.
 while (true)
 {
  cout << "> ";
  getline(cin, input);
  if (input == "quit") break;
  if (! test.eval(input.c_str()))
  {
   cout << "error" << endl;
  }
  test.clear();
 }

 return 0;
}
