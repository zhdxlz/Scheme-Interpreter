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

Value Let::eval(Assoc &env) {
    Assoc _e=env;
    for (int i=0;i<bind.size();i++){
        _e=extend(bind[i].first,bind[i].second->eval(env),_e);
    }
    return body->eval(_e);
} // let expression

Value Lambda::eval(Assoc &env) {
    return ClosureV(x,e,env);
} // lambda expression

Value Apply::eval(Assoc &e) {
    Value _v=rator->eval(e);//Evaluate the original form of the operator.
    //First search the Users defined functions(including represented by variable forms.)
    if (dynamic_cast<Closure*>(_v.get())){
        Closure* _c=dynamic_cast<Closure*>(_v.get());
        if (rand.size()!=_c->parameters.size()) throw RuntimeError("Illegal lambda call");
        Assoc _e=_c->env;
        for (int i=0;i<_c->parameters.size();i++){
            _e=extend(_c->parameters[i],rand[i]->eval(e),_e);
        }
        return _c->e->eval(_e);
    }
    //Then consider the default primitives
    if (dynamic_cast<Primi*>(_v.get())){
        Primi* _p=dynamic_cast<Primi*>(_v.get());

        //----Pure Number Calculation----
        if ((_p->_e==E_PLUS||_p->_e==E_MINUS||_p->_e==E_MUL)){
            if(rand.size()!=2) throw RuntimeError("Illegal Number Calculation.");
            //Plus
            if (_p->_e==E_PLUS){
                return Expr(new Plus(rand[0],rand[1]))->eval(e);
            }
            //Minus
            else if (_p->_e==E_MINUS){
                return Expr(new Minus(rand[0],rand[1]))->eval(e);
            }
            //Multiplication
            else if (_p->_e==E_MUL){
                return Expr(new Mult(rand[0],rand[1]))->eval(e);
            }
        }
        //----Comparison----
        else if (_p->_e==E_LT||_p->_e==E_LE||_p->_e==E_EQ||_p->_e==E_GE||_p->_e==E_GT){
            if(rand.size()!=2) throw RuntimeError("Illegal Comparison.");
            if (_p->_e==E_LT){
                return Expr(new Less(rand[0],rand[1]))->eval(e);
            }
            if (_p->_e==E_LE){
                return Expr(new LessEq(rand[0],rand[1]))->eval(e);
            }
            if (_p->_e==E_EQ){
                return Expr(new Equal(rand[0],rand[1]))->eval(e);
            }
            if (_p->_e==E_GE){
                return Expr(new GreaterEq(rand[0],rand[1]))->eval(e);
            }
            if (_p->_e==E_GT){
                return Expr(new Greater(rand[0],rand[1]))->eval(e);
            }
        }
        else if (_p->_e==E_NOT){
            if (rand.size()!=1) throw RuntimeError("Illegal ""Not"" operation.");
            return Expr(new Not(rand[0]))->eval(e);
        }
        else if (_p->_e==E_INTQ||_p->_e==E_BOOLQ||_p->_e==E_NULLQ||_p->_e==E_PAIRQ||_p->_e==E_SYMBOLQ||_p->_e==E_PROCQ){
            if (rand.size()!=1) throw RuntimeError("Illegal type judge.");
            if (_p->_e==E_INTQ)
                return Expr(new IsFixnum(rand[0]))->eval(e);
            else if (_p->_e==E_BOOLQ)
                return Expr(new IsBoolean(rand[0]))->eval(e);
            else if (_p->_e==E_NULLQ)
                return Expr(new IsNull(rand[0]))->eval(e);
            else if (_p->_e==E_PAIRQ)
                return Expr(new IsPair(rand[0]))->eval(e);
            else if (_p->_e==E_SYMBOLQ)
                return Expr(new IsSymbol(rand[0]))->eval(e);
            else if (_p->_e==E_PROCQ)
                return Expr(new IsProcedure(rand[0]))->eval(e);

        }
        else if (_p->_e==E_EQQ){
            if (rand.size()!=2) throw RuntimeError("Illegal ""eq?"" operation.");
            return Expr(new IsEq(rand[0],rand[1]))->eval(e);
        }
        //----Exit----
        else if (_p->_e==E_EXIT){
            if (rand.size()!=0) throw RuntimeError("Illegal ""exit"" operation.");
            return Expr(new Exit())->eval(e);
        }
        else if (_p->_e==E_VOID){
            if (rand.size()!=0) throw RuntimeError("Illegal ""void"" operation.");
            return Expr(new MakeVoid())->eval(e);
        }
        else if (_p->_e==E_CONS){
            if (rand.size()!=2) throw RuntimeError("Illegal cons");
            return Expr(new Cons(rand[0],rand[1]))->eval(e);
        }
        else if (_p->_e==E_CAR){
            if (rand.size()!=1) throw RuntimeError("Illegal car");
            return Expr(new Car(rand[0]))->eval(e);
        }
        else if (_p->_e==E_CDR){
            if (rand.size()!=1) throw RuntimeError("Illegal cdr");
            return Expr(new Cdr(rand[0]))->eval(e);
        }
    }
    //Then consider the Reserved_words
    else if (dynamic_cast<Reser*>(_v.get())){
        Reser* _p=dynamic_cast<Reser*>(_v.get());
        if (_p->_e==E_BEGIN){
            std::vector<Expr> _t{};
            for (int i=0;i<rand.size();i++){
                _t.push_back(rand[i]);
            }
            return Expr(new Begin(_t))->eval(e);
        }
        else if (_p->_e==E_QUOTE){//oops?
            if (rand.size()!=1) throw RuntimeError("Illegal quote.");
            return Expr(new Quote(syn[0]))->eval(e);
        }
        else if (_p->_e==E_IF){
            if (rand.size()!=3) throw RuntimeError("Illegal if comparison");
            return Expr(new If(rand[0],rand[1],rand[2]))->eval(e);
        }
        else if (_p->_e==E_LET){
            if (rand.size()!=2) throw RuntimeError("Illegal Let Size.");
            if (!dynamic_cast<List*>(syn[0].get())) throw RuntimeError("Illegal Let Definition(outside).");
            List* _d=dynamic_cast<List*>(syn[0].get());
            std::vector<std::pair<std::string, Expr>> _v{};
            //The variable assignments
            for (int i=0;i<_d->stxs.size();i++){
                if (!(dynamic_cast<List*>(_d->stxs[i].get()))) throw RuntimeError("Illegal Let Definition(inside).");
                List* _t=dynamic_cast<List*>(_d->stxs[i].get());
                if (_t->stxs.size()!=2) throw RuntimeError("Illegal Let Definition(syntax).");
                Expr _ee=_t->stxs[1].get()->parse(e);
                if (!dynamic_cast<Identifier*>(_t->stxs[0].get())) throw RuntimeError("Illegal Let definition(name)");
                _v.push_back(std::make_pair(dynamic_cast<Identifier*>(_t->stxs[0].get())->s,_ee));
            }
            return Expr(new Let(_v,rand[1]))->eval(e);
        }
        else if (_p->_e==E_LAMBDA){
            if (rand.size()!=2) throw RuntimeError("Illegal Lambda Size.");
            if (!dynamic_cast<List*>(syn[0].get())) throw RuntimeError("Illegal lambda");
            List* _d=dynamic_cast<List*>(syn[0].get());
            std::vector<std::string> _v{};
            for (int i=0;i<_d->stxs.size();i++){
                if (!dynamic_cast<Identifier*>(_d->stxs[i].get())) throw RuntimeError("Illegal lambda");
                Identifier* _i=dynamic_cast<Identifier*>(_d->stxs[i].get());
                _v.push_back(_i->s);
            }
            return Expr(new Lambda(_v,rand[1]))->eval(e);
        }
        else if (_p->_e==E_LETREC){
            if (rand.size()!=2) throw RuntimeError("Illegal Letrec Size.");
            if (!dynamic_cast<List*>(syn[0].get())) throw RuntimeError("Illegal Letrec Definition(outside).");
            List* _d=dynamic_cast<List*>(syn[0].get());
            std::vector<std::pair<std::string, Expr>> _v{};
            //The variable assignments
            for (int i=0;i<_d->stxs.size();i++){
                if (!(dynamic_cast<List*>(_d->stxs[i].get()))) throw RuntimeError("Illegal Letrec Definition(inside).");
                List* _t=dynamic_cast<List*>(_d->stxs[i].get());
                if (_t->stxs.size()!=2) throw RuntimeError("Illegal Letrec Definition(syntax).");
                Expr _ee=_t->stxs[1].get()->parse(e);
                if (!dynamic_cast<Identifier*>(_t->stxs[0].get())) throw RuntimeError("Illegal Letrec definition(name)");
                _v.push_back(std::make_pair(dynamic_cast<Identifier*>(_t->stxs[0].get())->s,_ee));
            }
            return Expr(new Letrec(_v,rand[1]))->eval(e);
        }
    }
    throw RuntimeError("Illegal Apply.");
} // for function calling

