#include "structs/structs.hpp"

#include "vkr/vkr.hpp"
#include "ecs/ecs.hpp"
#include "qed/qed.hpp"
#include "thm/thm.hpp"

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

// thm = thread manager
// vkr = vulkan rendering
// ecs = entity component system
// inp = game input system
// sps = spatial partitioning system
// qed = qoal editor

// structs = structs used throughout project

int main() {
    ecs::EntityManager em;
    vkr::VulkanRendering vkr;
    
    thm::ThreadManager thm{vkr, em};

    return 0;
}