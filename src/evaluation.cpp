#include "Def.hpp"
#include "value.hpp"
#include "expr.hpp"
#include "RE.hpp"
#include "syntax.hpp"
#include <cstring>
#include <vector>
#include <map>

extern std :: map<std :: string, ExprType> primitives;
extern std :: map<std :: string, ExprType> reserved_words;

Value Let::eval(Assoc &env) {} // let expression

Value Lambda::eval(Assoc &env) {} // lambda expression

Value Apply::eval(Assoc &e) {} // for function calling

Value Letrec::eval(Assoc &env) {} // letrec expression

Value Var::eval(Assoc &e) {

} // evaluation of variable

Value Fixnum::eval(Assoc &e) {
    return IntegerV(n);
} // evaluation of a fixnum

Value If::eval(Assoc &e) {
    //Return "Alter" except for "#f"
    Value _v=cond.get()->eval(e);
    if (dynamic_cast<Boolean*>(_v.get())){
        Boolean* _b=dynamic_cast<Boolean*>(_v.get());
        if (_b->b==false)
            return alter.get()->eval(e);
    }
    return conseq.get()->eval(e);
    
} // if expression

Value True::eval(Assoc &e) {
    return BooleanV(true);
} // evaluation of #t

Value False::eval(Assoc &e) {
    return BooleanV(false);
} // evaluation of #f

Value Begin::eval(Assoc &e) {
    if (es.size()==0) return NullV();
    for (int i=0;i<es.size()-1;i++){
        es[i].get()->eval(e);
    }
    return es[es.size()-1].get()->eval(e);
} // begin expression

Value Quote::eval(Assoc &e) {
    if (dynamic_cast<Number*>(s.get())){
        Number* _t=dynamic_cast<Number*>(s.get());
        return IntegerV(_t->n);
    }
    else if (dynamic_cast<TrueSyntax*>(s.get())){
        return BooleanV(true);
    }
    else if (dynamic_cast<FalseSyntax*>(s.get())){
        return BooleanV(false);
    }
    else if (dynamic_cast<Identifier*>(s.get())){
        Identifier* _i=dynamic_cast<Identifier*>(s.get());
        return SymbolV(_i->s);
    }
    else if (dynamic_cast<List*>(s.get())){
        List* _t=dynamic_cast<List*>(s.get());
        int _s=_t->stxs.size();
        //Empty list
        if (_s==0){
            return NullV();
            //return PairV(NullV(),NullV());
        }
        //Start with "." Ignore it
        else if (dynamic_cast<Identifier*>(_t->stxs[0].get())&&dynamic_cast<Identifier*>(_t->stxs[0].get())->s=="."){
            std::vector<Syntax> _s(_t->stxs.begin()+1,_t->stxs.end());
            List* _l=new List();
            _l->stxs=_s;
            return Expr(new Quote(_l))->eval(e);
        }
        //Length 1 and not "."
        else if (_s==1){
            //return PairV(Expr(new Quote(_t->stxs[0]))->eval(e),NullV());
            return Expr(new Quote(_t->stxs[0]))->eval(e);
        }
        //Length >=2 need recursive construction
        else{
            std::vector<Syntax> _s(_t->stxs.begin()+1,_t->stxs.end());
            List* _l=new List();
            _l->stxs=_s;
            return PairV(Expr(new Quote(_t->stxs[0]))->eval(e),Expr(new Quote(Syntax(_l)))->eval(e));
        }
        
    }
} // quote expression

Value MakeVoid::eval(Assoc &e) {
    return VoidV();
} // (void)

Value Exit::eval(Assoc &e) {
    return TerminateV();
} // (exit)

Value Binary::eval(Assoc &e) {
    //Recursively  Getting the value
    //Ultimately there will be a unit charcter
    return evalRator(rand1->eval(e),rand2->eval(e));
} // evaluation of two-operators primitive

Value Unary::eval(Assoc &e) {
    return evalRator(rand->eval(e));
} // evaluation of single-operator primitive

Value Mult::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return IntegerV((_r1->n)*(_r2->n));
    else
        throw RuntimeError("Illegal Number Calculation.");
    
} // *

Value Plus::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return Value(new Integer((_r1->n)+(_r2->n)));
    else
        throw RuntimeError("Illegal Number Calculation.");
    
} // +

