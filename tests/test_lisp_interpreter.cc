#define CATCH_CONFIG_MAIN

#include "lisp_interpreter.h"
#include "catch.hpp"
#include <cmath>

TEST_CASE("LispInterpreter::MakeList()")
{

    procdraw::LispInterpreter L;

    SECTION("Empty list") {
        REQUIRE(L.Null(L.MakeList({})));
    }

    SECTION("Single item") {
        auto list1 = L.MakeList({ L.MakeNumber(42) });
        REQUIRE(L.TypeOf(list1) == procdraw::LispObjectType::Cons);
        REQUIRE(L.NumVal(L.Car(list1)) == 42);
        REQUIRE(L.Null(L.Cdr(list1)));
    }

    // +---+  +---+  +---+  +---+
    // |1|*|->|2|*|->|3|*|->|4|/|
    // +---+  +---+  +---+  +---+
    //
    SECTION("Multiple items") {
        auto list1 = L.MakeList({ L.MakeNumber(1), L.MakeNumber(2), L.MakeNumber(3), L.MakeNumber(4) });
        REQUIRE(L.TypeOf(list1) == procdraw::LispObjectType::Cons);
        REQUIRE(L.NumVal(L.Car(list1)) == 1);
        REQUIRE(L.NumVal(L.Car(L.Cdr(list1))) == 2);
        REQUIRE(L.NumVal(L.Car(L.Cdr(L.Cdr(list1)))) == 3);
        REQUIRE(L.NumVal(L.Car(L.Cdr(L.Cdr(L.Cdr(list1))))) == 4);
        REQUIRE(L.Null(L.Cdr(L.Cdr(L.Cdr(L.Cdr(list1))))));
    }

    // +---+  +---+  +---+
    // |*|*|->|3|*|->|4|/|
    // +---+  +---+  +---+
    //  |
    //  v
    // +---+  +---+
    // |1|*|->|2|/|
    // +---+  +---+
    //
    SECTION("Embedded list") {
        auto list1 = L.MakeList({ L.MakeList({ L.MakeNumber(1), L.MakeNumber(2) }), L.MakeNumber(3), L.MakeNumber(4) });
        REQUIRE(L.TypeOf(list1) == procdraw::LispObjectType::Cons);
        REQUIRE(L.NumVal(L.Car(L.Car(list1))) == 1);
        REQUIRE(L.NumVal(L.Car(L.Cdr(L.Car(list1)))) == 2);
        REQUIRE(L.Null(L.Cdr(L.Cdr(L.Car(list1)))));
        REQUIRE(L.NumVal(L.Car(L.Cdr(list1))) == 3);
        REQUIRE(L.NumVal(L.Car(L.Cdr(L.Cdr(list1)))) == 4);
        REQUIRE(L.Null(L.Cdr(L.Cdr(L.Cdr(list1)))));
    }
}

TEST_CASE("LispInterpreter::PrintString()")
{

    procdraw::LispInterpreter L;

    SECTION("Integer") {
        REQUIRE(L.PrintString(L.MakeNumber(42)) == "42");
    }

    SECTION("Symbol") {
        REQUIRE(L.PrintString(L.SymbolRef("HELLO")) == "HELLO");
    }

    SECTION("List") {
        REQUIRE(L.PrintString(L.MakeList({})) == "nil");
        REQUIRE(L.PrintString(L.MakeList({ L.MakeNumber(42) })) == "(42)");
        REQUIRE(L.PrintString(L.MakeList({ L.MakeNumber(1), L.MakeNumber(2), L.MakeNumber(3), L.MakeNumber(4) })) ==
                "(1 2 3 4)");
        REQUIRE(L.PrintString(L.MakeList({ L.MakeList({ L.MakeNumber(1), L.MakeNumber(2) }), L.MakeNumber(3), L.MakeNumber(4) }))
                == "((1 2) 3 4)");
    }

    SECTION("Dotted pair") {
        REQUIRE(L.PrintString(L.Cons(L.MakeNumber(1), L.MakeNumber(2))) == "(1 . 2)");
    }

    SECTION("Boolean") {
        REQUIRE(L.PrintString(L.True) == "true");
        REQUIRE(L.PrintString(L.False) == "false");
    }

    SECTION("String") {
        REQUIRE(L.PrintString(L.MakeString("some string")) == "\"some string\"");
    }

    SECTION("Table") {
        REQUIRE(L.PrintString(L.MakeTable()) == "<Table>");
    }

    SECTION("Quote") {
        REQUIRE(L.PrintString(L.MakeList({ L.SymbolRef("quote"), L.MakeNumber(42) })) == "'42");
    }

}

