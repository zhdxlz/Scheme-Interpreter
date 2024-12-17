#ifndef PARSER 
#define PARSER

// parser of myscheme 

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include <map>
#include <cstring>
#include <iostream>
#define mp make_pair
using std :: string;
using std :: vector;
using std :: pair;

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Expr Syntax :: parse(Assoc &env) {}

Expr Number :: parse(Assoc &env) {
    return Expr(new Fixnum(n));
}

Expr Identifier :: parse(Assoc &env) {
    return Expr(new Var(s));
}

Expr TrueSyntax :: parse(Assoc &env) {
    return Expr(new True());
}

Expr FalseSyntax :: parse(Assoc &env) {
    return Expr(new False());
}

Expr List :: parse(Assoc &env) {
    //Empty List
    if (stxs.size()==0){
        //Constructor pass the pointer to a Shared_ptr,and the whole obj "Expr" be returned with its value.
        throw RuntimeError("Invalid Syntax.");
    }
    //Start with Text
    if (dynamic_cast<Identifier*>(stxs[0].get())){
        Identifier* pt=dynamic_cast<Identifier*>(stxs[0].get());
        //----Pure Number Calculation----
        if ((pt->s=="+"||pt->s=="-"||pt->s=="*")){
            if(stxs.size()!=3) throw RuntimeError("Illegal Number Calculation.");
            Expr _e1=stxs[1]->parse(env);
            Expr _e2=stxs[2]->parse(env);
            //Plus
            if (pt->s=="+"){
                return Expr(new Plus(_e1,_e2));
            }
            //Minus
            else if (pt->s=="-"){
                return Expr(new Minus(_e1,_e2));
            }
            //Multiplication
            else if (pt->s=="*"){
                return Expr(new Mult(_e1,_e2));
            }
        }
        //----Comparison----
        else if ((pt->s=="<"||pt->s=="<="||pt->s=="="||pt->s==">="||pt->s==">")){
            if(stxs.size()!=3) throw RuntimeError("Illegal Comparison.");
            Expr _e1=stxs[1]->parse(env);
            Expr _e2=stxs[2]->parse(env);
            if(!dynamic_cast<Fixnum*>(_e1.get())||!dynamic_cast<Fixnum*>(_e2.get())){
                throw RuntimeError("Illegal Comparison.");
            }
            if (pt->s=="<"){
                return Expr(new Less(_e1,_e2));
            }
            else if (pt->s=="<="){
                return Expr(new LessEq(_e1,_e2));
            }
            else if (pt->s=="="){
                return Expr(new Equal(_e1,_e2));
            }
            else if (pt->s==">"){
                return Expr(new Greater(_e1,_e2));
            }
            else if (pt->s==">="){
                return Expr(new GreaterEq(_e1,_e2));
            }
        }
        else if (pt->s=="not"){
            if (stxs.size()!=2) throw RuntimeError("Illegal ""Not"" operation.");
            Expr _e=stxs[1]->parse(env);
            return Expr(new Not(_e));
        }
        else if (pt->s=="fixnum?"||pt->s=="boolean?"||pt->s=="null?"||pt->s=="pair?"||pt->s=="symbol?"){
            if (stxs.size()!=2) throw RuntimeError("Illegal type judge.");
            Expr _e=stxs[1]->parse(env);
            if (pt->s=="fixnum?"){
                return Expr(new IsFixnum(_e));
                // if (dynamic_cast<Fixnum*>(_e.get()))
                //     return Expr(new True());
                // else
                //     return Expr(new False());
            }
            else if (pt->s=="boolean?")
                return Expr(new IsBoolean(_e));
            else if (pt->s=="null?")
                return Expr(new IsNull(_e));
            else if (pt->s=="pair?")
                return Expr(new IsPair(_e));
            else if (pt->s=="symbol?")
                return Expr(new IsSymbol(_e));

        }
        else if (pt->s=="eq?"){
            if (stxs.size()!=3) throw RuntimeError("Illegal ""eq?"" operation.");
            Expr _e1=stxs[1]->parse(env);
            Expr _e2=stxs[2]->parse(env);
            return Expr(new IsEq(_e1,_e2));
        }
        //----Exit----
        else if (pt->s=="exit"){
            return Expr(new Exit());
        }
        else if (pt->s=="void"){
            return Expr(new MakeVoid());
        }
        else if (pt->s=="quote"){
            if (stxs.size()!=2) throw RuntimeError("Illegal quote.");
            return Expr(new Quote(stxs[1]));
        }
        else if (pt->s=="begin"){
            vector<Expr> _t{};
            for (int i=1;i<stxs.size();i++){
                _t.push_back(stxs[i]->parse(env));
            }
            return Expr(new Begin(_t));
        }
        else if (pt->s=="if"){
            if (stxs.size()!=4) throw RuntimeError("Illegal if comparison");
            return Expr(new If(stxs[1]->parse(env),stxs[2]->parse(env),stxs[3]->parse(env)));
        }
        else if (pt->s=="cons"){
            if (stxs.size()!=3) throw RuntimeError("Illegal cons");
            return Expr(new Cons(stxs[1]->parse(env),stxs[2]->parse(env)));
        }
        else if (pt->s=="car"){
            if (stxs.size()!=2) throw RuntimeError("Illegal car");
            return Expr(new Car(stxs[1]->parse(env)));
        }
        else if (pt->s=="cdr"){
            if (stxs.size()!=2) throw RuntimeError("Illegal cdr");
            return Expr(new Cdr(stxs[1]->parse(env)));
        }
        else if (pt->s=="let"){
            
        }
        //else if (pt->)
        // //oops
        // //----An ordinary Pair----
        // //The latest Choice
        // else if (stxs.size()==2){
        //     return Expr(new Cons(stxs[0]->parse(env),stxs[1]->parse(env)));
        // }
        // //----An identifier----
        // //The latest Choise
        // else if (stxs.size()==1){
        //     return Expr(stxs[0]->parse(env));
        // }

        //----A pair----
        else{//oops
            List _t;
            _t.stxs.assign(stxs.begin()+1,stxs.end());
            return Expr(new Cons(stxs[0]->parse(env),_t.parse(env)));
        }
    }

}

#endif