Value Minus::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return Value(new Integer((_r1->n)-(_r2->n)));
    else
        throw RuntimeError("Illegal Number Calculation.");

} // -

Value Less::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return BooleanV((_r1->n)<(_r2->n));
    else
        throw RuntimeError("Illegal Number Comparison.");
} // <

Value LessEq::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return BooleanV((_r1->n)<=(_r2->n));
    else
        throw RuntimeError("Illegal Number Comparison.");
} // <=

Value Equal::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return BooleanV((_r1->n)==(_r2->n));
    else
        throw RuntimeError("Illegal Number Comparison.");
} // =

Value GreaterEq::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return BooleanV((_r1->n)>=(_r2->n));
    else
        throw RuntimeError("Illegal Number Comparison.");    
} // >=

Value Greater::evalRator(const Value &rand1, const Value &rand2) {
    Integer* _r1=dynamic_cast<Integer*>(rand1.get());
    Integer* _r2=dynamic_cast<Integer*>(rand2.get());
    if (_r1&&_r2)
        return BooleanV((_r1->n)>(_r2->n));
    else
        throw RuntimeError("Illegal Number Comparison.");
} // >

Value Cons::evalRator(const Value &rand1, const Value &rand2) {
    return PairV(rand1,rand2);
} // cons

Value IsEq::evalRator(const Value &rand1, const Value &rand2) {
    if(typeid(*rand1.get())==typeid(*rand2.get())){
        if (dynamic_cast<Integer*>(rand1.get())){
            return BooleanV(
                dynamic_cast<Integer*>(rand1.get())->n
                ==
                dynamic_cast<Integer*>(rand2.get())->n
                );
        }
        else if (dynamic_cast<Boolean*>(rand1.get())){
            return BooleanV(
                dynamic_cast<Boolean*>(rand1.get())->b
                ==
                dynamic_cast<Boolean*>(rand2.get())->b
                );
        }
        else if (dynamic_cast<Symbol*>(rand1.get())){
            return BooleanV(
                dynamic_cast<Symbol*>(rand1.get())->s
                ==
                dynamic_cast<Symbol*>(rand2.get())->s
                );
        }
        else if (dynamic_cast<Null*>(rand1.get())||dynamic_cast<Void*>(rand1.get())){
            return BooleanV(true);
        }
    }
    return BooleanV(rand1.get()==rand2.get());
} // eq?

Value IsBoolean::evalRator(const Value &rand) {
    return BooleanV((dynamic_cast<Boolean*>(rand.get()))!=nullptr);
} // boolean?

Value IsFixnum::evalRator(const Value &rand) {
    return BooleanV((dynamic_cast<Integer*>(rand.get()))!=nullptr);
} // fixnum?

Value IsSymbol::evalRator(const Value &rand) {
    return BooleanV((dynamic_cast<Symbol*>(rand.get()))!=nullptr);
} // symbol?

Value IsNull::evalRator(const Value &rand) {
    return BooleanV((dynamic_cast<Null*>(rand.get()))!=nullptr);
} // null?

Value IsPair::evalRator(const Value &rand) {
    return BooleanV((dynamic_cast<Pair*>(rand.get()))!=nullptr);
} // pair?

Value IsProcedure::evalRator(const Value &rand) {
    return BooleanV((dynamic_cast<Closure*>(rand.get()))!=nullptr);
} // procedure?

Value Not::evalRator(const Value &rand) {
    //Return "#f" except for "#f"
    if (dynamic_cast<Boolean*>(rand.get())){
        if ((dynamic_cast<Boolean*>(rand.get())->b)==false)
            return BooleanV(true);
    }
    return BooleanV(false);
} // not

Value Car::evalRator(const Value &rand) {
    if (!dynamic_cast<Pair*>(rand.get())) throw RuntimeError("Illegal car expr");
    Pair* _e=dynamic_cast<Pair*>(rand.get());
    return _e->car;
} // car

Value Cdr::evalRator(const Value &rand) {
    if (!dynamic_cast<Pair*>(rand.get())) throw RuntimeError("Illegal cdr expr");
    Pair* _e=dynamic_cast<Pair*>(rand.get());
    return _e->cdr;

} // cdr
