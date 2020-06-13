## 6. BRDF Models

BRDF (Bidirectional reflectance distribution function) is a function to compute how much of the light is being reflected towards the viewing direction on a surface. There are several BRDF models. We were only using Blinn-Phong model until now. In this version, we will be adding Phong and Torrance-Sparrow models along with modifications for each of the models we have.


### 6.1. Added Features


### 6.1.a. Phong

In Blinn-Phong we were computing half vector of light and viewing vectors. Then we were using the angle between half vector and normal. In the Phong model, however, we are going to compute reflected vector of light vector on the surface. Then we are going to use the angle between reflection vector and viewing vector in our computation. Below image compares Blinn-Phong and Phong models. Image is taken from [Wikipedia](https://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_reflection_model).

![Sc](/../assets/hw6/hw6-1-1.jpg)

> Figure 6.1.1: Blinn-Phong and Phong comparison.

### 6.1.b. Torrance-Sparrow

Torrance-Sparrow BRDF uses the Microfacet model. In Microfacet model, we simply state that the surface is not perfectly flat. Instead it has tiny Microfacets on it. These Microfacets reflect incoming light like a perfect mirror. In order to simulate this behaviour, we say that these Microfacets are oriented randomly around the surface normal. This model increases the realism of specular component of BRDF.

Below image shows three important effects of Microfacet model. Masking (a), Shadowing (b), Interreflection (c). Image is taken from [PBRT](http://www.pbr-book.org/3ed-2018/Reflection_Models/Microfacet_Models.html).

![Sc](/../assets/hw6/hw6-1-2.jpg)

> Figure 6.1.2: Geometry of Microfacet model.

### 6.1.c. Modification

In the implementation of BRDF function there is a `cos` term in the denominator which makes the BRDF computation not energy conserving. In order to solve this, we just simply remove the `cos` term. Since we modified the BRDF function, we call it modified BRDF. This modification allows us to normalize the BRDF.

### 6.1.d. Normalization

BRDF function will be energy conserving when we normalize it. However, normalizing the BRDF function is not enough. We also need to provide `kd` and `ks` values such that their summation should be less than or equal to one. If their summation is smaller than one, it means there is absorption on the surface. If the summation is equal to one, than it means all the incoming light is being reflected, there is no absorption.

### 6.2. Resulting Images

![Sc](/../assets/hw6/hw6-2-1.jpg)

> Figure 6.2.1: Blinn-Phong modified scene. Render time: 0m0,183s Sample count: 1

![Sc](/../assets/hw6/hw6-2-2.jpg)

> Figure 6.2.2: Blinn-Phong modified normalized scene. Render time: 0m0,158s Sample count: 1

![Sc](/../assets/hw6/hw6-2-3.jpg)

> Figure 6.2.3: Blinn-Phong original scene. Render time: 0m0,167s Sample count: 1

![Sc](/../assets/hw6/hw6-2-4.jpg)

> Figure 6.2.4: Phong modified scene. Render time: 0m0,160s Sample count: 1

![Sc](/../assets/hw6/hw6-2-5.jpg)

> Figure 6.2.5: Phong modified normalized scene. Render time: 0m0,162s Sample count: 1

![Sc](/../assets/hw6/hw6-2-6.jpg)

> Figure 6.2.6: Phong original scene. Render time: 0m0,166s Sample count: 1

![Sc](/../assets/hw6/hw6-2-7.jpg)

> Figure 6.2.7: Torrance-Sparrow scene. Render time: 0m0,160s Sample count: 1

![Sc](/../assets/hw6/hw6-2-8.jpg)

> Figure 6.2.8: Killeroo Blinn-Phong scene. Render time: 0m29,838s Sample count: 16

![Sc](/../assets/hw6/hw6-2-9.jpg)

> Figure 6.2.9: Killeroo Blinn-Phong close scene. Render time: 1m6,293s Sample count: 16

![Sc](/../assets/hw6/hw6-2-10.jpg)

> Figure 6.2.10: Killeroo Torrance-Sparrow scene. Render time: 0m33,900s Sample count: 16

![Sc](/../assets/hw6/hw6-2-11.jpg)

> Figure 6.2.11: Killeroo Torrance-Sparrow close scene. Render time: 1m9,586s Sample count: 16

### 6.3. Bugs and Fixes

Unfortunately (!) I didn't encounter any bugs in this version. I just refactored my code. Implementation was straight forward.

### 6.4. Conclusion

This version of the ray tracer was simpler to implement than previous ones. Once I understood the derivation of these models by watching the [Youtube videos](https://www.youtube.com/watch?v=PrhfRIplPXE&list=PLjU2zfBoP-T-YiDoRan5JyRujeuubDNDJ) of Ahmet Oguz Akyuz, it was very simple to add these features. For now, I am looking forward to move on to the next iteration of our project.