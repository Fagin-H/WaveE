# WaveE

WaveE is a low-level DirectX 12 (DX12) framework designed for creating and testing rendering techniques.

## Getting Started

To get started with WaveE, clone the repository:

```git
git clone https://github.com/Fagin-H/WaveE.git

cd WaveE
```

### Compile

Open the solution file in Visual Studio.

Compile the shader compiler.

Run the CompileShaders.bat script.

## Screen Space Glass Refraction

WaveE includes a demo showcasing screen space refraction.

![A rendered image showing a glass sphere in front of a cube and an icosphere.](./Images/Glass1.png)

## Maths Library

WaveE has a wrapper around DirectXMath to simplify handling it. To use the maths library, include WMaths.h and use the wma namespace to create vectors and matrices.

## Creating a New Project

I recommend duplicating the glass demo when creating a new project to ensure all the linker settings are configured correctly.

To use WaveE, include WaveManager.h.

Fill out a WaveEDescriptor to control screen resolution, camera controls, window title, and target framerate.

```cpp
#include "WaveManager.h"
#include "WMaths.h"

int WinMain() {
    // Init WaveE
    WaveEDescriptor waveEDesc{};

    // Fill out waveEDesc with your project settings

    WaveManager::Init(waveEDesc);

    // Create starting resources, materials, textures, buffers, etc...

    // End initialization
    WaveManager::EndInit();

    // Start game loop
    while(WaveInstance->BeginFrame()) {
        // Game logic and draw calls

        // End frame
        WaveInstance->EndFrame();
    }

    // Uninit WaveE
    WaveManager::Uninit();
}
```

## Contact
For any questions or feedback, feel free to reach out:

Fagin Hales - hales.fagin@gmail.com