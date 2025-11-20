/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef AP_MESH_H
#define AP_MESH_H
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
    Mesh networking module for multi-vehicle coordination
    
    Provides mesh networking capabilities for communication between
    multiple vehicles in a decentralized network topology.
*/

#include <AP_Common/AP_Common.h>
#include <AP_Param/AP_Param.h>
#include <AP_HAL/AP_HAL.h>

#define MESH_MAX_NODES          10      // Maximum number of nodes in mesh network
#define MESH_NODE_TIMEOUT_MS    30000   // Node timeout in milliseconds
#define MESH_MAX_HOPS           5       // Maximum hops for message routing

class AP_Mesh
{
public:
    // Mesh message types
    enum MeshMessageType {
        MESH_MSG_HEARTBEAT = 0,
        MESH_MSG_DATA = 1,
        MESH_MSG_ROUTE_REQUEST = 2,
        MESH_MSG_ROUTE_REPLY = 3
    };

    // Mesh node structure
    struct mesh_node_t {
        uint32_t node_id;               // Unique node identifier
        uint32_t last_seen_ms;          // Last time this node was seen
        float distance;                 // Distance to this node in meters
        uint8_t hop_count;              // Number of hops to reach this node
        uint32_t next_hop_id;           // Next hop node ID for routing
        bool is_active;                 // Node is currently active
    };

    // Constructor
    AP_Mesh();

    // for holding parameters
    static const struct AP_Param::GroupInfo var_info[];

    // Initialize the mesh network
    void init();

    // periodic task that maintains node_list
    void update();

    // add or update node in the mesh
    void update_node(uint32_t node_id, uint8_t hop_count);

    // send a message through the mesh network
    bool send_message(uint32_t dest_node_id, const uint8_t *data, uint16_t len);

    // handle incoming mesh message
    void handle_message(uint32_t src_node_id, MeshMessageType msg_type, 
                       const uint8_t *data, uint16_t len);

    // get number of active nodes in mesh
    uint16_t get_node_count() const { return _node_count; }

    // check if mesh is enabled
    bool enabled() const { return _enabled; }

    // get our node ID
    uint32_t get_node_id() const { return _node_id; }

private:
    // initialize node list
    void init_nodes();

    // find a node by ID, returns index or -1 if not found
    int16_t find_node(uint32_t node_id) const;

    // remove a node from the list
    void remove_node(uint16_t index);

    // update routing table
    void update_routing();

    // broadcast heartbeat to neighbors
    void send_heartbeat();

    // handle heartbeat message
    void handle_heartbeat(uint32_t src_node_id, uint8_t hop_count);

    AP_Int8     _enabled;               // Enable mesh networking
    AP_Int32    _node_id;               // Our node ID
    AP_Int16    _heartbeat_interval;    // Heartbeat interval in ms
    
    mesh_node_t _node_list[MESH_MAX_NODES];
    uint16_t    _node_count;
    uint32_t    _last_heartbeat_ms;
};

#endif // AP_MESH_H
