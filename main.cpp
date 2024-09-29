#include "app.hpp"
#include "teng_pipeline.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// The order of operations is
// 1. App: Create the App. Many things happen.
//   - Teng_Window: Window is created.
//   - Teng_Device: Vulcan is initialized in teng_device.cpp.
//   - Teng_Device: Physical GPU is picked.
//   - Teng_Device: Handle to device ("logical" device) is created and device is connected to window.
//     Vulkan calls the window a "surface".
//   - Teng_Renderer: Renderer is created. The renderer is responsible for the swap chain.
//   - Teng_Model: Models are created, which define the data contained in the vertex buffer.
//   - Pipeline layout is created. I don't really understand this currently.
//   - Swap chain is created. This needs to be recreated whenever the dimensions change.
//     - The swap chain is a dependency of the pipeline, so a new pipeline is created. I don't really understand this currently.
//     - The pipeline contains a render pass. A render pass tells the graphics pipeline what the contents of the final frame buffer are expected to be.
//     - The pipeline contains the pipeline layout.
//     - The pipeline contains shader data.
//   - Command buffer is created. I don't really understand this.
//
// 2. App is run, which really only calls m_DrawFrame().
//   - The swap chain provides the next framebuffer to write to.
//   - The swap chain might have to be recreated thus the pipeline as well.
//   - Command buffer is recorded.
int main() {
  teng::App app{}; // This creates a window

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
