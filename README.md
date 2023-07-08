# Darkness

Rendering engine and editor

Editor UI uses Qt.

![Editor](https://github.com/Karmiska/Darkness/blob/master/editor.png "Editor screenshot")

<p>Engine description</p>

<p>Fully GPU driven graphics architecture.
GPU based frustum and occlusion culling using cluster culling. (roughly the same as the one presented here http://advances.realtimerendering.com/s2015/aaltonenhaar_siggraph2015_combined_final_footer_220dpi.pdf)</p>

<p>Physically based lighting model (PBR) supporting the most common light types (directional, spot, point).
Also supporting image based lighting.</p>

![Substance](https://github.com/Karmiska/Darkness/blob/master/substance.png "Substance painter materials")
![Substance2](https://github.com/Karmiska/Darkness/blob/master/substance_metal.png "Substance painter materials 2")

<p>Temporal Antialiasing.
PCF Shadows.
Screen space ambient occlusion.</p>

<p>Light probes.</p>

![Probes](https://github.com/Karmiska/Darkness/blob/master/light_probe.png "Light probe")

<p>Supports both DX12 and Vulkan backends.</p>

<p>Shader hotreload (both in editor/in game).</p>
<p>Distributed asset processing.</p>

<p>This project is likely never going to be finished nor production quality. It is only a tool for me to study the subject.</p>

<p>Building it, while possible, is going to take considerable setup since only the source is available here. There are numerous external project requirements which are not included here. Reason for this is simply the sheer size of the project.</p>
