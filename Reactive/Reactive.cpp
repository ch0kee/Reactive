// Reactive.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <iostream>
#include <functional>
#include <vector>
#include <algorithm>
#include <type_traits>

class reactive_base {

};

namespace reactive {

  template<class T>
  class stream {
    std::vector< std::function<void(T)>>  _fs;
  protected:
    template<class U = T>
    typename std::enable_if<!std::is_void<U>::value, void>::type fire(U v) {
      std::for_each(_fs.begin(), _fs.end(), [&](std::function<void(T)> f) { f(v); });
    }

    template<class U = T>
    typename std::enable_if<std::is_void<U>::value, void>::type fire() {
      std::for_each(_fs.begin(), _fs.end(), [&](std::function<void(void)> f) { f(); });
    }
  public:

    void subscribe(std::function<void(T)> f)  {
      _fs.push_back(f);
    }

    template<class _Function>
    stream map(_Function f, stream to) {
     // _get_value = []() { f( }
    }
    
    template<class _Function>
    stream flatMap(_Function f, stream to) {
      // _get_value = []() { f( }
    }
  };

  namespace streams {
    template<class T>
    struct variable
      : public stream<T>
    {
      T __value__;
      operator T& () { return __value__; }
      operator const T& () const { return __value__; }
    };

    template<>
    struct variable<void>
      : public stream<void>
    {
    };


    struct clock
      : public reactive::stream<int>
    {
      double _sec_duration = 10.0;
      double _fps = 1.0;
    public:
      clock(double sec_duration, double fps)
        : _sec_duration(sec_duration), _fps(fps)
      {
      }


      void run(stream<void>& s) {
        const int sleep_interval = int( 1000 / _fps);
        const int iterations = int(_sec_duration * 1000. / sleep_interval);
        int t = 0;
        for (int i = 0; i < iterations; ++i) {
          system("cls");
          fire(i);
          s();
          Sleep(sleep_interval);
          t += sleep_interval;
        }
      }

      stream<int> throttle(int ms) {
        stream<int> s;
        return s;
      }



      ~clock() {

      }
    };
  }
/*
  stream filter(const stream& s) {

  }
  */
  template<class T>
  streams::variable<T> just(T v) {
    return streams::variable<T>();
  }

  template<class T, class T_a, class T_b>
  struct combinedStream
    : public streams::variable<T>
  {

    T_a _cached_a;
    T_b _cached_b;

    template<class T_Func>
    void update_a(T_a a, T_Func f) {
      __value__ = f(_cached_a = a, _cached_b);
      fire(__value__);
    }

    template<class T_Func>
    void update_b(T_b b, T_Func f) {
      __value__ = f(_cached_a, _cached_b = b);
      fire(__value__);
    }

  };

  template<class T_a, class T_b>
  struct combinedStream<void, T_a, T_b>
    : public streams::variable<void>
  {
    T_a _cached_a;
    T_b _cached_b;

    template<class T_Func>
    void update_a(T_a a, T_Func f) {
      f(_cached_a = a, _cached_b);
      fire();
    }

    template<class T_Func>
    void update_b(T_b b, T_Func f) {
      f(_cached_a, _cached_b = b);
      fire();
    }

  };

  template<class T>
  void update_variable(const T* v, streams::variable<T>& s) {
    s.__value__ = *v;
  }

  template<>
  void update_variable<void>(const void*, streams::variable<void>& s) { /*IDLE*/}


  template<class T, class T_a, class T_b, class T_Func>
  combinedStream<T, T_a, T_b> combineLatest(stream<T_a>& a, stream<T_b>& b, T_Func f) {
    combinedStream<T, T_a, T_b> comb;
    a.subscribe([&comb, f](T_a a) {
      comb.update_a(a, f);
    });
    b.subscribe([&comb, f](T_b b) {
      comb.update_b(b, f);
    });
    return comb;
  }

    

  //create_source

}



/*
const int& : reactive read-only int
const int : normal const int
int : normal int
int& : reactive writable int

*/


int _tmain(int argc, _TCHAR* argv[])
{
  using namespace reactive;

  
  streams::clock clk(1., 24.);
 

  // W másodpercenként nõ
  streams::variable<int> _W = just(10);
  auto secs = clk.throttle(1000);
  auto W = combineLatest<int>(secs, _W, [](int t, int w) {
    return w + (t % 1000);
  });;

  // H konstans
  streams::variable<int> H = just(20);

  auto area = combineLatest<int> (W, H, [](int w, int h)  {
    return w*h;
  });

  int _Area = 0;



  auto update_area = combineLatest<void>(area, clk, [&](int a, int t) {
    std::cout << "Area = " << _Area;
    std::cout << "Time Elapsed = " << t << std::endl;
  });



  clk.run(update_area);

  return 0;
}

