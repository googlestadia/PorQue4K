# Checkerboard Rendering

## Background

### Forerunners
Graphics programmers have used a variety of techniques to hit a target
resolution without using target resolution render targets throughout the render
pipeline.

Interlaced rendering is a classic technique where every other row or column of
pixels is rendered in an image. The full frame is reconstructed from the current
and previous frame. This reduces the pixel shading by half per frame.

Related to that technique is to render a lateral half of an image, instead of
every other row/column. For example, Frame N would render the left half of a
frame, and Frame N+1 would render the right half.

One of the more popular techniques is to actually render a smaller resolution
than the target resolution (e.g. render 1440p for a 2160p target). Then use a
custom upscale/resolve to generate a target resolution image.

### Initial Gen8 Efforts

Guerilla Games shared some of their rendering tech behind
[Killzone Shadow Fall][Valient14]. They used vertical interlaced rendering to
achieve 1080p 60fps in multi-player (compared to 1080p 30fps for single-player).
They used multiple frames of history, and temporal reprojection to improve
reconstruction of the current frame.

Ubisoft Montreal implemented _perhaps_ the first version of checkerboard for
[Rainbow Six Siege][ElMansouri16]. They made an initial attempt with vertical
interlaced, based on Guerilla's efforts. Not only the novel idea of using a
checkerboard pattern to re-constitute a full frame, but also leveraged MSAA 2X
to get the correct pixel positions.

### Leap to 4K

With the advent of consoles touting 4K capability, games were faced with the
challenge of rendering 2160p with the same image quality, but without the
raw GPU power to handle a direct upscale. Checkerboard was popularized
by the PS4 Pro and their launch titles. Checkerboard was a good fit for the GPU
performance increase.

In order to support developers, Sony made some enhancements to the HW and API to
address some of the technical shortcomings from previous GPUs/APIs.

* Explicit control of color and depth samples
* API control of texture gradient adjust
* Object/Primitive ID generation

## Implementation Details

Our goal is to generate a full resolution image by rendering half of the
target pixels per frame in an alternating checkerboard pattern, and using a
custom compute resolve to reconstruct our image.

![Example of checkerboard images resolving to full resolution][simple_cb]

In order to do this, we have to poke a variety of aspects of the Vulkan and HLSL
surface. The bulk of the work can be described as:

* Create checkerboard-aware render targets and pipelines
* Toggle checkerboard samples between frames
* Write custom checkerboard resolve

### Vulkan Setup

Before we can begin rendering, we have some setup work we need to do to prepare
our render pipeline for checkerboard rendering with Vulkan.

#### Sample Rate Shading Enabled on Device

When you create your device, make sure `sampleRateShading` is set to `VK_TRUE`
as part of  `pEnabledFeatures`. You can verify that the device supports this
feature by using `vkGetPhysicalDeviceFeatures`.

```c
VkPhysicalDeviceFeatures supported_features = {};
VkPhysicalDeviceFeatures requested_features = {};
vkGetPhysicalDeviceFeatures(physical_device_handle, &supported_features);

if (supported_features.sampleRateShading == VK_TRUE) {
    requested_features.sampleRateShading = VK_TRUE;
}

VkDeviceCreateInfo device_ci = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
device_ci.pEnabledFeatures = &requested_features;
//
// Tons of VkDeviceCreateInfo struct code
//
VkDevice device_handle = nullptr;
vkCreateDevice(physical_device_handle, &device_ci, nullptr, &device_handle);
```

#### Render Target Configuration

We need to create render targets with a couple properties:

* Half the count of target pixels
* Pixels arranged in a checkerboard pattern

We have several options depending on how we want to arrange our checkerboard
pattern. A popular option right now is to use a half-width target, with one
color and two depth/coverage samples per pixel. Every other pixel has the color
sample offset horizontally to create the checkerboard pattern.

![Layout of EQAA pixel quad with 1 color + 2 depth samples][eqaa_pixel_quad]

However, a drawback of this setup is that the gradients are non-uniformly
transformed across ddx and ddy. More importantly, we don't have easy access to
EQAA or CSAA on Stadia or Vulkan!

![Comparison of gradients in half-width EQAA][eqaa_gradients]

