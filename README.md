eval.cpp - This file contains all the source code.

I compiled eval.cpp with g++ (Ubuntu 5.4.0-6ubuntu1~16.04.2) 5.4.0 20160609.

This C++ program illustrates how to tokenize, parse, and evaluate a character string containing arithmetic operations. Rather than tokenizing the input in its entirety before parsing it, we simply tokenize as we parse. The function understand() illustrates how parenthetical ambiguity is resolved.

The program asks the user for input; tokenizes and parses the input; outputs its understanding of the input; and finally, evaluates the input. Distinct arithmetic expressions should be separated by a ';'. For example, inputting "sin(cos exp -2.123) * 3 - -1; 2 ^ -3 ^2/ 0.43e+1; e^pi" will result in an output of three doubles.
