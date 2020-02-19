# Introduction

This page includes progress information and personal experiences on the homeworks of CENG795 Advanced Ray Tracing class. It is under development and updated frequently. Owner of this page is Bahadir Coskun.

# Content

This section includes different versions of the Raytracer. In every new version, a new feature is added. Each version has it's own subsection. These subsections include brief explanations of what is being added in that version, what kind of bugs have been encountered and how did I manage to fix them, my personal experiences in that homework and conclusion. If applicable, time comparisons of different implementations are also added in these subsections.

## 1. Basic Raytracer (v0.1)

This is the very first version of the Raytracer. In this first version of Raytracer, I focused on ray-object intersection, basic shading and shadows. There is no acceleration technique used in this version.

### 1.1 Added features
### 1.1.a. Ray-Object Intersection
Supported object types in the Basic Raytracer are Sphere, Triangle and Mesh which consist of multiple Triangles.

### 1.1.b. Basic Shading
Supported shading models in the Basic Raytracer are Diffuse Shading, Blinn-Phong (Specular) Shading and Ambient Shading. Ambient Shading is used to ease complex light computations.

### 1.1.c. Shadow
Basic Raytracer supports shadows. Shadow computation is straight-forward. If there is an object between a point and light source, that point is considered to be in shadow with respect to the light source.

### 1.2 Bugs and fixes
![Sc](/assets/blackdots.jpg)

### 1.3 Conclusion
