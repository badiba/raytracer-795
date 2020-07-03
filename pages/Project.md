## Project

My project assignment was to convert [spaceship](https://benedikt-bitterli.me/resources/images/spaceship.png) into our format that we use in CENG795 Ray Tracing class. Spaceship scene has three different formats; pbrtv3, Tungsten, Mitsuba. I used all of them for different purposes to get the final result.

There are couple of things that I would like to mention before I start to explain how did I convert the scene. First of all some materials were hard to convert since at some points, PBRT implementation was different than ours. For example, PBRT uses "uroughness" and "vroughness" whereas we use "roughness". I did my best to get the most similar looking materials by rendering multiple times for each of those. Secondly, by the time I was working on this project, I did not have finished path tracing and object lights assignment. Therefore, I couldn't use these features in this scene.

### 1. Camera Alignment and Defining Objects

To align the camera correctly, I used the pbrtv3 format of the scene. There, you can find the world-to-camera transformation. I used this transformation to find the world position and orientation of the camera. Objects in the scene are all meshes. Therefore I created a mesh object for every ply file that we have. In order to test whether everything is okay up to this point, I made two simple materials. One for the ground which is a white diffuse material and the other for the spaceship which is a red diffuse material.

![Sc](/../assets/project/p-1.jpg)

> Figure 1: Checking whether camera and objects are in correct place.

### 2. Identifying Objects

At this point, we have just bunch of mesh objects and a camera in our scene. We verified that they are at correct position but we do not know which object in our scene specifically correspond to an object in the original scene. For example, we do not know which one of our objects correspond to the glass in the original scene. So, we have to identify them. In order to do that, I created simple materials like red diffuse, blue diffuse, green diffuse and mixture of these. Idea is to assign these materials to objects and render the scene to understand which shape do they have. After every rendering, I put a comment in the XML file to distinguish the objects.

![Sc](/../assets/project/p-2.jpg)

> Figure 2: Coloring objects for identification.

### 3. Light Sources

In order to produce above basic results, I used a simple point light. But point light was not looking good enough to keep it for the final result. So, I decided to work on lightning of the scene. Since PBRT used an area light, I wanted to go for it too. In order to find the correct position of the light source, I wrote a program to convert my transform commands into world position. I used that program to find the correct positioning of the light source. However, introducing area light source was expensive because I also had to put multisampling on. So I was switching back and forth between point light and area light to either render fast or check my scene under correct lightning.

Area light was not the only challange in the scene. I also had to illuminate the interior of the glass. This was the most difficult issue in my project. Since I didn't have object light and path tracing, I had to find my way with either an area light or a point light (I also tried environment light but render time was increased too much due to high sampling amount). In order to find the correct position of the light source, I used a sphere object because I couldn't see where my light sources are since they are not intersectable. I positioned the sphere object right under the light source to see where it is.

![Sc](/../assets/project/p-3.jpg)

> Figure 3: Sphere object used to visualize the position of light sources.

### 4. Materials

Finding the correct materials was the second most difficult issue in my project. I had many google searches to find the correct "AbsorptionIndex" and "RefractionIndex" of conductors. When I found a good looking materials, I also tried them with area light to see their true beauty. Since area light renders were taking longer time, I was able to study for my finals next to my computer.

For the dielectric material of the glass, I was not able to get the two reflections of light sources. I thought this was due to some sort of "thickness" in the materials defined in Tungsten, however, it might be another area light source. I wanted to try this but rendering time was increased too much. So, I decided it would be better to have just one area light.

![Sc](/../assets/project/p-4.jpg)

> Figure 4: Two reflections on glass in the original image.

## 5. Final Result

Here is my final result.

![Sc](/../assets/project/p-5.jpg)

> Figure 5: Final result.