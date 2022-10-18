// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <cinttypes>
#include <memory>

#include "example_interfaces/srv/add_two_ints.hpp"
#include "rclcpp/rclcpp.hpp"

using AddTwoInts = example_interfaces::srv::AddTwoInts;
using namespace std::literals::chrono_literals;

class MinimalClient: public rclcpp::Node {
public:
  MinimalClient(const std::string &node_name)
  : Node(node_name) {
    timer_ = create_wall_timer(1s, std::bind(&MinimalClient::timer_callback, this));
    client_ = create_client<AddTwoInts>("add_two_ints");

    while (!client_->wait_for_service(1s)) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(get_logger(), "client interrupted while waiting for service to appear.");
        return;
      }
      RCLCPP_INFO(get_logger(), "waiting for service to appear...");
    }

    internal_executor_.add_node(this->get_node_base_interface());
  }
private:
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Client<AddTwoInts>::SharedPtr client_;
  rclcpp::executors::SingleThreadedExecutor internal_executor_;

  void timer_callback(){
    auto request = std::make_shared<AddTwoInts::Request>();
    request->a = 41;
    request->b = 1;
    auto result_future = client_->async_send_request(request);

    RCLCPP_INFO(get_logger(), "Request sent, waiting...");

    if(result_future.wait_for(5s) != std::future_status::ready) //(internal_executor_.spin_until_future_complete(result_future, 5s) !=rclcpp::FutureReturnCode::SUCCESS)  // 
    {
      RCLCPP_ERROR(get_logger(), "service call failed :(");
      return;
    } else {
      RCLCPP_INFO(get_logger(), "service call succeed");
    }

    }

};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::executors::SingleThreadedExecutor exe;

  auto node = std::make_shared<MinimalClient>("minimal_client");
  exe.add_node(node->get_node_base_interface());

  exe.spin();
  rclcpp::shutdown();

  return 0;
}
