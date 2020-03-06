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

## 2. Recursive Ray Tracing - Week 3 & 4

Fast rendering is very important in Ray Tracing and Basic Ray Tracer is really slow. It takes minutes for not too complex scenes as mentioned in the earlier version. When I tried to run the Basic Ray Tracer for complex scenes which include around 1.8 Million triangles, it took nearly half an hour to get the image. This is for sure unacceptable. Therefore, I first focused on increasing the performance of the Ray Tracer.

The second issue I focused on is reflection and refraction. With these features, we can simulate glass-like, metal, and mirror objects. This definitely increases the realism of our images.

### 2.1. Added features

### 2.1.a. Bounding Volume Hierarchy

Bounding Volume Hierarchy (`BVH`) is used to increase the performance of the Ray Tracer. Normally we were checking the intersection of our rays with all objects in the scene. With `BVH` we put objects into a `Bounding Box`. Then we only check the intersection of ray with this box. If ray does not intersect with the box then it cannot intersect with the objects inside it.

We put bounding boxes recursively by dividing the objects into smaller parts. After each box-ray intersection, bounding boxes get smaller and smaller until we find the primitive that our ray truely intersects. This helps us to avoid many intersection and increase the performance of the Ray Tracer.

### 2.1.b. Reflection

Reflection is used to simulate mirror-like objects. When a ray intersects with a mirror object, it bounces off from the surface and continues to intersect with other objects in the scene. This bounced ray will return a color from the intersection with other objects which will be used to determine the color on the miror.

### 2.1.c. Dielectric Material Support

`Dielectric Material` is used to simulate glass-like objects. When a ray intersects with a glass-like object, it will divide into two rays. These two rays are smaller in magnitude compared to the original ray. One of these rays is bounced off from the surface like it does when it intersects with a mirror. This bounced ray will make glass-like object to partially behave like a mirror (which also happens in real life too).

The second ray is refracted and it passes through the object. When the ray is inside the object, we check the exit point of it. Depending on the angle, ray may not be able to exit which is called `total internal reflection`. If it is able to exit, then it will try to intersect with other objects in the scene and it will return a color from those intersections. This color is used to simulate the transparency of the glass-like objects.

Finally, `Beer's Law` is used to compute the attenuation of light travelling inside the object and `Fresnel Reflectance` is used to compute how much of the energy of the original ray will be shared with reflected and refracted rays.

### 2.1.d. Conductor Material Support

The other feature added in this version is `Conductor Material`. This type of material is used to simulate metal objects. Altough these objects will behave similar to mirrors, they will not completely reflect the incoming rays and some amount of energy from the ray will be absorbed. To simulate these kind of objects we use `Fresnel Reflectance` to compute how much of the energy will be absorbed. With the help of these computations we can simulate metal objects.

### 2.1.e. PLY parser

In Basic Ray Tracer we were not rendering very complex scenes but in the new version we will be doing that with the help of `BVH`. However, complex scenes include millions of triangles. Format of these triangles in an `XML` file contains unnecessary characters which make the file size too big and parsing too slow. Therefore, the current version now supports `PLY` files. `PLY` files stores vertex and index information as binary which is compact in size and it makes parsing process faster than parsing from an `XML`.

### 2.1.f Multithreading

In Ray Tracing, we compute the color of each pixel indepently from others. We can take advantage of this property by introducing `multithreading`. I used eight threads in my Ray Tracer and I observed almost linear decrease on rendering time.

### 2.2. Resulting Images

Here are some images rendered with the current version of the Ray Tracer. Since in this version `BVH` is added, time comprasions of with and without `BVH` is also provided.

![Sc](/assets/hw2-spheres.jpg)

> Figure 2.2.1: Sphere Scene. No BVH: 0m0.640s BVH: 0m0.270s

![Sc](/assets/hw2-cornell.jpg)

> Figure 2.2.2: Recursive cornellbox. No BVH: 0m1.392s BVH: 0m0.629s

![Sc](/assets/hw2-science.jpg)

> Figure 2.2.3: Science Tree. No BVH: 6m28.715s BVH: 0m4.562s

![Sc](/assets/hw2-chinese.jpg)

> Figure 2.2.4: Chinese Dragon. No BVH: 23m53.611s BVH: 0m2.756s

![Sc](/assets/hw2-golden.jpg)

> Figure 2.2.5: Other Dragon. No BVH: Unknown BVH: 0m6.136s

No BVH time for Other Dragon is unknown because it takes too much time to render. I cancelled the rendering after 45 minutes or so.

### 2.3. Bugs and fixes

First issue was about `BVH`. To partition the objects inside a `bounding box`, we divide the box into two boxes from the middle of one of its axis. This middle point is called `pivot`. Then we check whether the centers of objects inside the original box are on the left of the pivot or right. Objects on the left and right will have their own bounding boxes. During this process, in some cases, all of the objects will be on the left or right of the pivot (see Figure 2.3.1).

![Sc](/assets/hw2-BVH-bug.jpg)

> Figure 2.3.1: Center of objects are at the same side for both x and y axis division.

In such a case bounding box will never change and these objects will always be on the same side of the pivot. To avoid this, I changed the way that I picked the pivot. Instead of picking the middle point of the bounding box, I pick the median of centers of objects. Since I compare object centers with the median, there will always be objects on both left and right side of the pivot. Therefore, at any case I will be able to divide the objects to construct the proper `BVH`.

The second issue was about refraction. For glass-like objects we compute the refracted and reflected ray colors. In the refracted ray computation I was adding the light contribution at the exit point of ray from the object. It was wrong to do such a computation because the interior of the object will not directly get illuminated by the light sources. Since glass-like objects refract incoming rays, those light rays coming from the light source will also get refracted. To do such a computation we need more complex calculations to be done. At this point, we ignore such complex calculations. This mistakenly added lighting contribution at the exit point of ray gave us the below result. It is little bit brighter than it supposed to be.

![Sc](/assets/hw2-refraction-bug.jpg)

> Figure 2.3.2: Wrong light contribution in glass-like objects.

Finally, the last issue was about shadow rays and wrong shadowing. I was not aware of this bug in the last version since all of the scenes tested on that version were simpler than the ones I test with now. For example Chinese Dragon scene has 1.8 Million triangles. As expected, at some points on the dragon, these triangles are so dense that some calculations failed. In the Basic Ray Tracer, my shadow rays were moved with `shadowRayEpsilon` value (which is a very small value to avoid self intersection) towards the light source. Since the triangles are too dense in the Dragon scene, this small movement caused my rays to have origins still inside the dragon mesh. This error resulted in the below figure.

![Sc](/assets/hw2-shadow-bug.jpg)

> Figure 2.3.3: Incorrect shadowing on Chinese Dragon.

I solved this issue by moving my rays along the surface normal, instead of moving them towards the light source.

### 2.4. Conclusion

I really liked the idea behind the `BVH`. I admire the people who came with this solution. When I got my first correct result with `BVH` included I was really happy. I wanted to see the actual bounding boxes to see I do everything correctly. Doing this also helped me to visualize it. Below, you can observe bounding box of two spheres at left (gray and cyan).

![Sc](/assets/hw2-spheres.jpg)

![Sc](/assets/hw2-bounding-boxes.jpg)

> Figure 2.4.1: Visualization of bounding boxes.

I was really surprised to see my Ray Tracer render very complex scenes in just few seconds. First time I see that, I thought something went wrong and the Ray Tracer failed to render the scene. Fortunately, everything was okay.

Now, since we simulate more types of materials, our scenes look more real and exciting. I look forward to add more features and make our Ray Tracer more advanced.
