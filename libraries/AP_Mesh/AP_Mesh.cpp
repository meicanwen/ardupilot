/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "AP_Mesh.h"
#include <AP_HAL/AP_HAL.h>

extern const AP_HAL::HAL& hal;

// table of user settable parameters
const AP_Param::GroupInfo AP_Mesh::var_info[] = {
    // @Param: ENABLE
    // @DisplayName: Mesh Enable
    // @Description: Enable mesh networking
    // @Values: 0:Disabled,1:Enabled
    // @User: Advanced
    AP_GROUPINFO("ENABLE", 0, AP_Mesh, _enabled, 0),

    // @Param: NODE_ID
    // @DisplayName: Mesh Node ID
    // @Description: Unique identifier for this node in the mesh network
    // @Range: 1 4294967295
    // @User: Advanced
    AP_GROUPINFO("NODE_ID", 1, AP_Mesh, _node_id, 1),

    // @Param: HB_INTRVL
    // @DisplayName: Heartbeat Interval
    // @Description: Interval between heartbeat broadcasts in milliseconds
    // @Units: ms
    // @Range: 1000 60000
    // @User: Advanced
    AP_GROUPINFO("HB_INTRVL", 2, AP_Mesh, _heartbeat_interval, 5000),

    AP_GROUPEND
};

// Constructor
AP_Mesh::AP_Mesh() :
    _node_count(0),
    _last_heartbeat_ms(0)
{
    AP_Param::setup_object_defaults(this, var_info);
}

// Initialize the mesh network
void AP_Mesh::init()
{
    if (!_enabled) {
        return;
    }

    init_nodes();
    
    hal.console->printf("Mesh: Initialized with Node ID: %u\n", (unsigned)_node_id.get());
}

// Initialize node list
void AP_Mesh::init_nodes()
{
    for (uint16_t i = 0; i < MESH_MAX_NODES; i++) {
        _node_list[i].node_id = 0;
        _node_list[i].last_seen_ms = 0;
        _node_list[i].distance = 0;
        _node_list[i].hop_count = 0;
        _node_list[i].next_hop_id = 0;
        _node_list[i].is_active = false;
    }
    _node_count = 0;
}

// Periodic update task
void AP_Mesh::update()
{
    if (!_enabled) {
        return;
    }

    uint32_t now_ms = AP_HAL::millis();

    // Send periodic heartbeat
    if (now_ms - _last_heartbeat_ms >= (uint32_t)_heartbeat_interval.get()) {
        send_heartbeat();
        _last_heartbeat_ms = now_ms;
    }

    // Check for timed out nodes
    for (uint16_t i = 0; i < MESH_MAX_NODES; i++) {
        if (_node_list[i].is_active) {
            if (now_ms - _node_list[i].last_seen_ms > MESH_NODE_TIMEOUT_MS) {
                hal.console->printf("Mesh: Node %u timed out\n", 
                                  (unsigned)_node_list[i].node_id);
                remove_node(i);
            }
        }
    }
}

// Find a node by ID
int16_t AP_Mesh::find_node(uint32_t node_id) const
{
    for (uint16_t i = 0; i < MESH_MAX_NODES; i++) {
        if (_node_list[i].is_active && _node_list[i].node_id == node_id) {
            return i;
        }
    }
    return -1;
}