TEST_CASE("LispInterpreter::SymbolRef()")
{

    procdraw::LispInterpreter L;

    auto hello1 = L.SymbolRef("HELLO");
    REQUIRE(L.TypeOf(hello1) == procdraw::LispObjectType::Symbol);
    REQUIRE(L.SymbolName(hello1) == "HELLO");
    auto hello2 = L.SymbolRef("HELLO");
    REQUIRE(L.TypeOf(hello2) == procdraw::LispObjectType::Symbol);
    REQUIRE(L.SymbolName(hello2) == "HELLO");
    // Verify that they are the same address
    REQUIRE(hello1.get() == hello2.get());
}

TEST_CASE("LispInterpreter::Eq()")
{

    procdraw::LispInterpreter L;

    REQUIRE(L.Eq(L.Nil, L.Nil));

    REQUIRE(L.Eq(L.MakeNumber(42), L.MakeNumber(42)));
    REQUIRE_FALSE(L.Eq(L.MakeNumber(42), L.MakeNumber(1)));
    REQUIRE_FALSE(L.Eq(L.MakeNumber(42), L.Nil));
    REQUIRE_FALSE(L.Eq(L.Nil, L.MakeNumber(42)));

    REQUIRE(L.Eq(L.SymbolRef("A"), L.SymbolRef("A")));
    REQUIRE_FALSE(L.Eq(L.SymbolRef("A"), L.SymbolRef("B")));

    auto list1 = L.MakeList({ L.MakeNumber(42) });
    REQUIRE(L.Eq(list1, list1));
}

TEST_CASE("LispInterpreter::Assoc()")
{

    procdraw::LispInterpreter L;

    SECTION("Empty association list") {
        REQUIRE(L.Null(L.Assoc(L.SymbolRef("A"), L.Nil)));
    }

    SECTION("Non-empty association list") {
        auto symbolA = L.SymbolRef("A");
        auto symbolB = L.SymbolRef("B");
        auto alist = L.MakeList({ L.Cons(symbolA, L.MakeNumber(1)), L.Cons(symbolB, L.MakeNumber(2)) });

        auto pairA = L.Assoc(symbolA, alist);
        REQUIRE(L.Eq(symbolA, L.Car(pairA)));
        REQUIRE(L.NumVal(L.Cdr(pairA)) == 1);

        auto pairB = L.Assoc(symbolB, alist);
        REQUIRE(L.Eq(symbolB, L.Car(pairB)));
        REQUIRE(L.NumVal(L.Cdr(pairB)) == 2);

        REQUIRE(L.Null(L.Assoc(L.SymbolRef("C"), alist)));
    }

}

TEST_CASE("LispInterpreter::Rplaca()")
{

    procdraw::LispInterpreter L;

    auto cons = L.Cons(L.MakeNumber(1), L.MakeNumber(2));
    auto result = L.Rplaca(cons, L.MakeNumber(10));
    REQUIRE(L.NumVal(L.Car(cons)) == 10);
    REQUIRE(L.NumVal(L.Cdr(cons)) == 2);
    REQUIRE(L.Eq(cons, result));
}

TEST_CASE("LispInterpreter::Rplacd()")
{

    procdraw::LispInterpreter L;

    auto cons = L.Cons(L.MakeNumber(1), L.MakeNumber(2));
    auto result = L.Rplacd(cons, L.MakeNumber(20));
    REQUIRE(L.NumVal(L.Car(cons)) == 1);
    REQUIRE(L.NumVal(L.Cdr(cons)) == 20);
    REQUIRE(L.Eq(cons, result));
}

