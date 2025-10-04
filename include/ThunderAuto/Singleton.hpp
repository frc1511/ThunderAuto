#pragma once

class SingletonBase {
 public:
  virtual ~SingletonBase() = default;

  SingletonBase(const SingletonBase&) = delete;
  SingletonBase(SingletonBase&&) = delete;
  SingletonBase& operator=(const SingletonBase&) = delete;
  SingletonBase& operator=(SingletonBase&&) = delete;

 protected:
  SingletonBase() = default;
};

template <typename T>
class Singleton : public SingletonBase {
 public:
  static T& get() {
    static T instance;
    return instance;
  }
};
