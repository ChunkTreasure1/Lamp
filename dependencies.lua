VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLM"] = "%{wks.location}/Lamp/vendor/glm"
IncludeDir["STB"] = "%{wks.location}/Lamp/vendor/stb_image"
IncludeDir["Assimp"] = "%{wks.location}/Lamp/vendor/assimp/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["ImGui"] = "%{wks.location}/Lamp/vendor/imgui"
IncludeDir["ImNodes"] = "%{wks.location}/Lamp/vendor/imnodes"
IncludeDir["GLFW"] = "%{wks.location}/Lamp/vendor/glfw/include"
IncludeDir["spdlog"] = "%{wks.location}/Lamp/vendor/spdlog/include"
IncludeDir["glad"] = "%{wks.location}/Lamp/vendor/glad/include"
IncludeDir["spdlog"] = "%{wks.location}/Lamp/vendor/spdlog/include"
IncludeDir["fmod"] = "%{wks.location}/Lamp/vendor/fmod/include"
IncludeDir["PhysX"] = "%{wks.location}/Lamp/vendor/PhysX/include"
IncludeDir["yaml"] = "%{wks.location}/Lamp/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Lamp/vendor/ImGuizmo/include"
IncludeDir["rapidxml"] = "%{wks.location}/Lamp/vendor/rapidxml"
IncludeDir["ktx2"] = "%{wks.location}/Lamp/vendor/ktx2/include"
IncludeDir["Optick"] = "%{wks.location}/Lamp/vendor/Optick/src"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["Assimp"] = "%{wks.location}/Lamp/vendor/assimp"
LibraryDir["PhysX"] = "%{wks.location}/Lamp/vendor/PhysX/lib/%{cfg.buildcfg}"
LibraryDir["fmod"] = "%{wks.location}/Lamp/vendor/fmod"
LibraryDir["ktx2"] = "%{wks.location}/Lamp/vendor/ktx2/lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK_Debug}/VkLayer_utils.lib"
Library["Assimp"] = "%{LibraryDir.Assimp}/assimp-vc143-mt.lib"
Library["ktx2"] = "%{LibraryDir.ktx2}/ktx_release.lib"
Library["ktx2_debug"] = "%{LibraryDir.ktx2}/ktx_debug.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["ShaderC_Utils_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_utild.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["ShaderC_Utils_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_util.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Library["PhysX"] = "%{LibraryDir.PhysX}/PhysX_static_64.lib"
Library["PhysXCharacterKinematic"] = "%{LibraryDir.PhysX}/PhysXCharacterKinematic_static_64.lib"
Library["PhysXCommon"] = "%{LibraryDir.PhysX}/PhysXCommon_static_64.lib"
Library["PhysXCooking"] = "%{LibraryDir.PhysX}/PhysXCooking_static_64.lib"
Library["PhysXExtensions"] = "%{LibraryDir.PhysX}/PhysXExtensions_static_64.lib"
Library["PhysXFoundation"] = "%{LibraryDir.PhysX}/PhysXFoundation_static_64.lib"
Library["PhysXPvdSDK"] = "%{LibraryDir.PhysX}/PhysXPvdSDK_static_64.lib"

Library["fmod"] = "%{LibraryDir.fmod}/fmod_vc.lib"
Library["fmodstudio"] = "%{LibraryDir.fmod}/fmodstudio_vc.lib"