TEST_CASE("Tables")
{

    procdraw::LispInterpreter L;

    auto table = L.MakeTable();

    SECTION("Get, Put, and Clear") {
        REQUIRE(L.Null(L.Get(table, L.SymbolRef("key1"))));
        REQUIRE(L.NumVal(L.Put(table, L.SymbolRef("key1"), L.MakeNumber(42))) == 42);
        REQUIRE(L.NumVal(L.Get(table, L.SymbolRef("key1"))) == 42);
        REQUIRE(L.NumVal(L.Put(table, L.SymbolRef("key1"), L.MakeNumber(10))) == 10);
        REQUIRE(L.NumVal(L.Get(table, L.SymbolRef("key1"))) == 10);
        REQUIRE(L.Null(L.Get(table, L.SymbolRef("key2"))));
        L.Put(table, L.SymbolRef("key2"), L.MakeNumber(100));
        REQUIRE(L.NumVal(L.Get(table, L.SymbolRef("key2"))) == 100);
        REQUIRE(L.NumVal(L.Get(table, L.SymbolRef("key1"))) == 10);
        L.Clear(table);
        REQUIRE(L.Null(L.Get(table, L.SymbolRef("key1"))));
        REQUIRE(L.Null(L.Get(table, L.SymbolRef("key2"))));
    }

    SECTION("Keys") {
        REQUIRE(L.Null(L.Keys(table)));

        L.Put(table, L.SymbolRef("key1"), L.MakeNumber(42));
        auto singleKey = L.Keys(table);
        REQUIRE(L.SymbolName(L.Car(singleKey)) == "key1");
        REQUIRE(L.Null(L.Cdr(singleKey)));

        L.Put(table, L.SymbolRef("key2"), L.MakeNumber(10));
        auto twoKeys = L.Keys(table);
        REQUIRE(L.Null(L.Cddr(twoKeys)));
        auto foundExpectedKeys = (L.SymbolName(L.Car(twoKeys)) == "key1" && L.SymbolName(L.Cadr(twoKeys)) == "key2")
                                 || (L.SymbolName(L.Car(twoKeys)) == "key2" && L.SymbolName(L.Cadr(twoKeys)) == "key1");
        REQUIRE(foundExpectedKeys);
    }

}

TEST_CASE("LispInterpreter implicit type conversion")
{

    procdraw::LispInterpreter L;

    SECTION("to number") {
        SECTION("should preserve numbers and convert non-numbers to NaN") {
            // Nil
            REQUIRE(std::isnan(L.NumVal(L.Nil)));
            // Number
            REQUIRE(L.NumVal(L.MakeNumber(42)) == 42);
            // Symbol
            REQUIRE(std::isnan(L.NumVal(L.SymbolRef("SYMBOL"))));
            // Cons
            REQUIRE(std::isnan(L.NumVal(L.Cons(L.MakeNumber(1), L.MakeNumber(2)))));
            // CFunction
            REQUIRE(std::isnan(L.NumVal(L.MakeCFunction(nullptr, nullptr))));
            // Boolean
            REQUIRE(std::isnan(L.NumVal(L.True)));
            REQUIRE(std::isnan(L.NumVal(L.False)));
            // String
            REQUIRE(std::isnan(L.NumVal(L.MakeString("some string"))));
            // Table
            REQUIRE(std::isnan(L.NumVal(L.MakeTable())));
        }
    }

    SECTION("to bool") {
        SECTION("should preserve booleans, convert nil to false, and other non-booleans to true") {
            // Nil
            REQUIRE_FALSE(L.BoolVal(L.Nil));
            // Number
            REQUIRE(L.BoolVal(L.MakeNumber(42)));
            // Symbol
            REQUIRE(L.BoolVal(L.SymbolRef("SYMBOL")));
            // Cons
            REQUIRE(L.BoolVal(L.Cons(L.MakeNumber(1), L.MakeNumber(2))));
            // CFunction
            REQUIRE(L.BoolVal(L.MakeCFunction(nullptr, nullptr)));
            // Boolean
            REQUIRE(L.BoolVal(L.True));
            REQUIRE_FALSE(L.BoolVal(L.False));
            // String
            REQUIRE(L.BoolVal(L.MakeString("some string")));
            // Table
            REQUIRE(L.BoolVal(L.MakeTable()));
        }
    }

}

