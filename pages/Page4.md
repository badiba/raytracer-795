# Introduction

This page includes technical information and personal experiences on the homeworks of CENG795 Advanced Ray Tracing class. It is under development and updated frequently. Owner of this page is Bahadir Coskun.

# Content

This section includes different versions of the Ray Tracer. In every new version, new features are added. Each version has it's own subsection. These subsections include;

- brief explanations of what is being added in that version, 
- resulting images in that version,
- what kind of bugs have been encountered and how did I manage to fix them, 
- my personal experiences. 

If applicable, time comparisons of different implementations are also added in these subsections.

## 4. Texture, Normal and Bump Mapping

Up until now, we were assigning only materials to the objects. Types of these materials are discussed in the first and second chapter of this blog. Basically these materials (diffuse, dielectric etc.) contain some color values and with the correct computation we were able to produce "flat-looking" objects. However, objects in real life don't have just one color for their entire surface.

To overcome this issue, what we do is, obtain some image file which will contain color values for the surface of the objects. This image file is simply called a texture. Once we find an intersection point on the surface of an object, we can do several things with a texture. We can use the color obtained from the texture for shading, or we can use it to change our normal at the intersection point. Both of these methods includes some details which will be discussed later.

Instead of using an image file to make texture computations, we can use Perlin Noise calculations in real-time. This method is used to create some kind of effects in our scene such as fire, clouds etc.

### 4.1. Added features

The discussions below are based on image textures but they can also be applied with the Perlin texture. This is also stated in the 4.1.e section.

### 4.1.a. Texture Mapping

In texture mapping, we have several options. We can directly assign the color obtained from the image file to the surface. This method completely disables shading computations. Therefore, the result is not realistic. Second option is to use the color from the image file to replace the diffuse reflectance coefficient. This method is actually much better than the former one in terms of realism since we will be making shading computations. Third option is to use the value from the image file and the diffuse reflectance coefficient value together. This method is sometimes preferred. It gives a feeling of a painting over an already colored surface.

### 4.1.b. Background Image

Until now, we were assigning a single color value to our background. Since we are now enabling texture mapping and using image files, we can now just make simple computations to have an image at the background. By doing so, our scene now has more depth in it.

### 4.1.c. Normal Mapping

