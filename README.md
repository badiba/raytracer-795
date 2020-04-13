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

## 3. Transformations, Instancing, Multisampling and Distributed Ray tracing- Week 5 & 6

We parse objects and their vertices from an XML or a PLY file as explained in the earlier version of the Raytracer. These vertices are "stationary" in these files. In order to move, rotate or scale our objects, I added Transformations in this version.

Instancing is added to create more than one object from a single model defined in an XML or a PLY file. With this feauture, we can avoid copying the exact same vertex and index data for a model and multiple instances of it.

Multisampling -which is my favorite- is added to increase the quality of our images especially at the edges.

Distribution in ray tracing is used to add various visual effects such as motion blur, depth of field, glossy reflections.

### 3.1. Added features

### 3.1.a. Transformations

First of all there are three types of transformations: Translation, rotation and scaling. These transformation types are represented as a matrix. We can combine transformations by multiplying them and obtain a single composite transformation matrix. This composite matrix can be used to transform a point or a vector.

If we want to move our objects in our scene, one solution would be to change all of these vertices in the file that we are using. Imagine that we have millions of vertices for an object, this method would not be effective obviously.

Other solution could be to apply these transformations to all of the vertices of the object after the parsing is done. This would be a better solution than changing the file whenever we have new transformations. But this solution is not preferred for two reasons. First, it would not be effective to apply transformations for each vertex (there is a better solution). Second, if the object that we are transforming has an instance, then we will lose the base object data since all of its vertices are transformed now.

