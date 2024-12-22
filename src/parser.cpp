#ifndef PARSER 
#define PARSER

// parser of myscheme 

#include "RE.hpp"
#include "Def.hpp"
#include "syntax.hpp"
#include "expr.hpp"
#include "value.hpp"
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
    //vector<Expr> _v{};
    // for (int i=1;i<=stxs.size();i++){
    //     _v.push_back(stxs[i]->parse(env));
    // }
    //return Expr(new Apply(stxs[1]->parse(env),_v));
    //Don't know how to process the function name before the evaluated.
    //Start with Text
    if (dynamic_cast<Identifier*>(stxs[0].get())){
        Identifier* pt=dynamic_cast<Identifier*>(stxs[0].get());
        if (pt->s=="begin"){
            vector<Expr> _t{};
            for (int i=1;i<stxs.size();i++){
                _t.push_back(stxs[i]->parse(env));
            }
            return Expr(new Begin(_t));
        }
        else if (pt->s=="quote"){
            if (stxs.size()!=2) throw RuntimeError("Illegal quote.");
            return Expr(new Quote(stxs[1]));
        }
        else if (pt->s=="if"){
            if (stxs.size()!=4) throw RuntimeError("Illegal if comparison");
            return Expr(new If(stxs[1]->parse(env),stxs[2]->parse(env),stxs[3]->parse(env)));
        }
        else if (pt->s=="let"){
            if (stxs.size()!=3) throw RuntimeError("Illegal Let Size.");
            if (!dynamic_cast<List*>(stxs[1].get())) throw RuntimeError("Illegal Let Definition(outside).");
            List* _d=dynamic_cast<List*>(stxs[1].get());
            std::vector<std::pair<std::string, Expr>> _v{};
            //The variable assignments
            for (int i=0;i<_d->stxs.size();i++){
                if (!(dynamic_cast<List*>(_d->stxs[i].get()))) throw RuntimeError("Illegal Let Definition(inside).");
                List* _t=dynamic_cast<List*>(_d->stxs[i].get());
                if (_t->stxs.size()!=2) throw RuntimeError("Illegal Let Definition(syntax).");
                Expr _ee=_t->stxs[1].get()->parse(env);
                if (!dynamic_cast<Identifier*>(_t->stxs[0].get())) throw RuntimeError("Illegal Let definition(name)");
                _v.push_back(std::make_pair(dynamic_cast<Identifier*>(_t->stxs[0].get())->s,_ee));
            }
            return Expr(new Let(_v,stxs[2].get()->parse(env)));
        }
        else if (pt->s=="lambda"){
            if (stxs.size()!=3) throw RuntimeError("Illegal Letrec Size.");
            if (!dynamic_cast<List*>(stxs[1].get())) throw RuntimeError("Illegal lambda");
            List* _d=dynamic_cast<List*>(stxs[1].get());
            std::vector<std::string> _v{};
            for (int i=0;i<_d->stxs.size();i++){
                if (!dynamic_cast<Identifier*>(_d->stxs[i].get())) throw RuntimeError("Illegal lambda");
                Identifier* _i=dynamic_cast<Identifier*>(_d->stxs[i].get());
                _v.push_back(_i->s);
            }
            return Expr(new Lambda(_v,stxs[2].get()->parse(env)));
        }
        else if (pt->s=="letrec"){
            if (stxs.size()!=3) throw RuntimeError("Illegal Letrec Size.");
            if (!dynamic_cast<List*>(stxs[1].get())) throw RuntimeError("Illegal Letrec Definition(outside).");
            List* _d=dynamic_cast<List*>(stxs[1].get());
            std::vector<std::pair<std::string, Expr>> _v{};
            //The variable assignments
            for (int i=0;i<_d->stxs.size();i++){
                if (!(dynamic_cast<List*>(_d->stxs[i].get()))) throw RuntimeError("Illegal Letrec Definition(inside).");
                List* _t=dynamic_cast<List*>(_d->stxs[i].get());
                if (_t->stxs.size()!=2) throw RuntimeError("Illegal Letrec Definition(syntax).");
                Expr _ee=_t->stxs[1].get()->parse(env);
                if (!dynamic_cast<Identifier*>(_t->stxs[0].get())) throw RuntimeError("Illegal Letrec definition(name)");
                _v.push_back(std::make_pair(dynamic_cast<Identifier*>(_t->stxs[0].get())->s,_ee));
            }
            return Expr(new Letrec(_v,stxs[2].get()->parse(env)));
        }
        else{
            vector<Expr> _t{};
            for (int i=1;i<stxs.size();i++){
                _t.push_back(stxs[i]->parse(env));
            }
            return Expr(new Apply(stxs[0]->parse(env),_t));
        }
    }
    else{
        vector<Expr> _t{};
        for (int i=1;i<stxs.size();i++){
            _t.push_back(stxs[i]->parse(env));
        }
        return Expr(new Apply(stxs[0]->parse(env),_t));
    }
}

#endif