# Introduction

This page includes technical information and personal experiences on the homeworks of CENG795 Advanced Ray Tracing class. It is under development and updated frequently. Owner of this page is Bahadir Coskun.

# Content

This section includes different versions of the Ray Tracer. In every new version, new features are added. Each version has it's own subsection. These subsections include;

- brief explanations of what is being added in that version, 
- resulting images in that version,
- what kind of bugs have been encountered and how did I manage to fix them, 
- my personal experiences. 

If applicable, time comparisons of different implementations are also added in these subsections.

## 1. Basic Ray Tracer - Week 1 & 2

This is the very first version of the Ray Tracer. In this version, I focused on ray-object intersection, basic shading and shadows. There is no acceleration technique used in this version.

### 1.1. Added features

### 1.1.a. XML Parser and Scene
In this project, `Scenes` will be parsed from an XML file. Therefore, first utility that is added to the project is an XML parser. These `Scenes` will evolve with the project since there will be added components in them. Currently a `Scene` includes `Cameras`, `Point Lights`, `Materials`, and `Objects` along with their specifications which are details and will be avoided in this page.

### 1.1.b. Ray-Object Intersection
Supported object types in the Basic Ray Tracer are `Sphere`, `Triangle` and `Mesh` which consist of multiple triangles. A `Ray` is a half-line (it has an origin and one direction only). In this version, we will be casting rays from the camera (see [Ray Casting](https://en.wikipedia.org/wiki/Ray_casting)). These rays will pass through each and every pixel, intersect with the objects in the scene and determine the color of that pixel.

### 1.1.c. Basic Shading
When a ray intersects with an object, shading is used to determine the color at the intersection point. Supported shading models in the Basic Ray Tracer are `Diffuse Shading`, `Blinn-Phong (Specular) Shading` and `Ambient Shading` (See [Shading](https://en.wikipedia.org/wiki/Shading)).

### 1.1.d. Shadow
Basic Ray Tracer supports shadows. Shadow computation is straight-forward. If there is an object between a point and light source, that point is considered to be in shadow with respect to that light source.

### 1.2. Resulting Images
Here are some scenes rendered with Basic Ray Tracer. Rendering time is also included.

![Sc](/assets/hw1-simple-correct.jpg)

> Figure 1.2.1: Simple scene. Time: 0.309s

![Sc](/assets/hw1-sphere-correct.jpg)

> Figure 1.2.2: Spheres scene. Time: 0.342s

![Sc](/assets/hw-cornellbox-correct.jpg)

> Figure 1.2.3: Cornellbox scene. Time: 0.902s

![Sc](/assets/hw1-bunny-correct.jpg)

> Figure 1.2.4: Bunny scene. Time: 1m26.950s

![Sc](/assets/hw1-scienceTree-correct.jpg)

> Figure 1.2.5: Science Tree scene. Time: 3m8.178s

Since the images are compressed to `jpeg` format, their quality is lower than the original images.

Currently it takes too much time to render these scenes. In the next version, we will speed up rendering by using BVH and multithreading.

### 1.3. Bugs and fixes
I encountered several bugs during the implementation of Basic Ray Tracer. If you look closely at the below image, you will see that there are black dots on the gray square behind the spheres. The reason behind this is `Intersection Epsilon Value`. This value is used to tolerate floating point errors for intersection calculations. I forgot to set a default value for this variable. When the program tried to access this variable it was just a random number. Intersection tests were failing due to this problem resulting in the black dots as seen below.

![Sc](/assets/blackdots.jpg)

 > Figure 1.3.1: Black dots on cornell box.

Obviously this wasn't the only scene that the black dots appeared. Moreover, at different runs, this bug kept appearing and disappearing since at each run it had a random value. This helped me to understand the reason behind it. Setting a default value obviously fixed it.

Another bug was present because of the miscalculation of shadows. To check if an object is under shadow we send shadow rays. The purpose of these shadow rays are to check if there is an object between the intersection point and the light source. The problem was that I wasn't checking if the object is inbetween. If the object is behind the light source, shadow ray will still intersect with that object. Correct implementation should check whether the object is behind the light source or not. I wasn't doing that which resulted in incorrect shadows as seen below.

![Sc](/assets/hw1-cornellbox-pages.jpg)

> Figure 1.3.2: Incorrect shadows on cornell box.

The another bug can be observed at the bunny scene. It was caused by the wrong implementation of the intersection of a `ray` with a `Mesh`. I was not checking the nearest intersection point therefore the first intersection was being returned. First intersection point can be on the backface of the bunny. If that is the case, intersection will be under shadow which will result in black triangles on bunny as seen below.

![Sc](/assets/hw1-bunny-pages.jpg)

> Figure 1.3.3: Black triangles on bunny.

Finally, I was not paying attention to correcting a non-orthogonal camera vectors. For simplicity, camera vectors in XML file may not be orthogonal. It is programmers responsibility to correct these vectors. This bug caused my camera to have incorrect angle.

### 1.4. Conclusion

At this point, I'm very excited to work on newer versions of the Ray Tracer. With `Reflection` and `Refraction` our images will definitely look better. However, I am kind of worried about the possible unnoticed bugs in the current version. If there are bugs which I could not find, they will probably a big problem when it comes to implement more complex features. I obviously try to keep everything as simple as possible and write my code clean. But as a student, time is the most valuable resource and it's very limited.