The method I applied is transforming the rays rather than the objects. When the inverse matrix of object transformations is applied to a ray (both to it's origin and direction), we obtain a new ray which is in local space of the object. This ray now can be intersected with the object to get the 't' value. This 't' value can be used to get the world space point of the intersection by putting it into the original ray equation.

There are a couple of things to mind in the last method. Homogenous coordinates are used in both the points and transformations (see [Homogenous coordinates](https://en.wikipedia.org/wiki/Homogeneous_coordinates)). To transform the points, the last component of the matrix should be 1 and for vectors, the last component should be zero. To transform the normals, first the normal at the local space of the objects is computed. Then the inverse transpose matrix is applied to get the normal at the world space. 

BVH implementation is also changed. Since we are going to transform our rays with inverse matrix of object transformations, we will have different rays for different objects. Therefore, we should have seperate BVH structures for every object rather than having a single BVH for the entire scene.

It seems like too much things to do, but many of these matrices can be pre-computed during the parsing which is very helpful for performance issues.

### 3.1.b. Instancing

Instancing can easily be added since all of the hard work is done in the transformations part and it is done by considering the instancing issues too. All we need to do is to hold the base object index. Then we can use this base object to find the intersection point. That point belongs to the instance and not the base object. Notice that an instance can have it's own transformation and this transformation is handled in a similar fashion with the base object. Only difference is that we should determine whether the transformations should be applied on top of the base object transformations or not.

### 3.1.c. Multisampling

In ray tracing we compute the color for every pixel and each pixel occupies an area in the screen. In earlier versions we were only computing one sample per pixel. As a result of this there were sudden changes in our images which decreases the quality of them. We can take multiple samples for a pixel to solve this issue. One solution is to equally distribute the samples over the pixel area. This is just like increasing the resolution of the image and it's not very effective. This method is called Regular Sampling. Another solution is to first divide the pixel into equal sub-areas and then randomly choose a sample for every one of this area. This method is called Jittered (Quasi Random) Sampling.

After choosing our samples, we compute the color for every one of them and then simply take the average. This average value will be the color of our pixel. With multisampling, quality of our images (especially at the edges of objects) are greatly improved.

### 3.1.d. Motion blur

Motion blur effect is added to simulate moving (dynamic) objects. It's representation is similar to transformations. In order to simulate this effect we need to add time value to our rays. This time will be a random value between 0 and 1. A ray with time value of 0 will intersect this object at the beginning of it's motion and a ray with value of 1 will intersect this object at the end of it's motion.

### 3.1.e. Depth of field

Real cameras have focal distance and aperture size. Due to this, real cameras focus to a distance and the objects at that distance are clearly visible. Objects which are further away or closer to camera are blurry. To simulate this effect, we take a random point on the camera in the interval of it's aperture size. By using this point and the sample point produced with multisampling we can compute a new direction for our rays. This rays will now be able to simulate this effect.

### 3.1.f. Glossy reflections

In earlier versions, we assumed metal or mirror objects to have perfect surfaces. With the help of distributed ray tracing, we will now be able to simulate rough surfaces. Rather than computing the perfect reflection ray on a metal or mirror object, we find a random direction for that ray. With the help of this random ray, rough surfaces can be simulated.

### 3.2. Resulting Images

Here are some images created with the current version of the ray tracer. Sample count is also sahred since it is an important factor for the render time.

![Sc](/assets/hw3-simple.jpg)

> Figure 3.2.1: Simple Transform. Render time: 0m0,181s Sample count: 1

![Sc](/assets/hw3-spheres.jpg)

> Figure 3.2.2: Spheres DOF. Render time: 0m15,500s Sample count: 100

![Sc](/assets/hw3-cornell_brushed.jpg)

> Figure 3.2.3: Cornellbox Brushed. Render time: 1m28,701s Sample count: 400

![Sc](/assets/hw3-cornell_dynamic.jpg)

> Figure 3.2.4: Cornellbox Dynamic. Render time: 2m53,237s Sample count: 900

![Sc](/assets/hw3-metal.jpg)

> Figure 3.2.5: Metal Glass Plates. Render time: 0m57,103s Sample count: 36

![Sc](/assets/hw3-dragon.jpg)

> Figure 3.2.6: Dragon Dynamic. Render time: 2m20,411s Sample count: 100

![Sc](/assets/hw3-tap.jpg)

> Figure 3.2.7: Tap. Render time: 21m47,896s Sample count: 100

### 3.3. Bugs and fixes

The first problem was about the glm library that I added to use it's transformation functionalities. Glm applies transformations in reverse order. In other words, the last transformation applied to your 4x4 transformation matrix should be the first transformation that is going to be applied to your object. Not only this but also the fact that glm chooses to print their matrices in column major order made me lose a few hours trying to figure out what was wrong. After all, I still appreciate their work. Resulting image is shared below.

![Sc](/assets/hw3-bug-1.jpg)

The second problem was about transforming the rays from world space to object local space. In order to transform rays, we need to transform it's two components, namely it's origin and direction. My problem was about the direction. As explained above we use homogenous coordinates to make transformations calculations. In homogenous coordinates, a three dimensional vector has 4 components. The last component of a vector should be set to zero since vectors cannot be translated but they can be rotated and scaled. Setting the last component of a vector to 1 changes it's behvaiour and the results become wrong as shown below. I found this solution at online edition of PBRT (see [PBRT](http://www.pbr-book.org/))

![Sc](/assets/hw3-bug-2.jpg)

The third problem was about the wrong calculation of normals. After finding the normal at object local space, in order to transform it to world space we multiply the normal with inverse transpose of object transformations. While doing this, order of matrix multiplication was wrong resulting in the below image (You never know what you are going to see when the rendering is over).

![Sc](/assets/hw3-bug-3.jpg)

The most important issue was about the performance. And this issue kept appearing at different steps of this version of the Ray tracer mainly because of the transformation calculations and dielectric material. There was a significant increase in rendering time whenever I added dielectric material to an object. At first I thought this was normal since dielectric material computations are much more complex than other materials. But after trying to render dragon dynamic scene and seeing that it takes so much time (around 122 minutes), I thought I should do something. I first wanted to check and hopefully fix dielectric material computations but there was nothing wrong with it. Then I profiled every step of the program and found that in some rare cases my BVH intersection function was trying to intersect the ray with all of the triangles inside the BVH structure. When I found this, I was sure that I was on the right path. First I thought that my box intersection method was wrong and checked it. Again there was nothing wrong with it. Then I wanted to see what kind of rays caused this bug. And I saw the reason behind this bug. The direction of the ray was just three NaNs. Then I understood that whenever total internal reflection happened inside a dielectric material this bug occured since the square root was negative for not-reflected ray. I added this case into my dielectric material computations and it was fixed. After this improvement dragon dynamic took around 54 minutes.

Other performance issue was about the transformation calculations. At the first implementation, I was precomputing the composite transformation matrix of the object and taking it's inverse and inverse transpose while rendering. Then while trying to increase the performance of the program I also added the precomputation of inverse and inverse transpose matrices which increased the performance significantly. After this dragon dynamic scene took around 29 minutes.

The last performance issue was about the multithreading implementation. At first, I was dividing the image into eight pieces in terms of pixel count. Thinking about this, I realized that some hard-to-compute pixels will most likely be in the same region. Thus it caused one of my threads to keep computing those pixels while others are done with their jobs. I wanted to fix this and instead of dividing the whole image into eight big pieces I take the modulus of the pixel coordinates sum (x coordinate + y coordinate) and give that pixel to the thread with id of result of the modulus operation. For example, if you have eight threads, the pixel (0,0) goes to thread0, pixel(0,1) goes to thread1, pixel(55,23) goes to thread6. This again increased the performance. After this dragon dynamic scene took around 21 minutes.

### 3.4. Conclusion

I always wanted our objects to look smoother. With multisampling I finally achieved this. I also added multisampling to the images produced with the earlier versions of the ray tracer (I couldn't stop myself). Other visual effects also increased the quality. I was thinking to add a skybox but I need some free time to do that. I will hopefully do it this summer.

Another passion of mine with this project is to increase the performance. This was not the case before since the most complex scenes were being rendered in just a few seconds. However, now we have multisampling and it increases the render time significantly. Due to this, finding a way to decrease the render time is crucial and it is very fullfilling when I am successful.

Yet another passion of mine is to play with the images. Here is the best (in my opinion) image I could get.

![Sc](/assets/hw3-conclusion.jpg)
