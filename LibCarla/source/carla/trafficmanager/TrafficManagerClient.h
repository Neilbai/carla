// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#ifndef __TRAFFICMANAGERCLIENT__
#define __TRAFFICMANAGERCLIENT__

#include "carla/client/Actor.h"

#include <rpc/client.h>

#include <chrono>

#define TM_TIMEOUT            2000 // In ms
#define DEFAULT_RPC_TM_PORT   8000 // TM_SERVER_PORT

/// Provides communication with the rpc of TrafficManagerServer
class TrafficManagerClient {

public:

  TrafficManagerClient(const TrafficManagerClient &) = default;
  TrafficManagerClient(TrafficManagerClient &&) = default;

  TrafficManagerClient &operator=(const TrafficManagerClient &) = default;
  TrafficManagerClient &operator=(TrafficManagerClient &&) = default;

  /// Parametric constructor to initialize the parameters
  TrafficManagerClient(
    const std::string &_host,
    const uint16_t &_port)
    : tmhost(_host),
      tmport(_port) {
    Init();
  }

  /// Destructor
  ~TrafficManagerClient() {
    carla::log_info("TrafficManagerClient dtr");
    stop_thread = true;
    /*if(_thread) {
      delete _thread;
      _thread = nullptr;
    }*/
    carla::log_info("TrafficManagerClient dtr client");
    if(_client) {
      delete _client;
      _client = nullptr;
    }
    carla::log_info("TrafficManagerClient dtr end");
  };

  void Init() {
    if(!_client) {
      carla::log_info("TrafficManagerClient creating rpc::client", tmhost, tmport);
      _client = new rpc::client(tmhost, tmport);
      _client->set_timeout(TM_TIMEOUT);
    } else {
      carla::log_info("TrafficManagerClient already have rpc::client");
    }
    if(/* !_thread && */ _client){
      std::thread _thread = std::thread( [&] () {
        bool connection_active = false;
        std::chrono::milliseconds wait_time(TM_TIMEOUT);
        do {
          std::this_thread::sleep_for(wait_time);
          connection_active = (_client->get_connection_state() == rpc::client::connection_state::connected);
          carla::log_info("TrafficManagerClient - connection", connection_active?"active":"ended");
        } while (connection_active && !stop_thread);
        Stop();
      });
      _thread.detach();
    }
  }

  void Stop();

  /// Set parameters
  void setServerDetails(const std::string &_host, const uint16_t &_port) {
     tmhost = _host;
     tmport = _port;
  }

  /// Get parametrs
  void getServerDetails(std::string &_host, uint16_t &_port) {
    _host = tmhost;
    _port = tmport;
  }

  /// Register vehicles to remote TM server via RPC client
  void RegisterVehicle
  (const std::vector<carla::rpc::Actor> &actor_list) {
    if(_client){
      _client->call("register_vehicle", std::move(actor_list));
    } else {
      carla::log_info("TrafficManagerClient - cannot RegisterVehicle");
    }
  }

  /// Unregister vehicles to remote TM server via RPC client
  void UnregisterVehicle
  (const std::vector<carla::rpc::Actor> &actor_list) {
    if(_client){
      _client->call("unregister_vehicle", std::move(actor_list));
    }
  }

  /// Set target velocity specific to a vehicle.
  void SetPercentageSpeedDifference(const carla::rpc::Actor &_actor, const float percentage) {
    if(_client){
      _client->call("set_percentage_speed_difference", std::move(_actor), percentage);
    }
  }

  /// Set global target velocity.
  void SetGlobalPercentageSpeedDifference(const float percentage) {
    if(_client){
      _client->call("set_global_percentage_speed_difference", percentage);
    }
  }

  /// Set collision detection rules between vehicles.
  void SetCollisionDetection
    ( const carla::rpc::Actor &reference_actor
    , const carla::rpc::Actor &other_actor
    , const bool detect_collision) {
    if(_client){
      _client->call("set_collision_detection", reference_actor, other_actor, detect_collision);
    }
  }

  /// Method to force lane change on a vehicle.
  /// Direction flag can be set to true for left and false for right.
  void SetForceLaneChange(const carla::rpc::Actor &actor, const bool direction) {
    if(_client){
      _client->call("set_force_lane_change", actor, direction);
    }
  }

  /// Enable / disable automatic lane change on a vehicle.
  void SetAutoLaneChange(const carla::rpc::Actor &actor, const bool enable) {
    if(_client){
      _client->call("set_auto_lane_change", actor, enable);
    }
  }

  /// Method to specify how much distance a vehicle should maintain to
  /// the leading vehicle.
  void SetDistanceToLeadingVehicle(const carla::rpc::Actor &actor, const float distance) {
    if(_client){
      _client->call("set_distance_to_leading_vehicle", actor, distance);
    }
  }

  /// Method to specify the % chance of ignoring collisions with other actors
  void SetPercentageIgnoreActors(const carla::rpc::Actor &actor, const float percentage) {
    if(_client){
      _client->call("set_percentage_ignore_actors", actor, percentage);
    }
  }

  /// Method to specify the % chance of running a red light
  void SetPercentageRunningLight(const carla::rpc::Actor &actor, const float percentage) {
    if(_client) {
      _client->call("set_percentage_running_light", actor, percentage);
    }
  }

  /// Method to reset all traffic lights.
  void ResetAllTrafficLights() {
    if(_client){
      _client->call("reset_all_traffic_lights");
    }
  }

  void HealthCheckRemoteTM() {
    if(_client){
      _client->call("health_check_remote_TM");
    }
  }
private:
  //std::thread *_thread = nullptr;
  rpc::client *_client = nullptr;
  std::string tmhost;
  uint16_t    tmport;
  bool stop_thread = false;
};

#endif /* __TRAFFICMANAGERCLIENT__ */