Instead, we can use a 2x MSAA half-resolution image (or quarter-resolution,
depending on your preferred nomenclature). For example, if our target is
1920x1080, we'd create a 960x540 MSAA 2X image. We get the half-pixel sample
count in the target. And because of the default arrangement of 2x MSAA, we get
a natural checkerboard pattern.

![Layout of MSAA 2x pixel quad][msaa_pixel_quad]

We still have the issue of gradients being larger than they are in the target
resolution. But in this setup, they are uniformly scaled compared to the
original. We can take advantage of this property!

![Comparison of gradients in quarter-res MSAA][msaa_gradients]

We'll talk more in the [Gradient Adjust](#texture-gradient-adjust) section about
how this works.

In order to create our images properly, we just need to modify some properties
during image creation. We'll adjust our extents to be half-size the target
dimensions, and enable 2 samples.

```c
uint32_t checkerboard_width = target_width / 2;
uint32_t checkerboard_height = target_width / 2;

VkImageCreateInfo image_ci = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
image_ci.extent = {checkerboard_width, checkerboard_height, 1};
image_ci.samples = VK_SAMPLE_COUNT_2_BIT;
```

This needs to be done for color and depth images that need to be
checkerboard-aware.

##### Higher Rasterization Fidelity?

A feature that is popular with modern checkerboard is having full target
resolution coverage/depth samples in order to get full resolution geometry
information. However, the ability to mix attachments of different sample counts,
along with choosing with samples invoke the pixel shader is not available
broadly in Vulkan, and not available on Stadia.

#### Render Pass

For the render passes that will use checkerboard images, we need to update the
attachment descriptions before render pass creation. This includes the depth
attachment.

```c
VkAttachmentDescription attachment_desc = {};
attachment_desc.samples = VK_SAMPLE_COUNT_2_BIT;
```

Traditional multi-sample enabled render passes would have a resolve attachment,
but in this case, we don't need it. We aren't resolving _down_ to a lower
resolution image.

#### Graphics Pipeline Creation

The last bit of setup we need to do is making sure the relevant graphics
pipelines are compatible with the checkerboard render passes. When creating
a graphics pipeline, the relevant state is contained in
VkPipelineMultisampleStateCreateInfo. `rasterizationSamples` should be set to
`VK_SAMPLE_COUNT_2_BIT`, similar to your images and render pass attachments.
Configuring this will make sure we have enough samples for our checkerboard
image.

However, it does not guarantee that we actually shade at the sample location,
nor does it guarantee that we invoke a unique instance of our pixel shader per
covered sample. This goes against the core principles of MSAA (multiple samples,
single invocation of fragment shader). But we are using MSAA functionality to
take advantage of _some_ of the features, not to use MSAA.

![Difference between shading at sample location and pixel center][shading_location_delta]

We obtain some of our desired intent with `sampleShadingEnable` and
`minSampleShading`. If we set `sampleShadingEnable` to `VK_TRUE` and
`minSampleShading` to `1.0f`, we will get the correct number of unique pixel
shader invocations, evaluated at the sample location.

```c
VkPipelineMultisampleStateCreateInfo pipeline_multisample_ci = 
    { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

pipeline_multisample_ci.rasterizationSamples   = VK_SAMPLE_COUNT_2_BIT;
pipeline_multisample_ci.sampleShadingEnable    = VK_TRUE;
pipeline_multisample_ci.minSampleShading       = 1.0f;

VkGraphicsPipelineCreateInfo graphics_pipeline_ci = 
    { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
graphics_pipeline_ci.pMultisampleState = &pipeline_multisample_ci;
```

