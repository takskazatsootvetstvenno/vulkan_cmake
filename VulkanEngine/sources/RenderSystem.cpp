#include "RenderSystem.h"

//
namespace sge {

/*static*/ RenderSystem& sge::RenderSystem::Instance() noexcept {
    static RenderSystem renderSystem;
    return renderSystem;
}

}  // namespace sge