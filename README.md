# Introduction

This page includes information of progress and personal experience on the homeworks of CENG795 Advanced Ray Tracing class. It is under development and updated frequently.

# Content

This section includes different versions of the Raytracer. In every new version, a new feature is added progressively. For each version there is a subsection under this section.

## 1. Basic Raytracer

In this first version of Raytracer, I focused on ray-object intersection, basic shading and shadows. There is no acceleration technique used in this version.

### 1a. Ray-Object Intersection
Supported object types in the Basic Raytracer are Sphere, Triangle and Mesh which consist of multiple Triangles.

### 1b. Basic Shading
Supported shading models in the Basic Raytracer are Diffuse Shading, Blinn-Phong (Specular) Shading and Ambient Shading. Ambient Shading is used to ease complex light computations.

### 1c. Shadow
Basic Raytracer supports shadows. Shadow computation is straight-forward. If there is an object between a point and light source, that point is considered to be in shadow with respect to the light source.

### 1d. Bugs and fixes
![Sc](/assets/cornellbox.jpg)
