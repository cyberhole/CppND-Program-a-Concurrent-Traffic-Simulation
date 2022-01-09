#include "TrafficLight.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <iostream>
#include <random>
/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  // _condition.wait() to wait for and receive new messages and pull them from
  // the queue using move semantics. The received object should then be returned
  // by the receive function.

  std::unique_lock<std::mutex> uLock(_mutex);
  _cond.wait(uLock, [this] {return !_queue.empty();}); // pass unique lock to condition variable
  // remove last vector element from queue
 
  T msg = std::move(_queue.back());
 
  _queue.clear(); //<- udacity forum

  return msg; // will not be copied due to return value optimization (RVO) in
              // C++
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex> as well as _condition.notify_one() to add a new
  // message to the queue and afterwards send a notification.
  std::lock_guard<std::mutex> uLock(_mutex);
  
  _queue.push_back(std::move(msg));
  _cond.notify_one(); // notify client
}

/* Implementation of class "TrafficLight" */

#include <random>
TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true) {
    int message = _msg.receive();
    if (message == TrafficLightPhase::green) // 1 means that light is green
    {
      break;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be started
  // in a thread when the public method „simulate“ is called. To do this, use
  // the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the time
  // between two loop cycles and toggles the current phase of the traffic light
  // between red and green and sends an update method to the message queue using
  // move semantics. The cycle duration should be a random value between 4 and 6
  // seconds. Also, the while-loop should use std::this_thread::sleep_for to
  // wait 1ms between two cycles.

  //queue = std::make_shared<MessageQueue<TrafficLightPhase>>();

  auto toggle = [](TrafficLightPhase &_currentPhase) {
    _currentPhase == TrafficLightPhase::red
        ? _currentPhase = TrafficLightPhase::green
        : _currentPhase = TrafficLightPhase::red;
  };

auto start =std::chrono::high_resolution_clock::now(); // start timer
int _time=rand() % (6000 - 4000 + 1) + 4000; // first random value for time on/off of TrafficLight

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (duration.count()>=_time){
      
    toggle(_currentPhase); // change color of the light

    //futures=(std::async(std::launch::async,&MessageQueue<TrafficLightPhase>::send,queue, std::move(_currentPhase))); // send message of new color
    _msg.send(std::move(_currentPhase));
    //std::cout << "duration was " << duration.count() << " milliseconds"<< std::endl; // print duration of time
    _time=rand() % (6000 - 4000 + 1) + 4000; // change random value
    start =std::chrono::high_resolution_clock::now(); // reset timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    auto stop = std::chrono::high_resolution_clock::now(); // measure new time
    duration =std::chrono::duration_cast<std::chrono::milliseconds>(stop - start); // calculate the difference
 
  
  }
}
