## 7. Path Tracing

In this version, object lights and path tracing are added to the ray tracer.

### 7.1. Added Features

### 7.1.a Object Lights

Up until now, our light sources were invisible. They did not have a shape therefore it was impossible for our rays to intersect with them. However, with object lights being introduced, just like other objects, our rays will intersect with them and we will be able to see them in our scenes.

![Sc](/../assets/hw7/hw7-2-2.jpg)

> Figure 7.1.1: Ceiling is an object light. Its shape is mesh.

### 7.1.b Path Tracing

Our rendering equation consists of two parts. One of them is emitted radiance and the other is integration of incoming radiance multiplied with BRDF and cosine of angle between wi and normal at the intersection point with given wi and wo directions. Below image is taken from [Wikipedia](https://en.wikipedia.org/wiki/Rendering_equation).

![Sc](/../assets/hw7/hw7-1-2.jpg)

> Figure 7.1.2: Rendering equation of ray tracing.

Without path tracing, at every intersection, we were simply iterating through all light sources to find their contribution at the intersection point and sum them to find the outgoing radiance. Instead of integrting the full hemisphere, we were doing this simple calculation and it is called direct lightning. Path tracing aims to find the real value of the integration in the rendering equation. However, we will not directly find the exact value of it. Instead, we will be using Monte Carlo integration which will estimate it.

In order to do Monte Carlo integration, we need to take sample directions on the hemisphere. As we increase the number of samples, we get closer to the real value of the integration, since we capture the value of more and more directions on the hemisphere. There are many ways to take samples. In our path tracer, we added two ways of sampling, uniform sampling and importance sampling.

In order to learn more about path tracing, sampling and Monte Carlo integration, visit the [Youtube channel of Ahmet Oguz Akyuz](https://www.youtube.com/channel/UC0HevdJTfHUk3LI_fBtYEVA). That's where I learned all of those topics.

### 7.2. Resulting Images

There are couple of things that I would like to mention for the images below. In the implementation of Russian Roulette as stopping condition for Path Tracing, I chose to go for cosine of angle between sample direction and normal vector to decide termination probability. The reason behind this, I wanted to see the difference between this method and throughput method. Since provided images in the homework are produced with throughput method, I could compare my results with them. I observed that Russian Roulette with cosine method produces darker results since it doesn't care about throughput and some of the sample directions may have important contribution but terminated due their angle. Another observation is that glass objects get much more affected from "early termination" situation since reflected and refracted ray directions are dependant to the viewing direction (i.e. we do not uniformly sample them).

One other observation is that whenever Russian Roulette is enabled, rendering time decreases due to "early termination" issue. This behaviour also makes sense when you combine it with darker results in Russian Roulette scenes. Also, notice that in terms of rendering, importance sampling combined with Russian Roulette takes more time than just Russian Roulette (Figure 7.2.13 and Figure 7.2.14). This also makes sense when you consider that importance sampling favors the directions that are closer to normal direction, thus makes them harder to terminate with Russian Roulette cosine termination method.

Secondly, I couldn't find time to fix my tonemapper before the deadline of this assignment due to final exams. I had few days left for this homework when my last final exam was over. So, I spent all my time on implementing it and couldn't fix tonemapper on time. Therefore, I tonemapped exr files using external tools which means there might be minor differences between the results below and their correctly tonemapped versions.

Finally, for veach-ajar, I couldn't find time to render it with 6400 samples. Intead, below result is produced with 100 samples. One other thing about this scene is that I forgot to add degamma property in textures. Therefore the door and the painting in the scene looks white in my submission.

![Sc](/../assets/hw7/hw7-2-1.jpg)

> Figure 7.2.1: cornellbox_jaroslav_diffuse scene. Render time: 0m6,704s Sample count: 100

![Sc](/../assets/hw7/hw7-2-2.jpg)

> Figure 7.2.2: cornellbox_jaroslav_diffuse_area scene. Render time: 0m7,047s Sample count: 100

![Sc](/../assets/hw7/hw7-2-3.jpg)

> Figure 7.2.3: cornellbox_jaroslav_glossy scene. Render time: 0m7,200s Sample count: 100

![Sc](/../assets/hw7/hw7-2-4.jpg)

> Figure 7.2.4: cornellbox_jaroslav_glossy_area scene. Render time: 0m7,337s Sample count: 100

![Sc](/../assets/hw7/hw7-2-5.jpg)

> Figure 7.2.5: cornellbox_jaroslav_glossy_area_ellipsoid scene. Render time: 0m9,664s Sample count: 100

![Sc](/../assets/hw7/hw7-2-6.jpg)

> Figure 7.2.6: cornellbox_jaroslav_glossy_area_small scene. Render time: 0m12,704s Sample count: 100

![Sc](/../assets/hw7/hw7-2-7.jpg)

> Figure 7.2.7: cornellbox_jaroslav_glossy_area_sphere scene. Render time: 0m10,274s Sample count: 100

![Sc](/../assets/hw7/hw7-2-8.jpg)

> Figure 7.2.8: diffuse scene. Render time: 0m20,552s Sample count: 100

![Sc](/../assets/hw7/hw7-2-9.jpg)

> Figure 7.2.9: diffuse_importance scene. Render time: 0m20,131s Sample count: 100

![Sc](/../assets/hw7/hw7-2-10.jpg)

> Figure 7.2.10: diffuse_importance_russian scene. Render time: 0m12,758s Sample count: 100

![Sc](/../assets/hw7/hw7-2-11.jpg)

> Figure 7.2.11: diffuse_next scene. Render time: 1m8,125s Sample count: 100

![Sc](/../assets/hw7/hw7-2-12.jpg)

> Figure 7.2.12: diffuse_next_importance scene. Render time: 1m8,580s Sample count: 100

![Sc](/../assets/hw7/hw7-2-13.jpg)

> Figure 7.2.13: diffuse_next_importance_russian scene. Render time: 0m34,992s Sample count: 100

![Sc](/../assets/hw7/hw7-2-14.jpg)

> Figure 7.2.14: diffuse_next_russian scene. Render time: 0m22,331s Sample count: 100

![Sc](/../assets/hw7/hw7-2-15.jpg)

> Figure 7.2.15: diffuse_russian scene. Render time: 0m8,361s Sample count: 100

![Sc](/../assets/hw7/hw7-2-16.jpg)

> Figure 7.2.16: glass scene. Render time: 0m32,094s Sample count: 100

![Sc](/../assets/hw7/hw7-2-17.jpg)

> Figure 7.2.17: glass_importance scene. Render time: 0m34,378s Sample count: 100

![Sc](/../assets/hw7/hw7-2-18.jpg)

> Figure 7.2.18: glass_importance_russian scene. Render time: 1m21,664s Sample count: 100

![Sc](/../assets/hw7/hw7-2-19.jpg)

> Figure 7.2.19: glass_next scene. Render time: 1m42,631s Sample count: 100

![Sc](/../assets/hw7/hw7-2-20.jpg)

> Figure 7.2.20: glass_next_importance scene. Render time: 1m46,370s Sample count: 100

![Sc](/../assets/hw7/hw7-2-21.jpg)

> Figure 7.2.21: glass_next_importance_russian scene. Render time: 3m52,931s Sample count: 100

![Sc](/../assets/hw7/hw7-2-22.jpg)

> Figure 7.2.22: glass_next_russian scene. Render time: 0m29,765s Sample count: 100

![Sc](/../assets/hw7/hw7-2-23.jpg)

> Figure 7.2.23: glass_russian scene. Render time: 0m12,707s Sample count: 100

![Sc](/../assets/hw7/hw7-2-24.jpg)

> Figure 7.2.24: veach-ajar scene. Render time: 0m28,529s Sample count: 100

### 7.3. Bugs and Fixes

First one was about next event estimation. In next event estimation, we should be adding the contribution of light sources directly. I was iterating through all light sources to achieve this. I was also computing the emitted radiance at every point of intersection in path tracing. Combination of these two was problematic. If global illumination ray hits an object light, then I was calculating both emitted radiance and contribution of the same object light. To fix this, I check if global illumination ray hits an object light or not.

![Sc](/../assets/hw7/hw7-3-1.jpg)

> Figure 7.3.1: Double emitted radiance.

Second one was about mesh light shadow checking. For shadow checking, I cast a shadow ray and check the intersection point. If intersection happens on mesh light then I was saying that there is no shadow. However, if sampled point is on the backface of the mesh light, then even if shadow ray hits the mesh light, it should be shadowed. Not doing this introduced brighter scenes. To fix this, I now check distance of intersection point and sample point.

![Sc](/../assets/hw7/hw7-3-2.jpg)

> Figure 7.3.2: Incorrect shadow check with mesh light sources.

Third one was about next event estimation again. In next event estimation, there is one special case that we should handle correctly. When the sampled ray hits a mesh light object, if we take the contribution of both the mesh light source and sampled direction, then we will be favoring that direction. Which means we need to discard one of them. I was discarding the contribution of mesh light object which affected the behaviour of next event estimation and introduced noise. To fix it, I now discard contribution of sample direction, instead of mesh light object.

![Sc](/../assets/hw7/hw7-3-3.jpg)

> Figure 7.3.3: Noise due to eliminating contribution of light source instead of global illumination ray in next event estimation.

Fourth one was about dielectric materials. I was doing "regular" dielectric calculation and stopping reflected and refracted rays after they hit a diffuse object like in the direct lightning case. This mistake produced a black ball instead of a glass object. Though you can observe some pixels have color in that black ball. I think it's because some of reflected and refracted rays were able to hit the light source directly. Thus they illuminate the surface. To fix this, I treated reflected and refracted rays as global illumination rays.

![Sc](/../assets/hw7/hw7-3-4.jpg)

> Figure 7.3.4: Black glass due to wrong integration of dielectric material in path tracing.

Fifth and last one is not actually a bug. It's a result of one of my "What does this button do?" moments. I just wondered what would happen if we treated incoming light from path tracing as a point light source. I divided the radiance by square of the distance and got this sweet result.

![Sc](/../assets/hw7/hw7-3-5.jpg)

> Figure 7.3.5: Jelly beans.

### 7.4. Conclusion

Path tracing is just amazing. We can even see the light travel through the glass and illuminate the ground. I really enjoyed working on it. After path tracing is done, I couldn't even look at earlier images produced with direct lightning.

![Sc](/../assets/hw7/hw7-4-1.jpg)

> Figure 7.4.1: Light passing through dielectric object and illuminating ground.

With path tracing being added, we have come to the end of Advanced Ray Tracing class. But, my journey on learning Ray Tracing will continue. I was planning to do several things after the semester is done. These include; cleaning my code, working on GPU ray tracing, adding a skybox. I'm going to focus on cleaning my code now and then I will work on GPU ray tracing. I don't know if I can finish it on near future but even if I fail, I will be learning.