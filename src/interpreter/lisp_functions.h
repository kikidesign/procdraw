#pragma once

#include "lisp_interpreter.h"

namespace procdraw {

    LispObjectPtr lisp_Apply(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Assoc(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Car(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Cdr(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Cons(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Difference(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Eq(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Functionp(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Lerp(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_MapRange(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Memb(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Norm(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Not(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Product(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Putassoc(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Quotient(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Sum(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);
    LispObjectPtr lisp_Wrap(LispInterpreter *L, LispObjectPtr args, LispObjectPtr env, void *data);

}