Value Letrec::eval(Assoc &env) {
    Assoc _e=env;
    for (int i=0;i<bind.size();i++){
        _e=extend(bind[i].first,Value(nullptr),_e);
    }
    std::vector<Value> _v;
    for (int i=0;i<bind.size();i++){
        _v.push_back(bind[i].second->eval(_e));
    }
    for (int i=0;i<bind.size();i++){
        modify(bind[i].first,_v[i],_e);
    }
    return body->eval(_e);
} // letrec expression

Value Var::eval(Assoc &e) {
    //Shadow: First Find in assoc list
    Value _v=find(x,e);
    if (_v.get()!=nullptr){
        return _v;
    }
    //Then find in defualt list
    auto _pr=primitives.find(x);//For Primitives
    if (_pr!=primitives.end()){
        return PrimiV(_pr->second);
    }
    auto _re=reserved_words.find(x);//For Reserved_words
    if (_re!=reserved_words.end()){
        return ReserV(_re->second);
    }
    throw RuntimeError("Undefined variable. ("+x+")");
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
    if (es.size()==0) 
        throw RuntimeError("Illegal Begin(Insufficient expr)");
    for (int i=0;i<es.size()-1;i++){
        es[i].get()->eval(e);
    }
    return es[es.size()-1].get()->eval(e);
} // begin expression

