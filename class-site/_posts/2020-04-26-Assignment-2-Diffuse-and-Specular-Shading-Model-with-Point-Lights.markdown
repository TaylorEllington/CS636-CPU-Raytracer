---
layout: post
title:  "Diffuse and Specular Shading with Point Lights"
date:   2020-04-26 00:00:00 -0800
categories: lighting shading
visible: 1
---

## Required Images

The assignment did not require any specific references to the code-base to appear in the README document. Should you like to look at the code in more depth, it is available [here in my gitlab](https://gitlab.com/TaylorEllington/cs636-advanced-rendering-techniques).

That gitlab also holds the code for this website, the C++ is in the `src/` directory.

### Supertoroid and Spheres
This supersampled 512x512 image contains 2 white lights, 3 supertoroids and 4 spheres (3 in the foreground and one big one in the back) The full scene description with positions, shading parameters, and others settings can be viewed [here](https://gitlab.com/TaylorEllington/cs636-advanced-rendering-techniques/-/blob/78479ca5a6a1d2dfb43439d28b928cb36916429c/assets/solarized-sphere-and-supertoroid-scene.json). More on this file and its role can be found below in a later section. 

![Supersampled Solarized Spheres and Supertoroid - HW1](/cs636-advanced-rendering-techniques/images/HW_2/ss-solar-spheres-and-supertoroid.png)  

The image took 13240ms total, the log below shows how much of that was spent in raytracing (including shading) and how much was spent in full screen passes to normalize colors and sample the image down from the super-sized 1024x1024.

{% highlight text %}
Mesh - Loaded mesh: assets/supertoroid.smf with 64 verts and 129 faces
Mesh - Loaded mesh: assets/supertoroid.smf with 64 verts and 129 faces
Raytracer - Performing inital setup with:
        Camera at vec4(0.000000, 0.000000, 4.250000, 1.000000)
        Camera Space - X: vec3(1.000000, -0.000000, 0.000000)
        Camera Space - Y: vec3(0.000000, 1.000000, 0.000000)
        Camera Space - Z: vec3(0.000000, 0.000000, -1.000000)

        Dist to view plane 1

        View angle (DEG) 56

        View Plane center at: vec3(0.000000, 0.000000, 3.250000)
        View Plane upper left at: vec3(-0.531709, 0.531709, 3.250000)

Raytracer - Supersampling enabled, computations will be done on an 1024 x 1024image
Raytracer - Starting raytrace of scene..........Raytracer - full screen color norm pass
Raytracer - Supersampling image to512 x 512
DONE
Raytracer - Tracing took           13223ms
Raytracer - image proccessing took 17ms
Raytracer - Total time: 13240ms
Screen - Writing 262144 pixels to 512 by 512 file: ss-solar-spheres-and-supertoroid.png
{% endhighlight %}

### Another one


This image is yet to be generated, were getting there

## Work in progress images

These images were created as I was bringing Phong shading online during development and as a result they don't really meet the criteria for the end result of the assignment, however I liked the way they looked and thought they were worth sharing. 

The first thing I did was add color to the homework 1 code, giving each scene object its own RGB value that the ray trace intersection method would return, this meant the raytracing loop could start do add color to the scene. At this point I was still setting the nxm grid of pixels to a flat background color. 

![flat spheres](/cs636-advanced-rendering-techniques/images/HW_2/solarized_spheres.png)


The next step was to implement lights, and start to turn on the various components of phong shading. I did individual passes for the diffuse and ambient steps, however in all cases adding the specular component caused my lighting values to exceed acceptable thresholds and my images have bizarre unsightly color aberrations that aren't worth showing. 

![diffuse spheres](/cs636-advanced-rendering-techniques/images/HW_2/diffuse_spheres.png)
![ambient spheres](/cs636-advanced-rendering-techniques/images/HW_2/ambient_spheres.png)

At this point I took a break to do some non-visible improvements such as a full image pass to normalize color intensities, add all the Phong shading values to the json file format so I could specify per object characteristics, enable supersampiling,  and some other minor refactoring to get certain code paths more readable. At the end of which I managed to get what I think is the coolest looking image of the set.

![Fully Raytraced Spheres](/cs636-advanced-rendering-techniques/images/HW_2/ss-five-spheres.png)

## Whats with this Json file?

Something I identified very early on as a pain point (back in HW1) was the need to write repetitive code to position objects in a scene. Issues included losing good known scenes if you had to write a new one, tons of commented out lines, and other small issues that stacked up. To fix this I took an evening and pulled in a single header json reader library and spun all of my scene description information out into stand alone files. This means I can easily generate a library of scenes and keep them around to test future changes to the RayTracer against. An additional perk is that I can change details of a scene with out trigging a recompilation.  I store these scenes in the top level assets/ directory along with the smf files, in case you want to look. Json is fairly easy to read however the parsing is fairly error intolerant at the moment, so if you are at all interested in modifying them, note that the attributes of most structures are required, and the layout is not very flexible.

## Known Current Limitations of the Tool

While I completed everything necessary for HW 2, there are certain things I noticed that could be improved, some of which I know are part of incoming homeworks:  
*  Slow - even with small models, checking every object against the ray is expensive, HW3 brings acceleration
*  No Light Occlusion - Right now, shading calculations do not check if the light source is occluded by other scene objects, so shading is slightly inaccurate. 
*  Messy - the `RayTracer::Run()` method has grown in to an ugly behemoth, it needs some function extraction simply for readability's sake. I would also like to break Phong shading into its own series of functions. 
* Color norming - Can misbehave on images with very low color channel values. Right now the norming function is focused on bringing intensity values >1 back into the [0...1] range, this part works well, however if a particular channel's max value is small, that can cause issues when that channel is renormalized, the current work around is to just ensure any scenes have "safe" colors with high enough values in all 3 channels. The Solarized color pallet has worked well for this. 