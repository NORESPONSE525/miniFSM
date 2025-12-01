#include "gtest/gtest.h"
#include "../include/FSM.hpp"
#include <chrono>
#include <thread>
#include <atomic>

using namespace miniFSM;

// Helper function to wait for the FSM to process events
void waitForFSM() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// Test fixture for FSM tests
class FSMTest : public ::testing::Test {
protected:
    void SetUp() override {
        idleState = std::make_shared<State>("idle");
        runState = std::make_shared<State>("run");
        stopState = std::make_shared<State>("stop");
    }

    std::shared_ptr<State> idleState;
    std::shared_ptr<State> runState;
    std::shared_ptr<State> stopState;
};

TEST_F(FSMTest, Initialization) {
    FSM fsm(idleState);
    EXPECT_EQ(fsm.getCurrentStateName(), "idle");
}

TEST_F(FSMTest, InitializationWithNullState) {
    EXPECT_THROW(FSM fsm(nullptr), std::runtime_error);
}

TEST_F(FSMTest, BasicStateTransition) {
    FSM fsm(idleState);
    fsm.addTransition(idleState, "start", runState);
    fsm.addTransition(runState, "stop", idleState);

    EXPECT_EQ(fsm.getCurrentStateName(), "idle");

    fsm.trigger("start");
    waitForFSM();
    EXPECT_EQ(fsm.getCurrentStateName(), "run");

    fsm.trigger("stop");
    waitForFSM();
    EXPECT_EQ(fsm.getCurrentStateName(), "idle");
}

TEST_F(FSMTest, InvalidTransition) {
    FSM fsm(idleState);
    fsm.addTransition(idleState, "start", runState);

    EXPECT_EQ(fsm.getCurrentStateName(), "idle");

    // Trigger an event that is not registered for the current state
    fsm.trigger("unknown_event");
    waitForFSM();
    // The state should remain 'idle'
    EXPECT_EQ(fsm.getCurrentStateName(), "idle");
}

TEST_F(FSMTest, OnEnterAndOnExitCallbacks) {
    std::atomic<int> onEnterCounter(0);
    std::atomic<int> onExitCounter(0);

    idleState->setOnExit([&](FSM&) { onExitCounter++; });
    runState->setOnEnter([&](FSM&) { onEnterCounter++; });

    FSM fsm(idleState);
    fsm.addTransition(idleState, "start", runState);

    fsm.trigger("start");
    waitForFSM();

    EXPECT_EQ(fsm.getCurrentStateName(), "run");
    EXPECT_EQ(onExitCounter.load(), 1);
    EXPECT_EQ(onEnterCounter.load(), 1);
}

TEST_F(FSMTest, MultipleEventsInQueue) {
    FSM fsm(idleState);
    fsm.addTransition(idleState, "start", runState);
    fsm.addTransition(runState, "stop", stopState);

    // Trigger multiple events quickly
    fsm.trigger("start");
    fsm.trigger("stop");

    waitForFSM(); // Wait for both events to be processed

    EXPECT_EQ(fsm.getCurrentStateName(), "stop");
}