// Update or add a node
void AP_Mesh::update_node(uint32_t node_id, uint8_t hop_count)
{
    if (!_enabled || node_id == _node_id.get()) {
        return;
    }

    int16_t index = find_node(node_id);
    uint32_t now_ms = AP_HAL::millis();

    if (index >= 0) {
        // Update existing node
        _node_list[index].last_seen_ms = now_ms;
        if (hop_count < _node_list[index].hop_count) {
            _node_list[index].hop_count = hop_count;
            update_routing();
        }
    } else {
        // Add new node
        if (_node_count < MESH_MAX_NODES) {
            // Find first inactive slot
            for (uint16_t i = 0; i < MESH_MAX_NODES; i++) {
                if (!_node_list[i].is_active) {
                    _node_list[i].node_id = node_id;
                    _node_list[i].last_seen_ms = now_ms;
                    _node_list[i].hop_count = hop_count;
                    _node_list[i].is_active = true;
                    _node_count++;
                    hal.console->printf("Mesh: Added node %u (hop count: %u)\n", 
                                      (unsigned)node_id, hop_count);
                    update_routing();
                    break;
                }
            }
        } else {
            hal.console->printf("Mesh: Node list full, cannot add node %u\n", 
                              (unsigned)node_id);
        }
    }
}

// Remove a node from the list
void AP_Mesh::remove_node(uint16_t index)
{
    if (index < MESH_MAX_NODES && _node_list[index].is_active) {
        _node_list[index].is_active = false;
        _node_list[index].node_id = 0;
        if (_node_count > 0) {
            _node_count--;
        }
        update_routing();
    }
}

// Update routing table
void AP_Mesh::update_routing()
{
    // Simple routing: next hop is the node with lowest hop count to destination
    // In a real implementation, this would use a more sophisticated routing algorithm
    for (uint16_t i = 0; i < MESH_MAX_NODES; i++) {
        if (_node_list[i].is_active && _node_list[i].hop_count == 1) {
            // Direct neighbor - next hop is the node itself
            _node_list[i].next_hop_id = _node_list[i].node_id;
        }
    }
}

// Send heartbeat to announce presence
void AP_Mesh::send_heartbeat()
{
    if (!_enabled) {
        return;
    }

    // In a real implementation, this would broadcast a heartbeat message
    // through the communication channel
    hal.console->printf("Mesh: Sending heartbeat (Node ID: %u, Active nodes: %u)\n", 
                      (unsigned)_node_id.get(), _node_count);
}

// Handle heartbeat message
void AP_Mesh::handle_heartbeat(uint32_t src_node_id, uint8_t hop_count)
{
    if (!_enabled) {
        return;
    }

    update_node(src_node_id, hop_count + 1);
}

// Send a message through the mesh
bool AP_Mesh::send_message(uint32_t dest_node_id, const uint8_t *data, uint16_t len)
{
    if (!_enabled || data == nullptr || len == 0) {
        return false;
    }

    int16_t index = find_node(dest_node_id);
    if (index < 0) {
        hal.console->printf("Mesh: Node %u not found in routing table\n", 
                          (unsigned)dest_node_id);
        return false;
    }

    // In a real implementation, this would send the message through the
    // communication channel, potentially using multi-hop routing
    hal.console->printf("Mesh: Sending message to node %u (hop count: %u)\n", 
                      (unsigned)dest_node_id, _node_list[index].hop_count);
    
    return true;
}

// Handle incoming mesh message
void AP_Mesh::handle_message(uint32_t src_node_id, MeshMessageType msg_type, 
                            const uint8_t *data, uint16_t len)
{
    if (!_enabled) {
        return;
    }

    switch (msg_type) {
        case MESH_MSG_HEARTBEAT:
            if (len >= 1) {
                handle_heartbeat(src_node_id, data[0]);
            }
            break;
            
        case MESH_MSG_DATA:
            hal.console->printf("Mesh: Received data message from node %u (len: %u)\n", 
                              (unsigned)src_node_id, len);
            break;
            
        case MESH_MSG_ROUTE_REQUEST:
            hal.console->printf("Mesh: Received route request from node %u\n", 
                              (unsigned)src_node_id);
            break;
            
        case MESH_MSG_ROUTE_REPLY:
            hal.console->printf("Mesh: Received route reply from node %u\n", 
                              (unsigned)src_node_id);
            break;
            
        default:
            hal.console->printf("Mesh: Unknown message type %d from node %u\n", 
                              msg_type, (unsigned)src_node_id);
            break;
    }
}