Value Quote::eval(Assoc &e) {//oops
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
        }
        //Length 1 (Guaranteed not ".")
        else if (_s==1){
            if (dynamic_cast<Identifier*>(_t->stxs[0].get())&&dynamic_cast<Identifier*>(_t->stxs[0].get())->s==".") throw RuntimeError("Invalid Syntax List.(len=1 identi=.)");
            //return Expr(new Quote(_t->stxs[0]))->eval(e);
            return PairV(Expr(new Quote(_t->stxs[0]))->eval(e),NullV());
        }
        //Start with "." Ignore it (Guaranteed _s=2 and there must be 1 syntax after it)
        else if (dynamic_cast<Identifier*>(_t->stxs[0].get())&&dynamic_cast<Identifier*>(_t->stxs[0].get())->s=="."){
            if (_s!=2) throw RuntimeError("Invalid Syntax List.(len!=2 identi=.)");
            return Expr(new Quote(_t->stxs[1]))->eval(e);
        }
        //_s=2 and no "."(Guaranteed by the above if judgement)
        else if (_s==2){
            return PairV(Expr(new Quote(_t->stxs[0]))->eval(e),PairV((Expr(new Quote(_t->stxs[1]))->eval(e)),NullV()));
        }
        //Length >2 need recursive construction
        else{
            std::vector<Syntax> _s(_t->stxs.begin()+1,_t->stxs.end());
            List* _l=new List();
            _l->stxs=_s;
            return PairV(Expr(new Quote(_t->stxs[0]))->eval(e),Expr(new Quote(Syntax(_l)))->eval(e));
        }
    }
    else{
        throw RuntimeError("Illegal Quote type.");
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
    return BooleanV(
        (dynamic_cast<Closure*>(rand.get()))!=nullptr
        ||
        (dynamic_cast<Primi*>(rand.get()))!=nullptr
    );
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

Value Empt::eval(Assoc &e){
    return NullV();
}