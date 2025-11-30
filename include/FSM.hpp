#pragma once
#include <string>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include <unordered_map>
#include <queue>
#include <condition_variable>
#include <atomic>

namespace miniFSM{
    using Event = std::string;
    class FSM;
    class State{
        public:
            explicit State(std::string name): name_(std::move(name)){}  // 移动构造
            const std::string& getName() const{ return name_; }
            void setOnEnter(std::function<void(FSM&)> cb){ onEnter_ = std::move(cb); }
            void setOnExit(std::function<void(FSM&)> cb){ onExit_ = std::move(cb); }
            void enter(FSM& fsm){ if(onEnter_) onEnter_(fsm); }
            void exit(FSM& fsm){ if(onExit_) onExit_(fsm); }
            
        private:
            std::string name_;
            std::function<void(FSM&)> onEnter_;
            std::function<void(FSM&)> onExit_;
    };
    class FSM{
        public:
            explicit FSM(std::shared_ptr<State> initialState);
            ~FSM();
            void trigger(const Event& event);
            std::string getCurrentStateName() const;
            void addTransition(std::shared_ptr<State> from, const Event& event, std::shared_ptr<State> to);
        private:
            void workerThread();  // 工作线程
            std::shared_ptr<State> currentState_;
            // 转移表
            using TransitionKey = std::pair<std::shared_ptr<State>, Event>; // 状态和事件
            struct MyHash{  // 自定义类型需要自定义哈希函数
                std::size_t operator()(const TransitionKey& key) const{ 
                    return std::hash<void*>{}(key.first.get()) ^ std::hash<std::string>{}(key.second);  // std::hash<void*>{} 创建一个临时的哈希函数对象,然后用它计算指针的哈希值
                }
            };
            std::unordered_map<TransitionKey, std::shared_ptr<State>, MyHash> transitionTable_; // 转移表
            // 线程相关
            mutable std::mutex mtx_;
            std::queue<Event> eventQueue_;
            std::condition_variable cv_;
            std::atomic<bool> stop_{false};
            std::thread worker_;
    };
}