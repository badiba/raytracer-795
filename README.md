# INTRODUCTION

This page includes information of progress and personal experience on the homeworks of CENG795 Advanced Ray Tracing class. It is under development and updated frequently.

# CONTENT

## Basic Raytracer

In this first version of Raytracer, I focused on ray-object intersection, basic shading and shadows. There is no acceleration technique used in this version.

### Ray-Object Intersection
Supported object types in the Basic Raytracer are Sphere, Triangle and Mesh which consist of multiple Triangles.

### Basic Shading
Supported shading models in the Basic Raytracer are Diffuse Shading, Blinn-Phong (Specular) Shading and Ambient Shading. Ambient Shading is used to ease complex light computations.

### Shadow
Basic Raytracer supports shadows. Shadow computation is straight-forward. If there is an object between a point and light source, that point is considered to be in shadow with respect to the light source.

### Bugs and fixes
![Sc](assets/cornellbox.png)
