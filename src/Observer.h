#ifndef OBSERVER
#define OBSERVER

#include <list>

template <typename T> class Observer {
  public:
    virtual ~Observer() = default;

    virtual void update(const T &message) = 0;
};

template <typename T> class Subject {
    std::list<Observer<T> *> observerList;

  public:
    virtual ~Subject() = default;

    virtual void notifyAll(const T &message) const {
        auto observersCopy = observerList;
        for (auto &observer : observersCopy) {
            observer->update(message);
        }
    };

    virtual void attach(Observer<T> *observer) {
        this->observerList.push_back(observer);
    };

    virtual void detach(Observer<T> *observer) {
        this->observerList.remove(observer);
    }
};

#endif // OBSERVER