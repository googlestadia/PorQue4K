# Contribution Guidelines

## Upscaling Technique

If you are interested in implementing an upscaling technique, 
feel free to pick out out from the
[list of `upscaler` issues](https://github.com/robbiesri/VulkanPorQue4K/labels/upscaler),
and assign yourself.

Alternatively, if you'd like to implement a technique that isn't listed,
simply create a new issue, tag it with the `upscaler` label,
and add some supporting documentation to the issue (paper, website, Git repo,
etc).

### Add Shader

You'll almost certainly have to add a new compute shader for your technique.

* Add the HLSL file to the `src/app/shaders/` folder.
* Update the `src/app/shaders/build_shaders.sh` script
(though you aren't obliged to use it)
* Update `src/app/CMakeLists.txt` with the new shader.
* Compile the shader!
  * Make sure the compiled SPIR-V binary is added to `assets/shaders/`
  * If you run CMake with `-DBUILD_SHADERS=TRUE`, this will 
  automatically build the shaders you hooked through `src/app/CMakeLists.txt`

Current verified DXC commit is
[9c89a1c2](https://github.com/microsoft/DirectXShaderCompiler/commit/9c89a1c2c6baa76dabc154f126408973848b0069).

### Plumb into application

* Find how shaders are compiled into pipelines by looking at
`VkexInfoApp::SetupShaders`
* Add a toggle to the GUI via `VkexInfoApp::DrawAppInfoGUI`
* Execute your pipeline by adding it to `VkexInfoApp::UpscaleInternalToTarget`
* Allocate + update constant buffers and update descriptor sets

### Add Documentation

Add a new Markdown file to the `docs/` folder, and add a link to
[docs/TECHNIQUES.md](TECHNIQUES.md).

In your new Markdown file, describe the technique you implemented with:

* Supporting documentation - Papers, webpages, Git repos, etcs
* Performance information
* Diagrams
* Implementation details/pointers/code samples

## Other Work

There is plenty of general infrastructure work that needs to be done on the
project!

* [Non-upscaler Enhancements](
  https://github.com/robbiesri/VulkanPorQue4K/issues?q=is%3Aissue+is%3Aopen+label%3Aenhancement+-label%3Aupscaler)
\- These are issues that would provide project enhancements that are not
necessarily tied _directly_ to upscaling.
* [Cleanup](https://github.com/robbiesri/VulkanPorQue4K/issues?q=is%3Aissue+is%3Aopen+label%3Acleanup)
\- Quality-of-life changes needed to improve the project

## Shared Work Etiquette
There might be a scenario where you want to assign yourself an issue, only to
discover that someone is already assigned! Please reach out to the assignee to
see if you can share the work, or take the issue. Perhaps the technique has
variants, and we could support multiple implementations.
