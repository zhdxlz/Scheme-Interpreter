#ifndef SHAREDPTR
#define SHAREDPTR
#include<utility>
template<class T>
class Ctrl{//won't destruct along with the shared_ptr
public:
    int w_ctt;//Weak_ptr number
    int s_ctt;//Shared_ptr number
    Ctrl():s_ctt(1),w_ctt(0){
        
    }
    ~Ctrl(){

    }
    void releasse(){
        if (!w_ctt&&!s_ctt)
            delete this;
    }
};
template<class T>
class SharedPtr{
public:
    T* _p;//指针：指向实际内容的地址
    //int* _ctt;//指针：指向_ctt储存(该指针被储存的次数)的地址
    Ctrl<T>* _ctrl;//指针，指向控制块
    //普通构造函数：_p赋值为传入函数，_ctt赋值为指向1的指针(出现了1次)
    void releasse(){
        if (_ctrl) 
        {
            --(_ctrl->s_ctt);
            if((_ctrl->s_ctt)==0){
                if (_p) delete _p;
                _ctrl->releasse();
            }
            _p=nullptr;
            _ctrl=nullptr;
        }
    }
    //SharedPtr():_p(nullptr),_ctrl(nullptr){};//if get in a nullptr(won't implement this function)
    SharedPtr(T* pt=nullptr):_p(pt),_ctrl(nullptr){
        if (pt) _ctrl=new Ctrl<T>;
    }
    ~SharedPtr(){
        releasse();
    }
    SharedPtr(const SharedPtr<T> &other){//左值拷贝构造(_p一定nullptr,不需要release)
        _p=other._p;
        _ctrl=other._ctrl;
        if (_ctrl) ++(_ctrl->s_ctt);
    }
    SharedPtr<T> &operator=(const SharedPtr<T> &other){//左值拷贝赋值(_p不一定nullptr)
        if (this==&other) return *this;

        releasse();
        _p=other._p;
        _ctrl=other._ctrl;
        if (_ctrl) ++(_ctrl->s_ctt);
        return *this;
    }
    SharedPtr(SharedPtr<T> &&other){//右值移动构造(_p一定nullptr，不需要release)
        _p=other._p;
        _ctrl=other._ctrl;
        other._p=nullptr;//保证临时对象的析构函数不会改变计数器
        other._ctrl=nullptr;
    }
    SharedPtr<T> &operator=(SharedPtr<T> &&other){//右值移动赋值(_p不一定nullpptr) //使用引用加快速度
        if (this==&other) return *this;

        releasse();
        _p=other._p;
        _ctrl=other._ctrl;
        other._p=nullptr;//保证临时对象的析构函数不会改变计数器
        other._ctrl=nullptr;
        return *this;
    }
    int use_count() const{
        if (_ctrl)
            return _ctrl->s_ctt;
        else
            return 0;
    }
    T* get() const{
        return _p;
    }
    T &operator*() const{//返回的是指针，不然对对象操作时原对象没有变化
        return *_p;
    }
    T* operator->() const{
        return _p;
    }
    operator bool() const{
        return (_p!=nullptr);
    }
    void reset(T* other=nullptr){//imple this with the argument (nullptr)
        releasse();
        
        _p=other;
        if (other)
            _ctrl=new Ctrl<T>;
    }
};
template <typename T, typename... Args>
SharedPtr<T> make_shared(Args&&... args) {//函数返回值为右值，可以赋值
    //新建一个sharedptr，指针指向新建的T类型对象，对象里含有args数据
    return SharedPtr<T>{new T{std::forward<Args>(args)...}};
}
#endif

#ifndef WEAKPTR
#define WEAKPTR
template<class T>
class WeakPtr{
public:
    T* _p;//The address of pointer stored by the SharedPtr.
    Ctrl<T>* _ctrl;//The address of control block
    WeakPtr():_p(nullptr),_ctrl(nullptr){}
    WeakPtr(const SharedPtr<T> &sptr):_p(sptr._p),_ctrl(sptr._ctrl){
        if (_ctrl)
            ++(_ctrl->w_ctt);
    }
    WeakPtr(const WeakPtr<T> &other):_p(other._p),_ctrl(other._ctrl){
        if (_ctrl)
            ++(_ctrl->w_ctt);
    }
    WeakPtr(WeakPtr<T> &&other){
        _p=other._p;
        _ctrl=other._ctrl;
        other._p=nullptr;
        other._ctrl=nullptr;
    }
    ~WeakPtr(){
        if (_ctrl){
            --(_ctrl->w_ctt);
            _ctrl->releasse();
            _p=nullptr;
            _ctrl=nullptr;
        }
    }
    WeakPtr<T> &operator=(const WeakPtr<T> &other){//check self-assignment
        if (this==&other) return *this;
        if (_ctrl) {
            --(_ctrl->w_ctt);
            _ctrl->releasse();
        }
        _p = other._p;
        _ctrl = other._ctrl;
        ++(_ctrl->w_ctt);
        return *this;
    }
    WeakPtr<T> &operator=(const SharedPtr<T> &other){
        if (_ctrl) {
            --(_ctrl->w_ctt);
            _ctrl->releasse();
        }
        _p = other._p;
        _ctrl = other._ctrl;
        ++(_ctrl->w_ctt);
        return *this;
    }
    WeakPtr<T> &operator=(WeakPtr<T> &&other){
        if (this==&other) return *this;
        if (_ctrl) {
            --(_ctrl->w_ctt);
            _ctrl->releasse();
        }
        _p = other._p;
        _ctrl = other._ctrl;
        other._p = nullptr;
        other._ctrl = nullptr;
        return *this;
    }
    SharedPtr<T> lock(){
        if (!expired()){//The former shared_ptr might no longer exist.
            //return SharedPtr<T>(_p); error:another control block
            SharedPtr<T> backshared;//No with argument(otherwise memory leak with the newed _ctrl)
            backshared._p=_p;
            backshared._ctrl=_ctrl;
            ++(_ctrl->s_ctt);
            return backshared;
        }
        else{
            return SharedPtr<T>();
        }
    }
    bool expired() const{
        return !_ctrl||((_ctrl->s_ctt)==0);
    }
    int use_count() const{
        if (_ctrl) return _ctrl->s_ctt;
        else return 0;
    }
    void reset(){
        if (!_ctrl) return;

        --(_ctrl->w_ctt);
        _ctrl->releasse();//check whether to release every time the w_ctt reduce.
        _p=nullptr;
        _ctrl=nullptr;
    }
    void swap(WeakPtr<T> &other){//passing by reference
        T* tmpt=other._p;
        other._p=_p;
        _p=tmpt;

        Ctrl<T>* tmpc=other._ctrl;
        other._ctrl=_ctrl;
        _ctrl=tmpc;
    }
};
#endif