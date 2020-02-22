# Introduction

This page includes progress information and personal experiences on the homeworks of CENG795 Advanced Ray Tracing class. It is under development and updated frequently. Owner of this page is Bahadir Coskun.

# Content

This section includes different versions of the Raytracer. In every new version, new features are added. Each version has it's own subsection. These subsections include;

- brief explanations of what is being added in that version, 
- what kind of bugs have been encountered and how did I manage to fix them, 
- my personal experiences. 

If applicable, time comparisons of different implementations are also added in these subsections.

## 1. Basic Raytracer - v0.1 - Week 1 & 2

This is the very first version of the Raytracer. In this version, I focused on ray-object intersection, basic shading and shadows. There is no acceleration technique used in this version.

### 1.1 Added features

### 1.1.a XML Parser and Scene
In this project, `Scenes` will be parsed from an XML file. Therefore, first utility that is added to the project is an XML parser. These `Scenes` will evolve with the project since there will be added components in them. Currently a `Scene` includes `Cameras`, `Point Lights`, `Materials`, and `Objects` along with their specifications which are details and will be avoided in this page.

### 1.1.a. Ray-Object Intersection
Supported object types in the Basic Raytracer are `Sphere`, `Triangle` and `Mesh` which consist of multiple `Triangles`. A `ray` is a half-line (it has an origin and one direction only). In this version, we will be casting rays from the camera (see [Ray Casting](https://en.wikipedia.org/wiki/Ray_casting)). These `rays` will pass through each and every pixel and determine the color. 

### 1.1.b. Basic Shading
Supported shading models in the Basic Raytracer are Diffuse Shading, Blinn-Phong (Specular) Shading and Ambient Shading. Ambient Shading is used to ease complex light computations.

### 1.1.c. Shadow
Basic Raytracer supports shadows. Shadow computation is straight-forward. If there is an object between a point and light source, that point is considered to be in shadow with respect to the light source.

### 1.2 Bugs and fixes
![Sc](/assets/blackdots.jpg)

### 1.3 Conclusion
