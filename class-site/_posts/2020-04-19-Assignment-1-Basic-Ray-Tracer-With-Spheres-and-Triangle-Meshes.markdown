---
layout: post
title:  "Basic Ray Tracer with Spheres and Triangle Meshes"
date:   2020-04-19 00:00:00 -0800
categories: ray-tracing 
visible: 1
---


![Final RayTraced image - HW1](cs636-advanced-rendering-techniques/images/hw_1/traced_image.png)

Here it is! this image contains 4 spheres placed along the x and y axis equidistant from the origin, as well as the supertoroid smf file, placed at the origin, rotated 45 degrees.

The image is 512x512 and the ray tracing section (excluding set up and image writing) took 1452ms

{% highlight text %}
Screen - Setting up image as 512 by 512 with debug mode on
Raytracer - Performing inital setup with:
        Camera at vec4(0.000000, 0.000000, 6.000000, 1.000000)
        Camera Space - X:vec3(1.000000, -0.000000, 0.000000)
        Camera Space - Y:vec3(0.000000, 1.000000, 0.000000)
        Camera Space - Z:vec3(0.000000, 0.000000, -1.000000)

        View Plane center at: vec3(0.000000, 0.000000, 5.000000)
        View Plane upper left at: vec3(-1.000000, 1.000000, 5.000000)

starting raytrace of scene......
Raytracer - tracing took 1452ms
Screen - Writing 262144 pixels to 512 by 512 file: traced_image.png

{% endhighlight %}


Here are some of the other images I generated along the way.

![Diagonal](cs636-advanced-rendering-techniques/images/hw_1/diag.png)
![Oops](cs636-advanced-rendering-techniques/images/hw_1/oops.png)
![First Sphere](cs636-advanced-rendering-techniques/images/hw_1/first_sphere.png)
![transforms](cs636-advanced-rendering-techniques/images/hw_1/transforms.png)

