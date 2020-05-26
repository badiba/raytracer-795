## 3. Transformations, Instancing, Multisampling and Distributed Ray tracing

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

Here are some images created with the current version of the ray tracer. Sample count is also shared since it is an important factor for the render time.

![Sc](/../assets/hw3-simple.jpg)

> Figure 3.2.1: Simple Transform. Render time: 0m0,181s Sample count: 1

![Sc](/../assets/hw3-spheres.jpg)

> Figure 3.2.2: Spheres DOF. Render time: 0m15,500s Sample count: 100

![Sc](/../assets/hw3-cornell_brushed.jpg)

> Figure 3.2.3: Cornellbox Brushed. Render time: 1m28,701s Sample count: 400

![Sc](/../assets/hw3-cornell_dynamic.jpg)

> Figure 3.2.4: Cornellbox Dynamic. Render time: 2m53,237s Sample count: 900

![Sc](/../assets/hw3-metal.jpg)

> Figure 3.2.5: Metal Glass Plates. Render time: 0m57,103s Sample count: 36

![Sc](/../assets/hw3-dragon.jpg)

> Figure 3.2.6: Dragon Dynamic. Render time: 21m47,896s Sample count: 100

![Sc](/../assets/hw3-tap.jpg)

> Figure 3.2.7: Tap. Render time: 2m20,411s Sample count: 100

### 3.3. Bugs and fixes

The first problem was about the glm library that I added to use it's transformation functionalities. Glm applies transformations in reverse order. In other words, the last transformation applied to your 4x4 transformation matrix should be the first transformation that is going to be applied to your object. Not only this but also the fact that glm chooses to print their matrices in column major order made me lose a few hours trying to figure out what was wrong. After all, I still appreciate their work. Resulting image is shared below.

![Sc](/../assets/hw3-bug-1.jpg)

The second problem was about transforming the rays from world space to object local space. In order to transform rays, we need to transform it's two components, namely it's origin and direction. My problem was about the direction. As explained above we use homogenous coordinates to make transformations calculations. In homogenous coordinates, a three dimensional vector has 4 components. The last component of a vector should be set to zero since vectors cannot be translated but they can be rotated and scaled. Setting the last component of a vector to 1 changes it's behvaiour and the results become wrong as shown below. I found this solution at online edition of PBRT (see [PBRT](http://www.pbr-book.org/))

![Sc](/../assets/hw3-bug-2.jpg)

The third problem was about the wrong calculation of normals. After finding the normal at object local space, in order to transform it to world space we multiply the normal with inverse transpose of object transformations. While doing this, order of matrix multiplication was wrong resulting in the below image (You never know what you are going to see when the rendering is over).

![Sc](/../assets/hw3-bug-3.jpg)

The most important issue was about the performance. And this issue kept appearing at different steps of this version of the Ray tracer mainly because of the transformation calculations and dielectric material. There was a significant increase in rendering time whenever I added dielectric material to an object. At first I thought this was normal since dielectric material computations are much more complex than other materials. But after trying to render dragon dynamic scene and seeing that it takes so much time (around 122 minutes), I thought I should do something. I first wanted to check and hopefully fix dielectric material computations but there was nothing wrong with it. Then I profiled every step of the program and found that in some rare cases my BVH intersection function was trying to intersect the ray with all of the triangles inside the BVH structure. When I found this, I was sure that I was on the right path. First I thought that my box intersection method was wrong and checked it. Again there was nothing wrong with it. Then I wanted to see what kind of rays caused this bug. And I saw the reason behind this bug. The direction of the ray was just three NaNs. Then I understood that whenever total internal reflection happened inside a dielectric material this bug occured since the square root was negative for not-reflected ray. I added this case into my dielectric material computations and it was fixed. After this improvement dragon dynamic took around 54 minutes.

Other performance issue was about the transformation calculations. At the first implementation, I was precomputing the composite transformation matrix of the object and taking it's inverse and inverse transpose while rendering. Then while trying to increase the performance of the program I also added the precomputation of inverse and inverse transpose matrices which increased the performance significantly. After this dragon dynamic scene took around 29 minutes.

The last performance issue was about the multithreading implementation. At first, I was dividing the image into eight pieces in terms of pixel count. Thinking about this, I realized that some hard-to-compute pixels will most likely be in the same region. Thus it caused one of my threads to keep computing those pixels while others are done with their jobs. I wanted to fix this and instead of dividing the whole image into eight big pieces I take the modulus of the pixel coordinates sum (x coordinate + y coordinate) and give that pixel to the thread with id of result of the modulus operation. For example, if you have eight threads, the pixel (0,0) goes to thread0, pixel(0,1) goes to thread1, pixel(55,23) goes to thread6. This again increased the performance. After this dragon dynamic scene took around 21 minutes.

### 3.4. Conclusion

I always wanted our objects to look smoother. With multisampling I finally achieved this. I also added multisampling to the images produced with the earlier versions of the ray tracer (I couldn't stop myself). Other visual effects also increased the quality. I was thinking to add a skybox but I need some free time to do that. I will hopefully do it this summer.

Another passion of mine with this project is to increase the performance. This was not the case before since the most complex scenes were being rendered in just a few seconds. However, now we have multisampling and it increases the render time significantly. Due to this, finding a way to decrease the render time is crucial and it is very fullfilling when I am successful.

Yet another passion of mine is to play with the images. I wanted to make a stray comet. I used two spheres to make it. One for the head, other for the tail. Tail has an absurd motion blur though. I hope you like it.

![Sc](/../assets/hw3-conclusion.jpg)


