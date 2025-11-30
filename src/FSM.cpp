#include "../include/FSM.hpp"
#include <iostream>
#include <stdexcept>

namespace miniFSM{
    FSM::FSM(std::shared_ptr<State> initialState): currentState_(std::move(initialState)), stop_(false){
        if(!currentState_){
            throw std::runtime_error("FSM: Initial state cannot be null");
        }
        worker_ = std::thread(&FSM::workerThread, this);
    }
    FSM::~FSM() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        if(worker_.joinable()){
            worker_.join();
        }
    }
    void FSM::trigger(const Event& event){
        {
        std::lock_guard<std::mutex> lock(mtx_);
        if (stop_) return;
        eventQueue_.push(event);
        }
        cv_.notify_one();
    }
    std::string FSM::getCurrentStateName() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return currentState_ ? currentState_->getName() : "null";
    }
    void FSM::addTransition(std::shared_ptr<State> from, const Event& event, std::shared_ptr<State> to) {
        if (!from || !to) {
            throw std::invalid_argument("State cannot be null");
        }
        std::lock_guard<std::mutex> lock(mtx_);
        transitionTable_[{std::move(from), event}] = std::move(to);
    }
    //1 个 FSM 工作线程（运行 workerThread()，负责处理事件和切换状态），任意多个外部线程（调用 trigger(event)，只负责投递事件）
    void FSM::workerThread(){    // 工作线程
        while(!stop_){
            Event event;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                cv_.wait(lock, [this] { return stop_ || !eventQueue_.empty(); });
                if(stop_){
                    break;
                }
                if(eventQueue_.empty()){
                    continue;
                }
                event = std::move(eventQueue_.front());
                eventQueue_.pop();
            }
            auto key = std::make_pair(currentState_, event);
            auto it = transitionTable_.find(key);
            if(it == transitionTable_.end()){
                std::cerr << "[FSM]No transition found for event: " << event << " from state: " << currentState_->getName() << std::endl;
                continue;
            }
            currentState_->exit(*this);
            currentState_ = it->second;
            currentState_->enter(*this);
        }
    }
}