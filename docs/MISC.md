# Miscellaneous Topics

## Enabling FP16 for Shader Use on AMD Vega

### Background

After my first pass at the checkerboard resolve shader, I wanted to investigate
ways to relieve register pressure in order to squeeze in another wavefront. FP16
came to mind since I'd be able to half my register usage in certain cases, and
it _seemed_ like it would be relatively easy to implement. Just replace `float`
with `half` and that's it, right?

I started by searching for any articles/blogs documenting how to use FP16 with
HLSL, DXC, SPIR-V, Vulkan, and AMD. I found two blog posts:

* [mjp - Half The Precision, Twice The Fun: Working With FP16 In HLSL][mjp_fp16]
* [AMD GPUOpen - First Steps When Implementing FP16](https://gpuopen.com/learn/first-steps-implementing-fp16/)

`mjp`'s blog is far superior to the AMD blog, which elided over a lot of the
actual implementation behaviors. The GPUOpen post doesn't even mention DXC,
Vulkan or SPIR-V.

From these blogs, I learned that it wouldn't be anywhere as simple as I hoped,
mostly because I learned that `half` maps right to `float` in HLSL. My co-worker
[aellem](https://github.com/aellem) pointed me to an
[HLSL doc page on scalar types](https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-scalar).

For me to use FP16 in my shader, it seemed I'd have to do two things:

1. Generate FP16 code from my HLSL
   1. Use `min16float*` types to request minimum (aka relaxed or flexible)
      16-bit precision for floats. The underlying implementation can 'upgrade'
      them to 32-bit precision since I'm only requesting a lower bound on
      precision.
   1. Force 16-bit precision on `half` by using `-enable-16bit-types` when
      compiling my shaders with DXC.
1. Enable FP16 extensions on my Vulkan device

### Codegen Attempt 1

Changing _just_ the shader code seemed appealing (compared to shader code and
compiler options), so I tried using `min16float`. I decided to use
[Shader Playground][shader_playground] to quickly verify my ISA codegen.

[Sample shader using min16float](http://shader-playground.timjones.io/1c038e1bef0c88f1d1001ba4044aa39e)

I was able to use the RGA backend to inspect the ISA that _should_ be generated
based on my HLSL source. Once I looked at the ISA, I saw it was using some of
the instructions I expected (`v_pk_min_f16`, `v_cvt_pkrtz_f16_f32`). More
importantly, my generated ISA was saving 10 VPGRs compared to my 32-bit float
shader (42 vs 52). This seemed like decent validation for my approach, so I
moved forward with this approach.

### Codegen Verification

While Shader Playground is great for quick offline codegen verification, it
doesn't guarantee the actual behavior of the live driver + platform combo. It's
surprisingly tricky to actually easily verify the generated ISA of a live
application.

I was vaguely aware of an AMD extension to query the ISA, but when I actually
tried to find it, I couldn't 'discover' the extension easily. After revisiting
this for the purposes of this document, I was able to find
[VK_AMD_shader_info][AMD_shader_info], which does expose the ability to get
disassembly back from the implementation. In retrospect, I think I would have
used this functionality if I knew about it.

The other path I considered was using [RGA][RGA] directly. However, RGA doesn't yet
support the HLSL -> SPIR-V workflows. I could give it the generated SPIR-V,
but this multi-step pipeline wasn't appealing to me.

Which finally left me with [RGP][RGP]. RGP has the ability to capture the ISA
from pipelines in the trace. While it may _seem_ strange that I'd use a
profiling tool to debug my ISA generation, in practice, this was the 'easiest'
path.

### Device Extensions

#### Which Extensions to Use?

Now that I had FP16-enabled SPIR-V, and a way to verify ISA, I was ready for the
final step of enabling the device functionality!

There are two extensions I had to be aware of in order to enable FP16
functionality (plus a bonus third one):

* [VK_KHR_shader_float16_int8][KHR_shader_float16_int8] - Enable fp16/int8
  arithmetic in shaders
* [VK_KHR_16bit_storage][KHR_16bit_storage] - Enable 16-bit shader input/output
* [VK_KHR_8bit_storage][KHR_8bit_storage] - Enable 8-bit shader input/output

I was surprised to learn that the 16-bit data interfaces were separated from the
shader execution. Initially, I thought I didn't care, and I could just ignore
the `storage` extensions because I was just trying to save register pressure. I
was ok with the conversion happening after loading data. However, when I changed
the shader resources to use 32-bit `float` types, my VGPR register usage spiked
back to 52.

[Sample shader using min16float for math, float for resources](http://shader-playground.timjones.io/b39008f93bfc5499b0dece3b15dfd687)

Based on these results, I figured I'd have to use the `storage` extensions to
ensure my register savings.

#### How to Use the Extensions

My initial expectation was that I'd just add the extensions to
`VkDeviceCreateInfo.ppEnabledExtensionNames`, and I'd get the expected
functionality. However, when I inspected the extension specs, I realized that
utilized the `KHR_get_physical_device_properties2` style of obtaining
feature/property information. This process involves passing feature structures
into `vkGetPhysicalDeviceFeatures2` via `VkPhysicalDeviceFeatures2.pNext`.

Here is the structure for `VkPhysicalDeviceShaderFloat16Int8FeaturesKHR`:

```
typedef struct VkPhysicalDeviceShaderFloat16Int8FeaturesKHR {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           shaderFloat16;
    VkBool32           shaderInt8;
} VkPhysicalDeviceShaderFloat16Int8FeaturesKHR;
```

When I looked at the spec for `VkPhysicalDeviceShaderFloat16Int8Features`, I was
a bit confused by this passage:

> The VkPhysicalDeviceShaderFloat16Int8Features structure can also be included
> in the pNext chain of a VkDeviceCreateInfo structure, in which case it
> controls which additional features are enabled in the device.

The way I read this, it _appeared_ to me that if I wanted to enable/disable
specific aspects of the extension, then I would pass the structure in during
device creation. But if I just wanted the default implementation support for the
extension, then I could just omit the structure. That interpretation is _wrong_,
and I am obliged to pass the structure in via `VkDeviceCreateInfo.pNext`.

Briefly, in order to enable device functionality, I have to:

1. Query the implementation capabilities via `vkGetPhysicalDeviceFeatures2`
1. Enable the requested extensions in `VkDeviceCreateInfo.ppEnabledExtensionNames`
1. Enable the requested feature caps in `VkDeviceCreateInfo.pNext`

### Codegen Attempt 2

Now that everything was setup, I was ready to verify my higher occupancy compute
shader! I kicked off the app, took a RGP capture, and looked at the ISA! And...I
discovered that the VGPR count was the same as before. I inspected the ISA, and
it hadn't changed from previous iterations. No evidence of any FP16
instructions.

After some hacking around with compiler options and device creation, I realized
I would not be successful in my attempts. Which brought me back to my options
for generating FP16 code:

1. Use `min16float`
1. Use `half` and DXC option `-enable-16bit-types`

I recompiled the shader with `-enable-16bit-types` (without verifying in Shader
Playground). I did check the SPIR-V, and it was generating code that I expected
according to the `mjp` blog. I launched the application and...it was running
with FP16! The behavior was the same across Windows and Stadia, both using Vega
GPUs. Technically, `min16float` doesn't _guarantee_ FP16 usage, but I certainly
expected it to work based on the ISA that DX12 RGA exposed.

### Summary

1. Read `mjp`'s [blog entry][mjp_fp16] on FP16
1. Use `half` + `-enable-16bit-types` to guarantee FP16 codegen 
1. Experiment with [Shader Playground][shader_playground] to test codegen
   theories.
1. Query physical device FP16 features, and pass them back during device
   creation
1. Verify codegen with RGP or `VK_AMD_shader_info`

[mjp_fp16]: https://therealmjp.github.io/posts/shader-fp16/
[shader_playground]: http://shader-playground.timjones.io/

[AMD_shader_info]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_AMD_shader_info.html
[KHR_shader_float16_int8]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_shader_float16_int8.html
[KHR_16bit_storage]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_16bit_storage.html
[KHR_8bit_storage]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_KHR_8bit_storage.html

[RGA]: https://github.com/GPUOpen-Tools/radeon_gpu_analyzer
[RGP]: https://github.com/GPUOpen-Tools/radeon_gpu_profiler