TEST_CASE("LispInterpreter::Eval")
{

    procdraw::LispInterpreter L;

    SECTION("should evaluate NIL to itself") {
        REQUIRE(L.Null(L.Eval(L.Nil)));
    }

    SECTION("should evaluate a number to itself") {
        REQUIRE(L.NumVal(L.Eval(L.MakeNumber(42))) == 42);
    }

    SECTION("should evaluate booleans to themselves") {
        REQUIRE(L.BoolVal(L.Eval(L.True)));
        REQUIRE_FALSE(L.BoolVal(L.Eval(L.False)));
    }

    SECTION("should evaluate an unbound symbol to nil") {
        REQUIRE(L.Null(L.Eval(L.SymbolRef("a"))));
    }

    SECTION("should evaluate a bound symbol to the bound value") {
        auto env = L.MakeList({ L.Cons(L.SymbolRef("a"), L.MakeNumber(42)) });
        REQUIRE(L.NumVal(L.Eval(L.SymbolRef("a"), env)) == 42);
    }

    SECTION("should evaluate a string to itself") {
        REQUIRE(L.StringVal(L.Eval(L.MakeString("some string"))) == "some string");
    }

    SECTION("QUOTE") {
        REQUIRE(L.SymbolName(L.Eval(L.Read("(quote foo)"))) == "foo");
        REQUIRE(L.SymbolName(L.Eval(L.Read("'foo"))) == "foo");
    }

    SECTION("SUM") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(+)"))) == 0);
        REQUIRE(L.NumVal(L.Eval(L.Read("(+ 0)"))) == 0);
        REQUIRE(L.NumVal(L.Eval(L.Read("(+ 2)"))) == 2);
        REQUIRE(L.NumVal(L.Eval(L.Read("(+ 2 3)"))) == 5);
        REQUIRE(L.NumVal(L.Eval(L.Read("(+ 2 3 4)"))) == 9);
    }

    SECTION("SUM with subexpressions") {
        auto env = L.MakeList({
            L.Cons(L.SymbolRef("a"), L.MakeNumber(1)),
            L.Cons(L.SymbolRef("b"), L.MakeNumber(2)),
            L.Cons(L.SymbolRef("c"), L.MakeNumber(4))
        });
        REQUIRE(L.NumVal(L.Eval(L.Read("(+ (+ a b 8) 16 c)"), env)) == 31);
    }

    SECTION("DIFFERENCE") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(-)"))) == 0);
        REQUIRE(L.NumVal(L.Eval(L.Read("(- 0)"))) == 0);
        REQUIRE(L.NumVal(L.Eval(L.Read("(- 2)"))) == -2);
        REQUIRE(L.NumVal(L.Eval(L.Read("(- 5 2)"))) == 3);
        REQUIRE(L.NumVal(L.Eval(L.Read("(- 5 2 7)"))) == -4);
    }

    SECTION("PRODUCT") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(*)"))) == 1);
        REQUIRE(L.NumVal(L.Eval(L.Read("(* 0)"))) == 0);
        REQUIRE(L.NumVal(L.Eval(L.Read("(* 1)"))) == 1);
        REQUIRE(L.NumVal(L.Eval(L.Read("(* 2)"))) == 2);
        REQUIRE(L.NumVal(L.Eval(L.Read("(* 2 3)"))) == 6);
        REQUIRE(L.NumVal(L.Eval(L.Read("(* 2 3 4)"))) == 24);
    }

    SECTION("QUOTIENT") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(/)"))) == 1);
        REQUIRE(std::isinf(L.NumVal(L.Eval(L.Read("(/ 0)")))));
        REQUIRE(std::isinf(L.NumVal(L.Eval(L.Read("(/ 1 0)")))));
        REQUIRE(L.NumVal(L.Eval(L.Read("(/ 2)"))) == 0.5);
        REQUIRE(L.NumVal(L.Eval(L.Read("(/ 8 5)"))) == 1.6);
        REQUIRE(L.NumVal(L.Eval(L.Read("(/ 360 4 3)"))) == 30);
    }

    SECTION("should evaluate a LAMBDA expression to itself") {
        REQUIRE(L.PrintString(L.Eval(L.Read("(lambda (n) (+ n 1))"))) == "(lambda (n) (+ n 1))");
    }

    SECTION("LAMBDA call no args") {
        REQUIRE(L.NumVal(L.Eval(L.Read("((lambda () (+ 1 2)))"))) == 3);
    }

    SECTION("LAMBDA call 1 arg") {
        REQUIRE(L.NumVal(L.Eval(L.Read("((lambda (n) (+ n 1)) 1)"))) == 2);
    }

    SECTION("LAMBDA call 2 args") {
        REQUIRE(L.NumVal(L.Eval(L.Read("((lambda (m n) (+ m n 10)) 30 2)"))) == 42);
    }

    SECTION("LAMBDA should return the value of the last expression") {
        REQUIRE(L.Null(L.Eval(L.Read("((lambda ()))"))));
        REQUIRE(L.NumVal(L.Eval(L.Read("((lambda (n) n) 1)"))) == 1);
        REQUIRE(L.NumVal(L.Eval(L.Read("((lambda (n) 1 2 n) 3)"))) == 3);
        REQUIRE(L.NumVal(L.Eval(L.Read("((lambda () 1 2 3 (+ 40 2)))"))) == 42);
    }

    SECTION("PROGN should return the value of the last expression") {
        REQUIRE(L.Null(L.Eval(L.Read("(progn)"))));
        REQUIRE(L.NumVal(L.Eval(L.Read("(progn 1)"))) == 1);
        REQUIRE(L.NumVal(L.Eval(L.Read("(progn 1 2 3)"))) == 3);
        REQUIRE(L.NumVal(L.Eval(L.Read("(progn 1 2 3 (+ 40 2))"))) == 42);
    }

    SECTION("SETQ top level") {
        auto setqReturn = L.Eval(L.Read("(setq a 10)"));
        REQUIRE(L.NumVal(setqReturn) == 10);
        REQUIRE(L.NumVal(L.Eval(L.Read("a"))) == 10);
    }

    SECTION("SETQ in LAMBDA should modify the environment") {
        auto setqReturn = L.Eval(L.Read("(setq a 1)"));
        REQUIRE(L.NumVal(setqReturn) == 1);
        L.Eval(L.Read("(setq f (lambda (a) (progn (setq b a) (setq a 3) (setq c a))))"));
        REQUIRE(L.NumVal(L.Eval(L.Read("(f 2)"))) == 3);
        REQUIRE(L.NumVal(L.Eval(L.Read("a"))) == 1);
        REQUIRE(L.NumVal(L.Eval(L.Read("b"))) == 2);
        REQUIRE(L.NumVal(L.Eval(L.Read("c"))) == 3);
    }

    SECTION("DEF no args") {
        auto defReturn = L.Eval(L.Read("(def f () (+ 1 2))"));
        REQUIRE(L.TypeOf(defReturn) == procdraw::LispObjectType::Cons);
        REQUIRE(L.SymbolName(L.Car(defReturn)) == "lambda");
        REQUIRE(L.NumVal(L.Eval(L.Read("(f)"))) == 3);
    }

    SECTION("DEF 1 arg") {
        auto defReturn = L.Eval(L.Read("(def f (n) (+ n 1))"));
        REQUIRE(L.TypeOf(defReturn) == procdraw::LispObjectType::Cons);
        REQUIRE(L.SymbolName(L.Car(defReturn)) == "lambda");
        REQUIRE(L.NumVal(L.Eval(L.Read("(f 1)"))) == 2);
    }

    SECTION("APPLY no args") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(apply (lambda () (+ 1 2)) (quote ()))"))) == 3);
    }

    SECTION("APPLY 1 arg") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(apply (lambda (n) (+ n 1)) (quote (1)))"))) == 2);
    }

    SECTION("APPLY 2 args") {
        REQUIRE(L.NumVal(L.Eval(L.Read("(apply (lambda (m n) (+ m n 10)) (quote (30 2)))"))) == 42);
    }

    SECTION("CAR") {
        REQUIRE(L.Null(L.Eval(L.Read("(car nil)"))));
        REQUIRE(L.NumVal(L.Eval(L.Read("(car (quote (1 . 2)))"))) == 1);
    }

    SECTION("CDR") {
        REQUIRE(L.Null(L.Eval(L.Read("(cdr nil)"))));
        REQUIRE(L.NumVal(L.Eval(L.Read("(cdr (quote (1 . 2)))"))) == 2);
    }

    SECTION("EQ") {
        REQUIRE(L.BoolVal(L.Eval(L.Read("(eq 42 42)"))));
        REQUIRE_FALSE(L.BoolVal(L.Eval(L.Read("(eq 1 2)"))));
    }

    SECTION("IF") {
        auto env = L.MakeList({
            L.Cons(L.SymbolRef("a"), L.MakeNumber(1)),
            L.Cons(L.SymbolRef("b"), L.MakeNumber(2))
        });

        SECTION("true with else") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(if (eq 42 42) (setq a 10) (setq b 20))"), env)) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("a"), env)) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("b"), env)) == 2);
        }

        SECTION("true without else") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(if (eq 42 42) (setq a 10))"), env)) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("a"), env)) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("b"), env)) == 2);
        }

        SECTION("false with else") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(if (eq 1 2) (setq a 10) (setq b 20))"), env)) == 20);
            REQUIRE(L.NumVal(L.Eval(L.Read("a"), env)) == 1);
            REQUIRE(L.NumVal(L.Eval(L.Read("b"), env)) == 20);
        }

        SECTION("false without else") {
            REQUIRE(L.Null(L.Eval(L.Read("(if (eq 1 2) (setq a 10))"), env)));
            REQUIRE(L.NumVal(L.Eval(L.Read("a"), env)) == 1);
            REQUIRE(L.NumVal(L.Eval(L.Read("b"), env)) == 2);
        }
    }

    SECTION("Recursion") {
        auto exp = "(progn"
                   "  (setq f (lambda (n)"
                   "    (if (eq n 0)"
                   "      1"
                   "      (* n (f (- n 1))))))"
                   "  (f 5))";
        REQUIRE(L.NumVal(L.Eval(L.Read(exp))) == 120);
    }

    SECTION("LERP") {

        SECTION("should interpolate values for [0, 8]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 0 8 0)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 0 8 (/ 4))"))) == 2.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 0 8 (/ 2))"))) == 4.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 0 8 (/ 3 4))"))) == 6.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 0 8 1)"))) == 8.0);
        }

        SECTION("should interpolate values for [4, -4]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 4 -4 0)"))) == 4.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 4 -4 (/ 4))"))) == 2.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 4 -4 (/ 2 ))"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 4 -4 (/ 3 4))"))) == -2.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(lerp 4 -4 1)"))) == -4.0);
        }

    }

    SECTION("MAP-RANGE") {

        SECTION("should map values from [0, 10] to [-1, 0]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 -1 0 0)"))) == -1.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 -1 0 (/ 10 4))"))) == -0.75);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 -1 0 5)"))) == -0.5);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 -1 0 (/ 30 4))"))) == -0.25);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 -1 0 10)"))) == 0.0);
        }

        SECTION("should map values from [0, 10] to [1, -1]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 1 -1 0)"))) == 1.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 1 -1 (/ 10 4))"))) == 0.5);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 1 -1 5)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 1 -1 (/ 30 4))"))) == -0.5);
            REQUIRE(L.NumVal(L.Eval(L.Read("(map-range 0 10 1 -1 10)"))) == -1.0);
        }

    }

    SECTION("NORM") {

        SECTION("should normalize values for [0, 8]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 0 8 0)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 0 8 2)"))) == 0.25);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 0 8 4)"))) == 0.5);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 0 8 6)"))) == 0.75);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 0 8 8)"))) == 1.0);
        }

        SECTION("should normalize values for [4, -4]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 4 -4 4)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 4 -4 2)"))) == 0.25);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 4 -4 0)"))) == 0.5);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 4 -4 -2)"))) == 0.75);
            REQUIRE(L.NumVal(L.Eval(L.Read("(norm 4 -4 -4)"))) == 1.0);
        }

    }

    SECTION("WRAP") {

        SECTION("should wrap values for [0, 10]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 0)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 10)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 20)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 -10)"))) == 0.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 8)"))) == 8.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 12)"))) == 2.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 23)"))) == 3.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 -2)"))) == 8.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap 0 10 -13)"))) == 7.0);
        }

        SECTION("should wrap values for [-20, -10]") {
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -20)"))) == -20.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -10)"))) == -20.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 0)"))) == -20.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -30)"))) == -20.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -12)"))) == -12.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -8)"))) == -18.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 13)"))) == -17.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -22)"))) == -12.0);
            REQUIRE(L.NumVal(L.Eval(L.Read("(wrap -20 -10 -33)"))) == -13.0);
        }

    }

    SECTION("Tables") {
        L.Eval(L.Read("(setq t1 (make-table))"));

        SECTION("should provide getting and putting of key, value pairs") {
            REQUIRE(L.Null(L.Eval(L.Read("(get t1 'key1)"))));
            REQUIRE(L.NumVal(L.Eval(L.Read("(put t1 'key1 42)"))) == 42);
            REQUIRE(L.NumVal(L.Eval(L.Read("(get t1 'key1)"))) == 42);
            REQUIRE(L.NumVal(L.Eval(L.Read("(put t1 'key1 10)"))) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("(get t1 'key1)"))) == 10);
            REQUIRE(L.Null(L.Eval(L.Read("(get t1 'key2)"))));
            L.Eval(L.Read("(put t1 'key2 100)"));
            REQUIRE(L.NumVal(L.Eval(L.Read("(get t1 'key2)"))) == 100);
            REQUIRE(L.NumVal(L.Eval(L.Read("(get t1 'key1)"))) == 10);
            L.Eval(L.Read("(clear t1)"));
            REQUIRE(L.Null(L.Eval(L.Read("(get t1 'key1)"))));
            REQUIRE(L.Null(L.Eval(L.Read("(get t1 'key2)"))));
        }

        SECTION("keys") {
            REQUIRE(L.Null(L.Eval(L.Read("(keys t1)"))));

            L.Eval(L.Read("(put t1 'key1 42)"));
            REQUIRE(L.SymbolName(L.Car(L.Eval(L.Read("(keys t1)")))) == "key1");
            REQUIRE(L.Null(L.Cdr(L.Eval(L.Read("(keys t1)")))));
        }

        SECTION("should provide method calls") {
            auto exp = "(progn"
                       "  (put t1 'x 10)"
                       "  (put t1 'get-x"
                       "    (lambda (self) (get self 'x)))"
                       "  (put t1 'plus-x"
                       "    (lambda (self n) (+ (get self 'x) n))))";
            L.Eval(L.Read(exp));
            REQUIRE(L.NumVal(L.Eval(L.Read("((get t1 'get-x) t1)"))) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("((get t1 'plus-x) t1 2)"))) == 12);
            REQUIRE(L.NumVal(L.Eval(L.Read("('get-x t1)"))) == 10);
            REQUIRE(L.NumVal(L.Eval(L.Read("('plus-x t1 4)"))) == 14);
        }
    }

}

static int testCfunData = 42;

static procdraw::LispObjectPtr TestCfun(procdraw::LispInterpreter *L,
                                        procdraw::LispObjectPtr args,
                                        procdraw::LispObjectPtr env,
                                        void *data)
{
    REQUIRE(data == &testCfunData);
    int dataval = *(static_cast<int*>(data));
    REQUIRE(dataval == 42);
    return L->MakeNumber(L->NumVal(L->Car(args)) + dataval);
}

TEST_CASE("LispInterpreter::SetGlobalCFunction")
{
    procdraw::LispInterpreter L;

    SECTION("test-cfun should return the sum of its first argument and testCfunData") {
        L.SetGlobalCFunction("test-cfun", TestCfun, &testCfunData);
        REQUIRE(L.NumVal(L.Eval(L.Read("(test-cfun 10)"))) == 52);
    }
}