/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
   Simple example of AP_Mesh usage
 */

#include <AP_HAL/AP_HAL.h>
#include <AP_Mesh/AP_Mesh.h>

const AP_HAL::HAL& hal = AP_HAL::get_HAL();

// Create mesh instance
static AP_Mesh mesh;

void setup()
{
    hal.console->printf("AP_Mesh test\n");
    
    // Initialize mesh networking
    mesh.init();
}

void loop()
{
    // Update mesh network state
    mesh.update();
    
    // Print status
    if (mesh.enabled()) {
        hal.console->printf("Mesh Status - Node ID: %u, Active Nodes: %u\n",
                          (unsigned)mesh.get_node_id(),
                          mesh.get_node_count());
    }
    
    hal.scheduler->delay(1000);
}

AP_HAL_MAIN();
