#include "../include/FSM.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

using namespace miniFSM;
using namespace std::chrono_literals;

int main(){
    auto idle = std::make_shared<State>("idle");
    auto running = std::make_shared<State>("running");
    auto paused = std::make_shared<State>("paused");

    idle->setOnEnter([](FSM&) { std::cout << "→ Idle\n"; });
    running->setOnEnter([](FSM&) { std::cout << "→ Running\n"; });
    paused->setOnEnter([](FSM&) { std::cout << "→ Paused\n"; });

    FSM fsm(idle);

    fsm.addTransition(idle, "start", running);
    fsm.addTransition(running, "pause", paused);
    fsm.addTransition(paused, "resume", running);
    fsm.addTransition(running, "stop", idle);

    // 启动多个线程并发触发事件
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&, id = i]() {
            for (int j = 0; j < 3; ++j) {
                if (id % 2 == 0) {
                    fsm.trigger("start");
                    std::this_thread::sleep_for(10ms);
                    fsm.trigger("stop");
                } else {
                    fsm.trigger("start");
                    std::this_thread::sleep_for(15ms);
                    fsm.trigger("pause");
                    std::this_thread::sleep_for(10ms);
                    fsm.trigger("resume");
                    std::this_thread::sleep_for(10ms);
                    fsm.trigger("stop");
                }
                std::this_thread::sleep_for(50ms);
            }
        });
    }

    // 等待所有线程结束
    for (auto& t : threads) t.join();

    std::cout << "Final state: " << fsm.getCurrentStateName() << "\n";
    return 0;
}