# AP_Mesh Library

## Overview

The AP_Mesh library provides mesh networking capabilities for ArduPilot, enabling multiple vehicles to communicate with each other in a decentralized network topology. This allows for coordinated operations between multiple drones or vehicles without requiring a central ground station for inter-vehicle communication.

## Features

- **Node Discovery**: Automatic discovery of neighboring nodes through periodic heartbeat broadcasts
- **Multi-hop Routing**: Support for message routing across multiple hops (up to 5 hops)
- **Node Timeout**: Automatic removal of inactive nodes after 30 seconds
- **Configurable Parameters**: Enable/disable mesh networking, set node ID, and configure heartbeat intervals
- **Scalable**: Supports up to 10 nodes in the mesh network

## Parameters

- **MESH_ENABLE**: Enable or disable mesh networking (0=Disabled, 1=Enabled)
- **MESH_NODE_ID**: Unique identifier for this node in the mesh network (1-4294967295)
- **MESH_HB_INTRVL**: Interval between heartbeat broadcasts in milliseconds (1000-60000ms, default: 5000ms)

## Usage

### Basic Initialization

```cpp
#include <AP_Mesh/AP_Mesh.h>

// Create mesh instance
AP_Mesh mesh;

void setup() {
    // Initialize mesh networking
    mesh.init();
}

void loop() {
    // Update mesh network state
    mesh.update();
    
    // Check if mesh is enabled and print status
    if (mesh.enabled()) {
        uint16_t node_count = mesh.get_node_count();
        // ... use node_count for your application
    }
}
```

### Sending Messages

```cpp
// Send a message to a specific node
uint32_t dest_node_id = 123;
const uint8_t data[] = "Hello, mesh!";
bool success = mesh.send_message(dest_node_id, data, sizeof(data));
```

### Handling Incoming Messages

```cpp
// Handle incoming mesh message
void on_mesh_message(uint32_t src_node_id, AP_Mesh::MeshMessageType msg_type, 
                     const uint8_t *data, uint16_t len) {
    mesh.handle_message(src_node_id, msg_type, data, len);
}
```

## Message Types

- **MESH_MSG_HEARTBEAT**: Periodic heartbeat for node discovery
- **MESH_MSG_DATA**: Data message between nodes
- **MESH_MSG_ROUTE_REQUEST**: Route discovery request
- **MESH_MSG_ROUTE_REPLY**: Route discovery reply

## Architecture

The AP_Mesh library maintains a table of known nodes with the following information for each node:
- Node ID
- Last seen timestamp
- Distance (in meters)
- Hop count to reach the node
- Next hop node ID for routing
- Active status

Nodes are discovered through periodic heartbeat broadcasts. When a heartbeat is received, the node is added to the routing table or updated if it already exists. The routing algorithm uses hop count to determine the best path to each destination.

## Limitations

- Maximum 10 nodes in the network (MESH_MAX_NODES)
- Maximum 5 hops for message routing (MESH_MAX_HOPS)
- Node timeout after 30 seconds of inactivity (MESH_NODE_TIMEOUT_MS)
- Current implementation provides framework; actual RF communication layer needs to be integrated

## Future Enhancements

- Integration with MAVLink for message transport
- Advanced routing algorithms (AODV, OLSR)
- Network topology visualization
- Link quality metrics
- Power-aware routing
- Security and authentication

## Example

See `examples/AP_Mesh_test/AP_Mesh_test.cpp` for a complete example of how to use the AP_Mesh library.
