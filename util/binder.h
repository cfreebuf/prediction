// CopyRight 2019 360. All rights reserved.
// File   binder.h
// Date   2019-10-28 15:02:51
// Brief Used to replace large switch-case codes
//        by using BindStaticFunction and BindMemberFunction

#ifndef PREDICTION_SERVER_UTIL_BINDER_H_
#define PREDICTION_SERVER_UTIL_BINDER_H_

#include <functional>

namespace prediction {
namespace util {

// Brief: Bind Static Functions
// Usage: BindStaticFunction<int, int, int, int> handler(sum);  
//        std::cout << "handler(1, 2, 3) : " << handler(1, 2, 3) << std::endl; 
template<typename R = int, typename... Args>
class BindStaticFunction {
 public:
  BindStaticFunction() {}
  BindStaticFunction(std::function<R(Args...)> handler)
      : handler_(handler) { }
  R operator() (Args... args) { return handler_(args...); } 

 private:  
  std::function<R(Args...)> handler_;  
};

// Brief: Bind Member Functions
// Usage: MyClass* my_class_ = new MyClass();
//        BindMemberFunction<MyClass, int, int> binder(my_class_, Foo);
//        int a, b;
//        binder(a, b); // Equals to my_class_->Foo(a, b);
template<typename Object, typename R = int, typename... Args>
class BindMemberFunction {
 public:
  BindMemberFunction(Object* object, R (Object::*method)(Args...))
      : object_(object) {
      handler_ = [object,method](Args... args) {
          return (*object.*method)(args...);
      };
  }
  R operator() (Args... args) { return handler_(args...); } 
 private:  
  Object* object_;
  std::function<R(Args...) > handler_;  
};

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_BINDER_H_
