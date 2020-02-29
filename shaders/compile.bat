%VULKAN_SDK%/Bin32/glslc.exe shaders/shader.vert -o shaders/vert.spv
%VULKAN_SDK%/Bin32/glslc.exe shaders/shader.frag -o shaders/frag.spv
%VULKAN_SDK%/Bin32/glslc.exe shaders/imgui.vert -o shaders/imguiVert.spv
%VULKAN_SDK%/Bin32/glslc.exe shaders/imgui.frag -o shaders/imguiFrag.spv
%VULKAN_SDK%/Bin32/glslc.exe shaders/raytrace.rchit -o shaders/rchit.spv
%VULKAN_SDK%/Bin32/glslc.exe shaders/raytrace.rgen -o shaders/rgen.spv
%VULKAN_SDK%/Bin32/glslc.exe shaders/raytrace.rmiss -o shaders/rmiss.spv