After getting the color value from an image file, we are not limited to use it to determine the color of our surface. We can also use it to change our normal at the intersection point. By doing so, we can obtain hollows, heightness and roughness on a surface. For example, if we want to have the Earth in our scene, using a sphere object would make sense. Then we should do texture mapping which means we are going to need an image file. But this is not enough since we will put the colors from the image file directly on the surface of a sphere. Which means the mountains on the Earth will not be on our sphere object. It will look like a painting on a perfectly smooth sphere. To avoid this and improve the realism we need to change the normal of the surface whenever we have an intersection. Changing the normal will trick the eye that there is some height difference on the surface although it is still perfectly smooth. The reason behind this is that normals are used to make shading computations and changing the normal will change what you are seeing. Details of this reason are intentionally avoided here. For more information see [Shading](https://en.wikipedia.org/wiki/Shading).

![Sc](/assets/hw4-normal-mapping.jpg)

> Figure 4.1.1: Normal mapping example

### 4.1.d. Bump Mapping

Bump mapping is used to change the normal just like in the normal mapping. The difference is that in the normal mapping we are simply replacing the normal with the value obtained from the image file. In bump mapping, however, we are adjusting the normal in a way that it represents displacements on the surface more accurately. This means that we are trying to find the normal if we had such complex surface although we don't. First of all just like in the normal mapping we have to find an intersection point on the surface and the corresponding value from the image file. Value from the image file gives us the amount of change at the intersection point of the surface. This change is used to find the new normal at that intersection point.

![Sc](/assets/hw4-bump-mapping.jpg)

> Figure 4.1.2: Bump mapping example

### 4.1.e. Perlin Noise

All of the above discussion can be made with using an image file as a texture but that is not the only choice. We can also use Perlin Noise computations to change the diffuse reflectance coefficient value or find the new perturbed normal. Space in our scene has perlin values at every point. This values are obtained with the computations suggested by Perlin (see [Perlin Noise](https://en.wikipedia.org/wiki/Perlin_noise)). Once we find the perlin value at an intersection point, we can now use it as diffuse reflectance coefficient just like in image texture case. Moreover, we can find the change of perlin noise at the intersection point to find the perturbed normal for bump mapping computations.

### 4.2. Resulting Images

![Sc](/assets/hw4-bump_mapping_transformed.jpg)

> Figure 4.2.1: bump_mapping_transformed scene. Render time: 0m0,807s Sample Count: 1

![Sc](/assets/hw4-cube_cushion.jpg)

> Figure 4.2.2: cube_cushion scene. Render time: 0m0,346s Sample Count: 1

![Sc](/assets/hw4-cube_perlin.jpg)

> Figure 4.2.3: cube_perlin scene. Render time: 0m0,950s Sample Count: 1

![Sc](/assets/hw4-cube_perlin_bump.jpg)

> Figure 4.2.4: cube_perlin_bump scene. Render time: 0m3,030s Sample Count: 1

![Sc](/assets/hw4-cube_wall.jpg)

> Figure 4.2.5: cube_wall scene. Render time: 0m0,324s Sample Count: 1

![Sc](/assets/hw4-cube_wall_normal.jpg)

> Figure 4.2.6: cube_wall_normal scene. Render time: 0m0,365s Sample Count: 1

![Sc](/assets/hw4-cube_waves.jpg)

> Figure 4.2.7: cube_waves scene. Render time: 0m0,336s Sample Count: 1

![Sc](/assets/hw4-ellipsoids_texture.jpg)

> Figure 4.2.8: ellipsoids_texture scene. Render time: 0m0,346s Sample Count: 1

![Sc](/assets/hw4-galactica_dynamic.jpg)

> Figure 4.2.9: galactica_dynamic scene. Render time: 1m42,265s Sample Count: 100

![Sc](/assets/hw4-galactica_static.jpg)

> Figure 4.2.10: galactica_static scene. Render time: 0m1,868s Sample Count: 1

![Sc](/assets/hw4-killeroo_bump_walls.jpg)

> Figure 4.2.11: killeroo_bump_walls scene. Render time: 0m16,250s Sample Count: 16

![Sc](/assets/hw4-sphere_nearest_bilinear.jpg)

> Figure 4.2.12: sphere_nearest_bilinear scene. Render time: 0m0,312s Sample Count: 1

![Sc](/assets/hw4-sphere_nobump_bump.jpg)

> Figure 4.2.13: sphere_nobump_bump scene. Render time: 0m6,994s Sample Count: 1

![Sc](/assets/hw4-sphere_nobump_justbump.jpg)

> Figure 4.2.14: sphere_nobump_justbump scene. Render time: 0m0,240s Sample Count: 1

![Sc](/assets/hw4-sphere_normal.jpg)

> Figure 4.2.15: sphere_normal scene. Render time: 0m11,001s Sample Count: 100

![Sc](/assets/hw4-sphere_perlin.jpg)

> Figure 4.2.16: sphere_perlin scene. Render time: 0m0,764s Sample Count: 1

![Sc](/assets/hw4-sphere_perlin_bump.jpg)

> Figure 4.2.17: sphere_perlin_bump scene. Render time: 0m3,746s Sample Count: 1

![Sc](/assets/hw4-sphere_perlin_scale.jpg)

> Figure 4.2.18: sphere_perlin_scale scene. Render time: 0m1,250s Sample Count: 1

### 4.3. Bugs and fixes

First problem I encountered was about the bump mapping. To find the perturbed normal, first we need to compute the tangent and bitangent vectors at the intersection point. I thought I should normalize these two vectors since I thought the normal, tangent and bitangent coordinate system should be orthonormal. However, when I normalize these vectors I lose the scaling information and different surfaces will end up having the same size of vectors giving me the wrong result. For example bigger spheres should have larger size of tangent and bitangent vectors. Here is an example of the wrong implementation.

![Sc](/assets/hw4-bug-1.jpg)

> Figure 4.3.1: Wrong implementation of bump mapping

Another issue was about Perlin weight function computation. I was just computing the weight with the components of the vectors without taking their absolute value. However, this is incorrect since we are trying to find the weighted average of vector contributions to the point in space. Therefore I have to take the absolute value of the components of vectors. Wrong result can be observed below.

![Sc](/assets/hw4-bug-2.jpg)

> Figure 4.3.2: Wrong implementation of Perlin weight function

The last issue was not about textures. I had a bug which stayed alive through the earlier versions of the ray tracer. It was inside the gett(point) function of the ray class. Gett(point) function simply returns the t value of the ray at given point. I was just making the computations with respect to the x component of the direction of ray. Whenever the x component is zero, a division becomes a zero division and function returns NaN. Now I just check this case and the issue is gone. Problem can be observed here.

![Sc](/assets/hw4-bug-3.jpg)

> Figure 4.3.3: Bug at gett(point) function of Ray class

### 4.4. Conclusion

In the earlier versions, we had important advancements. I especially liked the last version where I implemented the multisampling and distributed ray tracing. But I have to say that I liked this version as much as the last one. Putting the textures on our objects brought life to the scene. I really liked how different it feels after implementing this version. Moreover, as a space enthusiast I found out that Nasa has some pretty cool images for texture and bump mapping of celestial bodies such as the Moon, Mars, etc. I immediately downloaded them and used to get the below result.

![Sc](/assets/hw4-conclusion.jpg)

> Figure 4.4.1: Moon

After getting the result of the Moon scene, I can say that we need directional light to better simulate the Moon object. At this point I used point light. I positioned the light at a very long distance from the obejct and I increased its intensity. Obviously I also set the values of ambient light to zero. However, sunlight would be better simulated with a directional light. I am looking forward to add different types of light sources including directional light in our next version of the ray tracer.
