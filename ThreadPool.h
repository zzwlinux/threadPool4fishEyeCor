#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdio.h>

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

template<typename T> struct Type {
    typedef  T                 type;
    typedef  T*                pointer;
    typedef  T&                refference;
    typedef const  T*          const_pointer;
    typedef const  T&          const_refference;
};

template <typename T> void defaultFunc(typename Type<T>::const_refference) {}

template <typename T>
class ThreadPool
{
public:
    enum {
        WorkThreadNum = 4
    };

public:
    typedef std::function<void(typename Type<T>::refference)> WorkFunc;

public:
    ThreadPool(int threadNum = 0) {
        maxThreadNum   = threadNum > WorkThreadNum ? threadNum : WorkThreadNum;

        workFunc       = new WorkFunc[maxThreadNum];
        running        = new bool[maxThreadNum];
        threads        = new std::thread[maxThreadNum];
        Mutex          = new std::mutex[maxThreadNum];
        conditionVal   = new std::condition_variable[maxThreadNum];
        x              = new T[maxThreadNum];
        sourceMutex    = new std::mutex[maxThreadNum];
        sleepThreadNum = maxThreadNum;

        for(int i = 0; i < maxThreadNum; ++i) {
            workFunc[i] = defaultFunc<T>;
            running[i] = false;
            threads[i] = std::thread(&ThreadPool::workLoop, this, i);
        }
    }

    ~ThreadPool() {
        for(int i = 0; i < maxThreadNum; ++i)
            threads[i].join();

        delete[]workFunc;
        delete[]running;
        delete[]threads;
        delete[]Mutex;
        delete[]conditionVal;
        delete[]x;
        delete[]sourceMutex;
    }

    bool reduce(int idx, WorkFunc f, typename Type<T>::refference x) {
        if(idx < 0 || idx >= maxThreadNum)
            return false;

        sourceMutex[idx].lock();
        --sleepThreadNum;
        workFunc[idx] = f;
        this->x[idx] = x;
        running[idx] = true;
        sourceMutex[idx].unlock();

        conditionVal[idx].notify_all();
        return true;
    }

    bool isSynchronize() {
        return sleepThreadNum == maxThreadNum;
    }

private:
    void workLoop(int idx) {
        std::unique_lock<std::mutex> lock(Mutex[idx]);
        while(true)
        {
            if(running[idx] == false)
                conditionVal[idx].wait(lock);

            workFunc[idx](x[idx]);
            ++sleepThreadNum;
            workFunc[idx] = defaultFunc<T>;
            running[idx] = false;
        }
    }


private:
    ////////////////////////////////////////////
    /// \brief thread manager
    ////////////////////////////////////////////
    std::condition_variable *conditionVal;
    std::mutex              *Mutex;
    std::thread             *threads;
    int                     maxThreadNum;
    std::atomic_int         sleepThreadNum;


    /////////////////////////////////////////
    /// \brief workFunc param
    /////////////////////////////////////////
    WorkFunc    *workFunc;
    T           *x;
    bool        *running;
    std::mutex  *sourceMutex;
};


template <typename T>
class ThreadPool<T*>
{
public:
    typedef typename Type<T>::const_pointer    const_pointer;
    typedef typename Type<T>::const_refference const_refference;
    typedef typename Type<T>::pointer          pointer;
    typedef typename Type<T>::refference       refference;
    typedef typename Type<T>::type             type;

public:
    enum {
        WorkThreadNum = 4
    };

public:
    typedef std::function<void(refference)> WorkFunc;

public:
    ThreadPool(int threadNum = 0) {
        maxThreadNum   = threadNum > WorkThreadNum ? threadNum : WorkThreadNum;

        workFunc       = new WorkFunc[maxThreadNum];
        running        = new bool[maxThreadNum];
        threads        = new std::thread[maxThreadNum];
        Mutex          = new std::mutex[maxThreadNum];
        conditionVal   = new std::condition_variable[maxThreadNum];
        x              = new type[maxThreadNum];
        sourceMutex    = new std::mutex[maxThreadNum];

        sleepThreadNum = maxThreadNum;

        for(int i = 0; i < maxThreadNum; ++i) {
            workFunc[i] = defaultFunc<type>;
            running[i] = false;
            threads[i] = std::thread(&ThreadPool::workLoop, this, i);
        }
    }

    ~ThreadPool() {
        for(int i = 0; i < maxThreadNum; ++i)
            threads[i].join();

        delete[]workFunc;
        delete[]running;
        delete[]threads;
        delete[]Mutex;
        delete[]conditionVal;
        delete[]x;
        delete[]sourceMutex;
    }

    bool reduce(int idx, WorkFunc f, pointer x) {
        if(idx < 0 || idx >= maxThreadNum)
            return false;
        
//        sourceMutex[idx].lock();
        --sleepThreadNum;
        workFunc[idx] = f;
        this->x[idx] = x[idx];
        running[idx] = true;
//        sourceMutex[idx].unlock();

        conditionVal[idx].notify_all();
        return true;
    }

    bool isSynchronize() {
        return sleepThreadNum == maxThreadNum;
    }

private:
    void workLoop(int idx) {
        std::unique_lock<std::mutex> lock(Mutex[idx]);
        while(true)
        {
            if(running[idx] == false)
                conditionVal[idx].wait(lock);

            workFunc[idx](x[idx]);
            ++sleepThreadNum;
            workFunc[idx] = defaultFunc<T>;
            running[idx] = false;
        }
    }

private:
    ////////////////////////////////////////////
    /// \brief thread manager
    ////////////////////////////////////////////
    std::condition_variable *conditionVal;
    std::mutex              *Mutex;
    std::thread             *threads;
    int                     maxThreadNum;
    std::atomic_int         sleepThreadNum;

    /////////////////////////////////////////
    /// \brief workFunc param
    /////////////////////////////////////////
    WorkFunc    *workFunc;
    type        *x;
    bool        *running;
    std::mutex  *sourceMutex;
};



#endif // THREADPOOL_H