We can look at the [Vulkan specification](
https://vulkan.lunarg.com/doc/view/1.2.135.0/windows/chunked_spec/chap26.html#primsrast-sampleshading)
for more information:

```
Sample Shading

...When minSampleShadingFactor is 1.0, a separate set of associated data are
evaluated for each sample, and each set of values is evaluated at the sample
location.

Sample shading is enabled for a graphics pipeline:

* If the interface of the fragment shader entry point of the graphics pipeline
includes an input variable decorated with SampleId or SamplePosition. In this
case minSampleShadingFactor takes the value 1.0.

* Else if the sampleShadingEnable member of the
VkPipelineMultisampleStateCreateInfo structure specified when creating the
graphics pipeline is set to `VK_TRUE`. In this case minSampleShadingFactor takes
the value of VkPipelineMultisampleStateCreateInfo::minSampleShading.
```

The spec points out that we can enable this in the pixel shader itself (more
details in the following section). In fact, it's hard to avoid changes to the
pixel shader for the correct checkerboard functionality, so it might be simpler
to avoid touching `sampleShadingEnable` and `minSampleShading`.

### HLSL Setup

We have some work to do on our pixel shaders to make sure they are
checkerboard compatible. The primary concerns here are performing operations at
the sample location, and adjusting our gradient generation.

#### Sample Location Shading

We covered some of the issues surrounding shading at the sample location,
instead of the pixel center, in the [Vulkan Setup](#vulkan-setup) section.
However, we can configure our shaders to enable sample location shading.

In order to do this, we need to modify the pixel shader input structure. Let's
use an example pixel shader input, pre-checkerboard:

```c
struct PSInput
{
    float4 PositionCS : SV_Position;
    float3 PositionWS : WORLDPOS;
    float3 Normal : NORMAL;
    float2 UV0 : TEXCOORD0;
};

float4 psmain(PSInput input) : SV_Target
{
```

In order to enable sample location shading, we need to use the `sample`
interpolation modifier on the input associated with the `SV_Position` semantic.

```c
struct PSInput
{
    sample float4 PositionCS : SV_Position; // Add the sample modifier
    float3 PositionWS : WORLDPOS;
    float3 Normal : NORMAL;
    float2 UV0 : TEXCOORD0;
};
```

Alternatively, we could use the `SV_SampleIndex` semantic to produce the same
effect.

```c
struct PSInput
{
    float4 PositionCS : SV_Position;
    float3 PositionWS : WORLDPOS;
    float3 Normal : NORMAL;
    float2 UV0 : TEXCOORD0;

    uint sampleIndex : SV_SampleIndex; // Also enables sample location shading
};
```

#### Sample Location Parameter Interpolation

Even though we have _enabled_ sample location shading, our other input
parameters are not automatically interpolated at the sample position! In order
to get this functionality, we have to use those same `sample` interpolation
modifiers on the parameters:

```c
struct PSInput
{
    sample float4 PositionCS : SV_Position;
    sample float3 PositionWS : WORLDPOS;
    sample float3 Normal : NORMAL;
    sample float2 UV0 : TEXCOORD0;
};
```

![Comparison of parameter interpolation modifiers][param_interp_compare]

Because the shader _needs_ to be modified in order to get correct parameter
interpolation, it makes the Vulkan-side sample location shading controls less
appealing.

It could be interesting if there was a Vulkan-side control to force `sample`
interpolation on pipelines without having to modify the shader source. With
that feature, it might be possible to entirely avoid shader source modification
in order to add checkerboard support.

#### Texture Gradient Adjust

GPUs use texture coordinate gradients to determine the correct LOD to sample.
Typically, these are implicitly calculated by the HW by using 2x2 pixel quads,
and calculating the partial derivatives based on screen-space x and y
coordinates.

However, our checkerboard gradients are affected because of how we configured
our render targets. As previously mentioned, gradients are calculated on a pixel
quad basis. Our render targets are half-sized in each dimension (compared to the
target resolution). This means that the distance between neighboring pixels is
_twice_ as far in our checkerboard targets compared to our target resolution.
This will cause our derivatives to be twice as large, leading to lower quality
mips to be sampled.

![Comparison of gradients in quarter-res MSAA][msaa_gradients]

We need to correct our gradients in order to sample the same fidelity of texture
mips as our target resolution render.

##### Manual Gradient Generation

HLSL allows us to manually provide gradients when sampling textures. We can
explicitly generate gradients in the shader by using the `ddx\ddx_fine` and
`ddy\ddy_fine` instructions. Then, we can scale these gradients to match what
we'd get with a target resolution pixel quad.

```c
#define GRADIENT_SCALING_FACTOR 0.5f

    float2 tdx = ddx_fine(input.UV0) * GRADIENT_SCALING_FACTOR;
    float2 tdy = ddy_fine(input.UV0) * GRADIENT_SCALING_FACTOR;

    float4 metallicRoughness = 
        metallicRoughnessTexture.SampleGrad(texSampler, input.UV0, tdx, tdy);
```

In this example, we have the gradient scaling factor sourced from a macro. This
can be sourced from a constant buffer, or wherever else. The sample in this repo
uses both the macro and a constant buffer value for debugging.

While this technique works, it is annoying because we have to instrument all our
texture sampling instructions to adjust for this. This might not be bad
depending on the architecture of your shader code. However, we have an
alternative solution...

##### MIP LOD Bias

Adjusting gradients is pretty typical for checkerboard implementations. However,
in other implementations, it's not _unusual_ for the gradients to be non-uniform
because of how the render target is configured. In our setup, we can uniformly
scale our gradients to get the correct result. And we can take advantage of that
uniform gradient scaling property!

Let's look at how LODs selection is typically calculated: 

```
dux = du/dx
dvx = dv/dx
duy = du/dy
dvy = dv/dy

p = max( (dux^2 + dvx^2)^0.5,
         (duy^2 + dvy^2)^0.5 )
LOD = log2(p)
```

This is before biases, clamps and base mip levels are applied. For simplicity,
we can pretend that the ddx derivatives are going to be 'bigger' than ddy. The
derivations are the same in either case.

```
LOD = log2((dux'^2 + dvx'^2)^0.5)
LOD = 0.5 * log2(dux'^2 + dvx'^2)
```
We'll consider this our 'standard' LOD calculation. Let's factor in our gradient
scaling factor, and work through the calculations.

```
dux' = dux * 0.5
dvx' = dvx * 0.5

LOD' = log2((dux'^2 + dvx'^2)^0.5)
LOD' = 0.5 * log2(dux'^2 + dvx'^2)
LOD' = 0.5 * log2((dux*0.5)^2 + (dvx*0.5)^2)
LOD' = 0.5 * log2((dux^2)*(0.5^2) + (dvx^2)*(0.5^2))
LOD' = 0.5 * log2((0.5^2) * (dux^2 + dvx^2))
LOD' = 0.5 * (log2(0.5^2) + log2(dux^2 + dvx^2))
LOD' = 0.5 * ((2 * log2(0.5)) + log2(dux^2 + dvx^2))
LOD' = 0.5 * ((2 * -1) + log2(dux^2 + dvx^2))
LOD' = 0.5 * (-2 + log2(dux^2 + dvx^2))
LOD' = -1 + (0.5 * log2(dux^2 + dvx^2))
LOD' = -1 + LOD
```

Essentially, instead of adjusting our gradients, we can apply a LOD bias of -1,
and get the same result. This greatly simplifies our shader modifications, as we
can apply the LOD bias in our samplers!

```c
VkSamplerCreateInfo sampler_ci = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
sampler_ci.mipLodBias = -1.f;
```

This conclusion is confirmed in [ElMansouri16]
([video](https://youtu.be/RAy8UoO2blc)).

##### API-side Gradient Adjust

There are platforms that allow for a 2x2 matrix to be programmed in order to
adjust gradient calculations outside the shader (see [Wihlidal17]). This
feature is not supported on Stadia.

##### Extra: Sample Location Shading and Partial Derivatives
It's actually hard to determine the actual behavior for derivatives generated
when sample location shading is enabled on an MSAA target. In 'typical' MSAA
usage, shading happens at the pixel center, so derivatives can just use the
normal pixel quad. Additionally, it's only 1 pixel shader invocation per
primitive.

When we enable sample location shading, we are no longer shading at the pixel
center, and we could have multiple invocations per pixel! So what is the pixel
quad in this setup?

The natural assumption would be to create a quad from samples that share the
same location/sample index. As in, 4 samples at sample index 0 would create the
pixel quad.

We can quickly validate this in a shader by manually calculating the derivatives
of `SV_SampleIndex` values, and writing out the value to the render target. If
our pixel quads share the same sample index, the 0s should be written to the
render target.

```c
    float sampleIndexGradX = ddx_fine(input.sampleIndex);
    float sampleIndexGradY = ddy_fine(input.sampleIndex);
    return float4(sampleIndexGradX, sampleIndexGradY, 0.f, 1.0f);
```

And in our tests on a couple Vulkan implementations, this is what happens. In
reality, if this was not the case, sample location shading would fall over in
a variety of ways (not just with checkerboard!).

Sparse D3D11 Reference info:
https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules#derivative-calculations-when-multisampling

### Scene Rendering

Once we've done with setting up our Vulkan and HLSL state, we can get into the
actual rendering. We have two primary responsibilities during scene rendering:

* Toggle checkerboard frame ID, along with associated state
* Toggle checkerboard samples

Toggling the checkerboard ID and state is relatively straightforward, and left
up to the implementer.

#### Sample Location Toggling

We want to toggle our sample locations every frame. The current frame and
previous frame should generate a full target resolution's worth of pixels.
Basically, we need to make sure the GPU rasterizer generates fragments that
align with the _target_ pixel destinations.

If we think about this as a 'rasterization location' problem, this gives us a
large hint about where we want to manipulate the graphics pipeline: before
rasterization! We have a few options:

* Viewport - By shifting the viewport horizontally, we're able to change which
samples are overlapped by the primitive during rasterization.
* Projection matrix - Jittering the projection matrix is a common technique to
offset primitives during rasterization.
* Sample locations - We can actually change the rasterization sample locations
directly, assuming the sample locations fit our uses and the functionality is
supported on the platform.

##### Viewport

The viewport transform is responsible for putting our primitive into
screen-space coordinates before rasterization. Because of how a MSAA 2X target
arranges sample locations, we can shift the rasterization to get full target
coverage.

Our MSAA 2X targets are half-sized in both width and height. One pixel of
distance in the half-size target corresponds to two pixels of distance in the
full size target. Therefore, if we need to shift one target pixel between
frames, we should shift our viewport a half-pixel.

![How a half-pixel viewport offset provides full pixel coverage][vp_offset]

By default, Vulkan specifies that viewports are static graphics pipeline state.
We need to make sure that we can change the viewport on a per-frame basis, 
unless we want to create alternating pipelines solely to toggle viewports.

```c
VkDynamicState dynamic_state_list[] = { VK_DYNAMIC_STATE_VIEWPORT };

VkPipelineDynamicStateCreateInfo dynamic_state_ci = { 
    VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
dynamic_state_ci.dynamicStateCount = 1;
dynamic_state_ci.pDynamicStates = &dynamic_state_list[0];

VkGraphicsPipelineCreateInfo graphics_pipeline_ci = 
    { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
graphics_pipeline_ci.pDynamicState = &dynamic_state_ci;
```

Once we've done that, we're able to generate per-frame checkerboard viewports,
and set them with the pipelines that can have dynamic viewports.

```c
VkViewport cb_viewport = {};
cb_viewport.x = x;
cb_viewport.y = height; // y-inverted
// Set width, height, min/max depth

uint32_t cb_frame_index = GetFrameIndex() % 2;
cb_viewport.x += 0.5f * cb_frame_index;

VkCommandBuffer cmd_buf = GetCommandBuffer();
vkCmdSetViewport(cmd_buf, 0, 1, &cb_viewport);
vkCmdBindPipeline(cmd_buf, //...
```

##### VK_EXT_sample_locations

<!--- TO INVESTIGATE --->

##### Projection Matrix Jittering

If you are already jittering your projection matrix for other reasons(temporal
AA?), then you can fold the checkerboard jitter into your existing process.

### Checkerboard Resolve

Once we've decided to resolve our full resolution image, we have a number of
resources to build our image. At a minimum, we require our current checkerboard
color target and either the previous checkerboard color or resolved color
target. Additional inputs can be used depending on what is available from the
engine, such as depth or velocity!

Checkerboard resolve inputs:

* Current checkerboard color (required)
* Previous checkerboard or resolved color (required)
* Current checkerboard depth
    * Current full resolution depth (on platforms that support mixed sample count
    attachments)
* Previous checkerboard/full depth
* Current checkerboard velocity
* Current checkerboard/full object/primitive ID

Independent of input, the core algorithm is:

* Current frame pixels are passed through to the output
* 'Empty' checkerboard pixels are extrapolated from current and previous results
    * Augmented by extra data provided by depth, velocity, and geometry IDs

#### Utilizing Depth

`TODO`

#### Utilizing Velocity

`TODO`

#### Performance Considerations 

#### 1 Pixel per thread or 1 Quad per Thread?

The initial implementation launched had one compute thread per output pixel, in
threadgroups of 16x16x1. Each threadgroup was processing 256 pixels. For a 1080p
target, this leads to a dispatch dimensions of 120x68x1. RGP reported the
dispatch time as 106 us, 32640 wavefronts, with an average wavefront time of
~6.3 us.

As an experiment, I tried writing 4 pixels per thread. Conceptually, this is
one thread per 'input' pixel, assuming the 4 MSAA samples (across) two frames
are tied to one pixel location. Alternatively, one thread is piecing together
the smallest checkerboard quad (2x2). In order to match the previous output
pixels/threadgroup ratio, the new threadgroup size is 8x8x1. Each threadgroup
still writes out 256 pixels, but now it's just one wavefront doing the work.
RGP reports a dispatch time of 59 us. We launch 8160 wavefronts with an
average wavefront time of ~12.4 us.

By writing out 4 pixels per thread, we save 44% of dispatch time, even though
our wavefront time roughly doubles.

Currently, I don't have instruction timing data on Stadia, so we can't say what
exactly caused the speedup. I suspect L1 cache re-use increases because of the
increased data locality per wavefront.

It could be interesting to experiment with other pixels-per-thread counts.

## Postprocessing

`TBD, implementation differs before and after checkerboard resolve`

## Future Considerations

### Combine With Temporal AA

### Combine With Dynamic Resolution

### Combine With Sharpener

### Rotated Checkerboard

## Vulkan/Platform Requests

* Offer a facility to _force_ `sampler` interpolation for parameters (aka 
varyings) in graphics pipelines. Right now, we can force sample location
shading at the API level, but parameter interpolation can't be
controlled/enforced.

* RenderDoc supports checkerboard visualization

* `VK_EXT_sample_locations` allows for sample location positions and
interpolation, which the spec says is 'undefined' when using with
`VK_EXT_sample_locations`. Maybe a listing of pre-determined locations that
will work could be nice.

## References
* [Rendering Rainbow Six Siege][ElMansouri16] - Jalal El Mansouri
* [4K Checkerboard in Battlefield 1 and Mass Effect][Wihlidal17] - Graham
Wihlidal
* [Checkerboard Rendering in Dark Souls: Remastered][Purche18] - Markus Purche,
Andreas Vennström
* [Checkerboard Rendering for Real-Time Upscaling on Intel Integrated Graphics][McFerron18]
\- Trapper McFerron, Adam Lake
* [DECIMA ENGINE: ADVANCES IN LIGHTING AND AA][deCarpentier17] - Giliam de 
Carpentier, Kohei Ishiyama
* [TAKING KILLZONE SHADOW FALL IMAGE QUALITY INTO THE NEXT GENERATION][Valient14]
    \- Michal Valient

[Valient14]: https://www.guerrilla-games.com/read/taking-killzone-shadow-fall-image-quality-into-the-next-generation-1
[ElMansouri16]: https://twvideo01.ubm-us.net/o1/vault/gdc2016/Presentations/El_Mansouri_Jalal_Rendering_Rainbow_Six.pdf
[deCarpentier17]: https://www.guerrilla-games.com/read/decima-engine-advances-in-lighting-and-aa
[Wihlidal17]: https://www.ea.com/frostbite/news/4k-checkerboard-in-battlefield-1-and-mass-effect-andromeda
[McFerron18]: https://software.intel.com/en-us/articles/checkerboard-rendering-for-real-time-upscaling-on-intel-integrated-graphics
[Purche18]: https://www.slideshare.net/QLOC

[simple_cb]: images/simple_cb_resolve_example.PNG
[eqaa_pixel_quad]: images/eqaa_half_width_pixel_quad.PNG
[eqaa_gradients]: images/eqaa_half_width_skewed_gradients.PNG
[msaa_pixel_quad]: images/msaa_quarter_size_pixel_quad.PNG
[msaa_gradients]: images/msaa_quarter_size_uniform_gradients.PNG
[vp_offset]: images/viewport_offset.PNG
[shading_location_delta]:images/shading_delta_between_sample_and_center_location.PNG
[param_interp_compare]:images/parameter_interpolation_compare.PNG