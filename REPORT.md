# 真实感图像渲染器 项目报告

<div style="text-align:center;font-family:楷体;font-size: 0.9em">王星淇 2019011340 计96</div>

[TOC]

## 项目概述
### 整体概况
本项目以课程PA1代码为基本框架，基于重要性采样的路径追踪算法实现了带有反射、折射、纹理、支持三角网格模型的真实感图像渲染器（光线追踪引擎）。物体方面，实现了平面、矩形、三角、球体、圆柱体、长方体、Beizer和B样条曲线的旋转曲面、复杂三角网格的求交，并可通过线性变换对物体进行拉伸、平移、旋转等操作。材质方面，实现了漫反射材质，镜面反射材质（包含从漫反射到完全镜面反射的过渡材质），透明材质，散射体材质等。纹理方面，实现了棋盘格纹理，柏林噪声纹理，贴图纹理等。性能优化方面，实现了轴对齐层次包围盒的算法加速，并使用OpenMP进行硬件加速。此外，还实现了抗锯齿、软阴影、景深、运动模糊、体积光（基于散射体材质）等功能
### 运行环境
本项目使用cmake套件进行编译，其版本要求参见cmakelist.txt。项目在windows和linux环境下均可正常运行，分别使用项目根目录中的`run_all.ps1`和`run_all.sh`脚本即可编译运行（注：windows环境下需要安装MinGW）。
### 文件目录
\project	根目录

   \bin	存放编译后可执行文件

   \build cmake生成的编译文件

   \deps 相关依赖

   \include 物体、场景生成、算法的hpp文件

   \mesh obj资源文件

   \output 图像输出

   \resource 贴图资源

   \src 主函数和物体、场景生成的cpp文件

   \testcase 文本输入的txt文件

   REPORT.md/pdf 项目报告

   run_all.sh/ps1 运行脚本

### 运行说明
在项目根目录下直接运行对应系统的脚本即可。可通过修改脚本中运行可执行文件的命令行参数控制输入与输出。

可执行文件命令行格式如下（以windows系统为例）：

```shell
.\bin\FINAL.exe <method> <input> <output>

# 内部场景生成器
.\bin\FINAL.exe 0 5 .\output\final_scene1.bmp
# 文件输入
.\bin\FINAL.exe 1 .\testcases\simpleLight.txt .\output\simpleLight.bmp
```
其中，method参数选择输入方法，为0时使用内部的场景生成器，为1时使用文件输入格式。input参数选择具体输入，使用内部场景生成器时，参数为一个数字以选择对应场景（具体选择场景范围可见scene_generator.hpp文件中的switch语句，您也可以根据给定的示例修改现有的场景或编写新的场景）；使用文件输入时，参数为输入文件的路径。output参数给定输出图片的路径。文件具体输入格式可参见给定的范例，需注意，由于时间所限，文件输入不支持实现的全部功能（如长方体等）且可能存在未知bug，使用需谨慎。

## 渲染算法
本项目的渲染算法为路径跟踪算法。具体思路是从相机向像素发出射线，经过场景中物体不断反射、折射，如果能够达到光源，则将光源发光强度乘上光路相交物体的的反射率（衰减率），作为该光源对像素颜色的贡献；如果无法打到物体，则将背景光强乘上光路相交物体的反射率作为该像素的颜色；如果超过预先设定好的迭代深度，则将之前所有积累的光照强度算作该像素的颜色，并退出。由于存在漫反射面反射、折射面折射与反射导致光线分叉、部分镜面反射等不确定情况，因此采用蒙特卡洛模拟方法，对每一个像素多次发出射线，射线接触物体过程中的漫反射、折射面光线分叉、部分镜面反射的情况都根据物理定律计算相应概率，根据概率计算相应的出射方向，将每次发出射线的色彩贡献取平均作为该像素点的颜色。路径跟踪算法能够实现较好的漫反射面、镜面和折射光线直接进入相机的成像效果，并且具有较好的Color Bleeding效果。

主算法代码如下：

```c++
void render() {
    for (int x = 0; x < image_width; ++x) {
        printf("\rrendering image pass %.3lf%%", x*100.f/image_width);
        #pragma omp parallel for schedule(dynamic, 60), num_threads(12)
        for (int y = 0; y < image_height; ++y) {
            Vector3f finalColor = Vector3f::ZERO;
            float actual_samples = sample_per_pixel; 
            for (int i=0; i<sample_per_pixel; i++) {
                float bias_x = random_double(0,1);
                float bias_y = random_double(0,1);
                Ray camRay = camera->generateRay(Vector2f(x+bias_x, y+bias_y));
                Vector3f color = traceRay(camRay, max_depth, init_weight);
                for (int i = 0; i < 3; i++) {
                    if(color[i] != color[i]||color[i] < 0) {
                        color[i] = 0;
                        actual_samples -= 1;
                    }
                }
                finalColor += color;
            }
            if (actual_samples < 1.0) actual_samples = 1.0;
            finalColor = finalColor/actual_samples;
            renderedImg->SetPixel(x, y, finalColor);
        }
    }
    printf("\rrendering image pass 100.000%%\n");
    renderedImg->SaveImage(outputfile);
    printf("Successfully rendered image!\n");
}
```

其中具体的光线追踪算法写在`traceRay()`函数中，具体代码如下：
```c++
Vector3f traceRay(Ray &camRay, int depth, float weight) {
    Hit record;
    if (depth<=0||weight<MIN_WEIGHT){
        return Vector3f::ZERO;
    }

    if (!baseGroup->intersect(camRay, record, 0.001, infinity)) {
        return backgroundColor;
    }
    Vector3f color = Vector3f::ZERO;
    ScatterRecord srec;
    Vector3f attenuation;
    Vector3f emitted = record.getMaterial()->emitted(record, record.u, record.v, record.getIntersectP()); 
    if(!record.getMaterial()->scatter(camRay,record,srec)) {
        if (depth == max_depth) {
            return record.getMaterial()->emitted(record, record.u, record.v, record.getIntersectP(),true);
        }
        else {
            return emitted;
        }
    }
    if(srec.is_specular) {
        return srec.attenuation * traceRay(srec.specular_ray, depth-1, weight);
    }
    shared_ptr<PDF> light_ptr = make_shared<HittablePDF>(lights, record.getIntersectP());
    MixturePDF p(light_ptr, srec.pdf_ptr);
    Ray scattered = Ray(record.getIntersectP(), p.generate(), camRay.getTime());
    auto pdf_val = p.value(scattered.getDirection());

    return emitted
        + srec.attenuation * record.getMaterial()->scatterPDF(camRay, record, scattered)
        * traceRay(scattered, depth-1, weight*attenuation.length()) / pdf_val;
}
```
Color Bleeding 效果演示如图，可见兔子身上体现的较为明显：
![bunny](./output/bunny_fine1.bmp)

同时，在具体的实现中，我还实现了重要性采样技术，根据不同材质和不同表面设置不同的出射光线分布概率密度函数，并且单独设置直接由物体交点指向光源的概率密度函数。每次相交时，以一定概率选择两种密度函数的一种，以适当增大光线最终命中光源的概率。由此，在较小的模拟次数下即可取得较好的效果，大大减小因未命中光源而产生的黑色噪点数量。

`traceRay()`函数中与重要性采样相关的代码如下，概率密度函数的具体实现可参见pdf.hpp和pdf.cpp的全部内容，这里不再展示。
```c++
if(srec.is_specular) {
	return srec.attenuation * traceRay(srec.specular_ray, depth-1, weight);
}
shared_ptr<PDF> light_ptr = make_shared<HittablePDF>(lights, record.getIntersectP());
MixturePDF p(light_ptr, srec.pdf_ptr);
Ray scattered = Ray(record.getIntersectP(), p.generate(), camRay.getTime());
auto pdf_val = p.value(scattered.getDirection());
```

实现前后在相同采样次数下的效果对比如图：
![before](./output/cornell_box.bmp) ![after](./output/cornell_box_is.bmp)

## 性能优化
### 算法加速
在算法加速方面，我主要实现了层次包围盒技术。其中，包围盒我使用了轴对齐包围盒技术，即沿坐标轴方向构建物体的包围盒，求交时先判断是否与包围盒相交，只有相交后才进行物体本身的求交运算。由于与坐标轴平行的性质，与包围盒求交非常简单，只需只需计算三个维度光线射入和射出的t，取三个tmax 中的最小者和三个tmin 中的最大者比较即可。轴对齐包围盒的代码部分可以参见aabb.hpp文件。而将包围盒用树状结构组织起来，就形成了层次包围盒。构建层次包围盒时，叶子节点为实际物体，父节点的包围盒为全部子节点包围盒的包围盒，由此即可自下而上构建层次包围盒。求交时，自顶向下，若与父节点包围盒相交，再与两个子节点的包围盒相交，直到叶子节点为止。这样避开了大量的求交运算，有效的提升了渲染速度。层次包围盒的实现可以参见bvh.hpp和bvh.cpp文件的全部内容。

层次包围盒特别适用于复杂网格模型的求交，将三角面片用层次包围盒组织起来即可大大加快三角网格的求交速度。从原理上分析，其将复杂度从$O(n)$降到了$O(\log{n})$，与实测用时基本相符。层次包围盒的代价在于构建时使用指针大量申请内存耗时增加，且很难使用并行优化，但相比于反复进行的求交运算还是值得的。

### 硬件加速

硬件加速我主要使用了OpenMP，通过调用CPU的多个核心同时对不同位置的像素进行渲染实现并行加速。虽然没有分数，但是有效提升了运行速度，节省了渲染时间，也节省了调试的时间。具体代码在render.hpp文件的`render()`函数中：

```c++
#pragma omp parallel for schedule(dynamic, 60), num_threads(12)
```

## 附加功能
### 抗锯齿
锯齿产生的原因在于只对像素的中心点进行采样，必然产生不够平滑的情况，在图像上就表现为锯齿。因此每次对像素点采样时，可以在宽度和高度两个方向上各引入一个0-1之间的随机数，采样完后取平均，这样相当于做了一次平滑，自然就减少了锯齿。具体效果可见上文插图，代码如下：
```c++
for (int i=0; i<sample_per_pixel; i++) {
    float bias_x = random_double(0,1);
    float bias_y = random_double(0,1);
    Ray camRay = camera->generateRay(Vector2f(x+bias_x, y+bias_y));
    Vector3f color = traceRay(camRay, max_depth, init_weight);
    for (int i = 0; i < 3; i++) {
        if(color[i] != color[i]||color[i] < 0) {
            color[i] = 0;
            actual_samples -= 1;
        }
    }
    finalColor += color;
}
if (actual_samples < 1.0) actual_samples = 1.0;
finalColor = finalColor/actual_samples;
renderedImg->SetPixel(x, y, finalColor);
```
### 软阴影
路径追踪算法是一种全局光照模型的算法，可以自动实现软阴影，在上面几张图中均有表现。
### 景深

景深效果的本质在于现实世界中的摄像机光圈并不是理想的小孔，而是具有一定半径，这会导致成像发生散焦。为了修正散焦，可以在孔内加入一块透镜，由此在一定距离范围内的所有物体都会被对焦。可见，如果代码中的相机在射出光线时，可以随机的从一个圆盘范围而不是一个点射出光线，就与现实世界中相机类似，由此就可以实现景深效果。

具体效果如下图：

![depth](./output/cornell_box_depth1.bmp)

![depth](./output/cornell_box_depth.bmp)

代码如下，在camera类的`generatorRay()`函数中：
```c++
Vector3f rd = len_radius * random_in_unit_disk();
```

### 运动模糊

运动模糊的本质在于为相机设置一个曝光时间范围，每次射出射线取一个在曝光时间范围内的随机数，代表射线射出的时间。对于可运动的物体，设置一个位置随时间变化的函数，求交时，根据射线的时间判断其所处的位置，根据这一位置计算是否相交。由此即实现运动模糊。

代码如下，射出射线时间在最后Ray的构造函数中设置。可运动物体可以参见moving_sphere.hpp中的全部内容。
```c++
Ray generateRay(const Vector2f &point) override {
        Vector3f rd = len_radius * random_in_unit_disk();
        Vector3f offset = horizontal * rd.x() + up * rd.y();
        Vector3f dir_ray = focus_origin + (point.x()/(width-1)) * focus_horizontal + (point.y()/(height-1)) * focus_vertical - origin - offset;
        dir_ray.normalize();
        return Ray(origin+offset,dir_ray,random_double(time0,time1));
}
```
效果如下图，可见有一个运动的紫色小球：
![moving](./output/weekend.bmp)

### 纹理贴图
纹理贴图的关键在于将物体表面上的点映射为贴图的像素。可以为贴图建立一个直角坐标系，横纵坐标分别为u, v（范围在0到1之间，代表比例）。射线与物体相交时，计算交点在物体上的相对位置，由此推算出uv的值，再取贴图上相应坐标的点。对于矩形，直接映射即可；对于球体，可以通过球坐标与直角坐标的转换完成；对于旋转曲面，可以通过柱坐标系和直角坐标系的转换完成。

以球体为例，其转换代码如下。其他物体可参见`intersect`函数中Hit类对象的u, v设置。
```c++
static void get_sphere_uv(const Vector3f& p, float& u, float& v)  {
        auto phi = atan2(p.z(), p.x());
        auto theta = asin(p.y());
        u = 1-(phi + M_PI) / (2*M_PI);
        v = (theta + M_PI/2) / M_PI;
    }
```
效果可见上一节图像的地球贴图。

贴图的具体加载与映射可见texture.hpp和texture.cpp文件中ImageTexture类的实现。

### 柏林噪声
柏林噪声与贴图的思路类似，也是根据交点相对物体的位置根据特定函数获取颜色值。而函数直接根据资料输入公式即可。代码实现可见perlin.hpp的Perlin类与texture.hpp中的NoiseTexture类的全部内容。

效果可见运动模糊一节图像的地面材质。
### 散射材质（烟雾、体积光）
散射材质的物理模型是，光线在与散射材质物体相交时，并不直接发生反射或折射，而是进入物体，在进入后穿出前以一定概率向各个方向发生散射，由此就可以实现类似烟雾等散射体的效果。在代码中，通过constant_medium.hpp中的虚拟物体ConstantMedium类，以及material.hpp中的Isotropic类（215-237行）共同实现。之所以称ConstantMedium类为虚拟物体类，是因为它本身并没有有形的边界，而是依赖于其他实体物体类如球、长方体等为其设置边界。通过求交时在边界范围内随机设置t值实现进入物体后发生散射，同时即使相交也有一定概率直接返回false，实现射线穿透效果。当物体只占据一部分空间时，效果相当于烟雾，当物体充满整个空间时并设置好散射光线分布的概率密度函数时，效果相当于体积光。具体代码实现可参考上述两个文件的对应部分。

具体效果如下：
![smoke](./output/cornell_smoke_is.bmp)
![scatter](./output/cornell_box_light.bmp)

上图虽然有了较为明显的散射效果，但是由于概率密度函数是球形均匀分布，导致体积光表现的不是特别好，未来有机会的话，可以继续改进概率密度函数，使其表现真实的效果。

### 三角面片法向量插值
为了使用较少的面片就获得柔和的曲面效果，可以在三角形面片求交时进行法向量插值处理。具体思路是：在已知三角形各顶点法向量的情况下用交点与三顶点连线形成的三个三角形面积作为权重，对三个顶点法向量做线性插值操作即可得到交点的法向量。具体代码可以参见triangle.hpp中`getNorm()`函数的实现（107-114行）。各顶点法向量可以由obj文件直接导入，此外在旋转曲面中也实现了求顶点法向量，在后文中叙述。对于不含顶点法向量的obj文件，可以通过对顶点所属三角面片的法向量以三角面片在该顶点所对的角为权重进行插值处理，但囿于时间精力以及计算量，没有实现。

旋转曲面转换成三角面片的效果如图所示：
![curve_mesh](./output/final_scene22.bmp)

## 复杂网格
复杂网格模型求交的基础在PA1中均已给出，项目主要做出了两点改进。一是使用tiny obj loader替代了原有的文件输入处理代码，速度更快，功能更丰富，如支持顶点法向量导入等；二是使用层次包围盒保存全部的三角面片，求交速度更快。具体代码实现仍可参见mesh.cpp文件中Mesh构造函数`Mesh()`与求交函数`intersect()`的实现。

## Beizer和B样条旋转曲面求交
Beizer和B样条旋转曲面的解析法求交是我在完成项目中最艰难、耗时最多、研究最多的一部分，前前后后共用了三天完整的时间。

首先我按照常规思路高斯牛顿迭代法，通过计算雅可比矩阵进行迭代逼近，但是效果却始终不尽如人意，总是出现大量残缺（可参见output文件夹中保存的若干图）。这应该是高斯牛顿迭代法不收敛导致的，为了解决这个问题我通过与同学、助教交流尝试了各种附加措施，比如设置圆柱包围盒得到初始值的精确估计；通过设置取值区间并在区间内多次取初始值尝试迭代求解；迭代过程中如果结果超过取值范围阈值则截断；设置迭代速率等等。这些方法一定程度上改善了结果，但并没有完全解决残缺的问题。

通过查阅资料，我得知高斯牛顿迭代法本身就并非解决最小二乘最优化问题的最佳算法，极其容易出现不收敛的问题。由此，我学习了Levenberg-Marquardt迭代法，并且在代码中实现。经过实验后我发现，这一方法不仅收敛性较高斯牛顿迭代更好，而且速度也更快，经过调节参数后，几乎已经完全解决了破损问题，但在边缘处仍有少许瑕疵。我进一步思考，认为突破口仍在初始值的精确估计上。毕竟圆柱包围盒相较内部曲线仍有不少距离，特别是用包围盒交点相对于圆柱高度估计曲线参数更是可能存在严重误差，例如所给的酒杯物体上半部分曲线存在折叠的情况，某种意义上变成了双层曲线，仅使用高度估计显然不可能得到精确的估计值。因此，我借用网格化的思想，首先对整条曲线进行离散化取样，以每两个取样点x轴坐标绝对值（即旋转曲面在该点的半径）的最大值构造圆柱体，再将全部的圆柱体组织成层次包围盒的形式，令光线与这些圆柱体相交，根据所交的圆柱体的半径，相关联的取样点的参数获得初始值更加精确的估计。最终，我取得了令人较为满意的结果，如下图所示。

![bezier1](./output/bezier_perfect.bmp)

关于解析法求交的具体代码如下：
```c++
\\构造函数中离散化取样的部分
pCurve->discretize(resolution, curvePoints);
            std::vector<shared_ptr<Object3D>> cylinders;
            cylinders.reserve(curvePoints.size()-1);
            for (int i=0; i<curvePoints.size()-1; i++) {
                float r = fmax(fabs(curvePoints[i].V.x()), fabs(curvePoints[i+1].V.x()));
                cylinders.push_back(make_shared<Cylinder>(Vector3f(0,0,0), r, fmin(curvePoints[i].V.y(), curvePoints[i+1].V.y()),
                                fmax(curvePoints[i].V.y(), curvePoints[i+1].V.y()), material));
            }
            cylinder_bvh = make_shared<BVHnode>(cylinders, 0, cylinders.size(), 0, 0);
            
\\求交函数
bool intersect(const Ray &r, Hit &h, float tmin = 0.0, float tmax = infinity) const override {
    Vector3f ro = r.getOrigin();
    Vector3f rd = r.getDirection();
    double dt, dl, d0;

    // intersect with the cylinder
    double l, a, b, c;
    a = (rd.x() * rd.x() + rd.z() * rd.z());
    b = (2 * rd.x() * ro.x() + 2 * rd.z() * ro.z());
    c = ro.x() * ro.x() + ro.z() * ro.z() - radius * radius;
    double det = b * b - 4 * a * c;

    if (det < 0) return false;
    else det = std::sqrt(det);
    if ((-b - det) / (2 * a) > tmin) l = (-b - det) / (2 * a);
    else if ((-b + det) / (2 * a) > tmin) l = (-b + det) / (2 * a);
    else return false;
    if(l > tmax) return false;
    Vector3f np = ro + l * rd;

    double t = (np.y() - miny) / (maxy - miny);
    if (t < 0 || t > 1) return false;
    double theta0 = std::acos(np.x() / radius);

    Hit rec;
    std::vector<float> t_vec;
    if (cylinder_bvh->intersect(r, rec, tmin, tmax)) { 
        l = rec.getT();
        theta0 = rec.u * 2 * M_PI;
        np = ro + l * rd;
        t = (np.y() - miny) / (maxy - miny);
        for (int i = 0; i < curvePoints.size()-1; i++) {
            if (np.y() >= fmin(curvePoints[i].V.y(), curvePoints[i+1].V.y()) && np.y() < fmax(curvePoints[i].V.y(), curvePoints[i+1].V.y())) {
                t_vec.push_back((i + (np.y() - curvePoints[i].V.y())/(curvePoints[i+1].V.y() - curvePoints[i].V.y()))/(float)resolution);
            }
        }

    }
    bool flag = false; float cloest_t = tmax;
    if (Levenberg_Marquardt(r, h, tmin, cloest_t, l, t, theta0)){
        flag = true;
        cloest_t = h.getT();
    }
    for (auto tt : t_vec) {
        if (Levenberg_Marquardt(r, h, tmin, cloest_t, l, tt, theta0)) {
            cloest_t = h.getT();
            flag = true;
        }
    }

    float interval = 100;
    for (int j = 0; j <= interval; j ++){
        double s = pCurve->range[0] + (pCurve->range[1] - pCurve->range[0]) * (float)j/interval;
        if (Levenberg_Marquardt(r, h, tmin, cloest_t, l, s, theta0)) {
            flag = true;
            cloest_t = h.getT();
        }

    }
    return flag;
}

\\ 高斯牛顿迭代法
bool newton_iteration(const Ray &r, Hit &h, float tmin, float tmax, double tr, double s, double theta) const {
    double eps = 1, eta = 1;
    Vector3f F0 = Vector3f(0, 0, 0);
    for (int i = 0; i < NEWTON_STEPS; ++i) {
        if (s >= 1) s = 1.0 - FLT_EPSILON;
        if (s <= 0) s = FLT_EPSILON;
        Vector3f p = r.pointAtParameter(tr);
        CurvePoint fs = pCurve->caculate(s);
        Vector3f F(p.x() - fs.V.x() * cos(theta),
                   p.y() - fs.V.y(),
                   p.z() - fs.V.x() * sin(theta));
        if (F.length() < eps) {
            if(eps < 0.01){
                if (tr >= tmin && tr < tmax) {
                    Vector3f n(-fs.T.y() * cos(theta), fs.T.x(), -fs.T.y() * sin(theta));
                    n.normalize();
                    h.set(tr, material, n, r);
                    h.u = theta/(2*M_PI);
                    h.v = 1 - s;   
                    return true;
                }
                return false;
            }
            eps *= 0.8, eta *= 0.8;
        }

        Matrix3f JF(r.getDirection().x(), -cos(theta) * fs.T.x(), sin(theta) * fs.V.x(),
                    r.getDirection().y(), -fs.T.y(),              0,
                    r.getDirection().z(), -sin(theta) * fs.T.x(), -cos(theta) * fs.V.x());
        Vector3f delta = JF.inverse() * F;
        tr -= eta * delta.x(); s -= eta * delta.y(); theta -= eta * delta.z();

        if (std::isnan(tr) || std::isnan(s) || std::isnan(theta))
            return false;
    }
    return false;
}

\\Levenberg-Marquardt迭代法
bool Levenberg_Marquardt(const Ray &r, Hit &h, float tmin, float tmax, double tr, double s, double theta) const {
     float epsilon1 = 1e-10, epsilon2 = 1e-10, epsilon = 0.01;
     int imax = 100; float nu=2.0;
     Vector3f X(tr, s, theta);
     CurvePoint fs = pCurve->caculate(s);
     Vector3f p = r.pointAtParameter(tr);
     Matrix3f JF(r.getDirection().x(), -cos(theta) * fs.T.x(), sin(theta) * fs.V.x(),
                 r.getDirection().y(), -fs.T.y(),              0,
                 r.getDirection().z(), -sin(theta) * fs.T.x(), -cos(theta) * fs.V.x());
     Vector3f F(p.x() - fs.V.x() * cos(theta),
                p.y() - fs.V.y(),
                p.z() - fs.V.x() * sin(theta));
     Matrix3f H = JF.transposed()*JF;
     Vector3f g = JF.transposed()*F;
     bool found = g.length() <= epsilon1 || F.length() < epsilon;
     float mu = - infinity;
     for (int i = 0; i<3; i++) {
         mu = fmax(mu, H(i,i));
     }
     mu *= 0.001;
     for (int i = 0; i<=imax; i++) {
         if (found) {
             if (X.x() >= tmin && X.x() < tmax && X.y() <= pCurve->range[1] && X.y() >= pCurve->range[0] && F.length() < epsilon) {
                 Vector3f ro = r.getOrigin(); ro.y() = 0;
                 Vector3f n(fs.T.y() * cos(X.z()), -fs.T.x(), fs.T.y() * sin(X.z()));
                 if ((ro.length() > (fabs(fs.V.x()+0.001)) && Vector3f::dot(r.getDirection(), n) > 0)||
                     (ro.length() < (fabs(fs.V.x()+0.001)) && Vector3f::dot(r.getDirection(), n) < 0)) {
                     n = -n;
                 }
                 n.normalize();
                 h.set(X.x(), material, n, r);
                 h.u = X.z()/(2*M_PI);
                 h.v = 1 - X.y();
                 return true;
             } 
             return false;
         }
         H(0, 0) += mu;H(1, 1) += mu;H(2, 2) += mu;
         Vector3f delta =  H.inverse() * (- g);

         if (delta.length() < epsilon2*(X.length() * epsilon2)) { 
             found = true;
         } else {
             Vector3f X_new = X + delta;
             CurvePoint fs_new = pCurve->caculate(X_new.y());
             Vector3f p_new = r.pointAtParameter(X_new.x());
             Vector3f F_new(p_new.x() - fs_new.V.x() * cos(X_new.z()),
                            p_new.y() - fs_new.V.y(),
                            p_new.z() - fs_new.V.x() * sin(X_new.z()));
             float rho = (F.squaredLength()-F_new.squaredLength())/(Vector3f::dot(delta, mu*delta - g));
             if (rho > 0) {
                 X = X_new;
                 fs = fs_new;
                 p = p_new;
                 F = F_new;
                 JF = Matrix3f(r.getDirection().x(), -cos(X.z()) * fs.T.x(), sin(X.z()) * fs.V.x(),
                               r.getDirection().y(), -fs.T.y(),              0,
                               r.getDirection().z(), -sin(X.z()) * fs.T.x(), -cos(X.z()) * fs.V.x());
                 H = JF.transposed()*JF;
                 g = JF.transposed()*F;
                 found = g.length() <= epsilon1 || F.length() < epsilon;
                 mu = mu * fmax(1.0/3.0, 1-pow(2*rho-1, 3));
                 nu = 2.0;
             } else {
                 mu = mu * nu;
                 nu = 2 * nu;
             }
         }
         if (std::isnan(X.x()) || std::isnan(X.y()) || std::isnan(X.z()))
             return false;
         if (X.y() >= 1) X.y() = 1.0 - FLT_EPSILON;
         if (X.y() <= 0) X.y() = FLT_EPSILON;
         if (X.x() < tmin) X.x() = tmin + FLT_EPSILON;
         if (X.x() > tmax) X.x() = tmax - FLT_EPSILON;
     }
     return false;
 }
```
与此同时，我还实现了将旋转曲面转化为三角网格模型的方法，其实其基本思路在PA3旋转曲面类的drawGL方法就以实现，直接稍加改造，使用层次包围盒组织三角面片，求交时直接对三角网格求交即可。此外，通过计算取样点的曲面法向量即可通过矩阵变换获得三角面片顶点的法向量，通过上面叙述的法向量插值即可获得较为柔和的曲面效果。

但三角网格法存在不少缺点，如取样精度较高时使用层次包围盒构建三角网格的时间极长，渲染速度仍然略慢于初始值估计较为精确的参数解析法求交，而且三角网格剖分不够精细的话，仍然能看出三角面片，相比于参数解析法仍然不够平滑。特别是对于金属、玻璃等非漫反射材质三角面片呈现效果远不如参数解析法（如下图所示）。此外三角网格较难映射uv坐标，贴图处理也不如参数解析法简便。因此，在解决迭代法收敛问题后，参数解析法是更优的方法。不过在较低取样精度下，三角网格模型法仍不失为一种调试和观测图片小样的好方法。
![curve_mesh](./output/final_scene22.bmp)

## 结果与感想

最终几张较好的结果如下图所示：

![final_scene1](./output/final_scene1.bmp)

这张图是我实现功能的一个较为综合的展示，包括各类物体、材质、纹理、贴图、散射体（不太明显，但可对比光源下方背景色与图片边缘背景色的区别）、运动模糊等。事实上也开了景深、但为了方便辨认主要物体，调的不是特别明显。其中比较有创意的一点是实现了光源材质和纹理贴图的组合，从而做出了一个发光太阳的效果。此外，通过随机化生成物体，实现了较有艺术感的地面（由随机生成的长方体组成），和类似于泡沫塑料或分子模型的立方体（由在立方体区间内随机生成的球体组成），二者均通过层次包围盒进行加速。

![final_scene2](./output/final_scene2.bmp)

这张图主要展示了Bezier曲线的旋转面求交，其中，背景是利用天空盒贴图完成，旋转面使用镜面材质反射周围环境。其实，我本人更喜欢Bezier求交一节的黄昏背景，颇有一种《三体》水滴的意境，但是奈何背景清晰度有限，边缘较为模糊，影响了图片质量，只能换一套更清晰的天空盒素材。

![final_scene3](./output/final_scene3.bmp)

这张图主要展示复杂网格模型的求交，其实我的初衷是想营造一种青龙行于九天的感觉，但是实际出图效果并没有那么理想，但是体现复杂网格模型求交的目的毕竟是达到了，因此也作为最终图片展示。

通过这次大作业，我对图形学在整体上可谓有了更深刻的认识，从核心的渲染算法，到一些有趣而实用的小功能，从架构设计到数学方法，我都有了亲身的体验和收获。我感觉图形学的一大魅力在于，每学习一点内容，都能看到自己的图像向真实的方向更进了一步，都能更精确地用数学和代码在显示屏上描绘世界的模样，这给人一种创造世界的奇妙感觉，大大激发我进一步深入探索的热情和欲望。当然，探索也并不是一番风顺的，例如我在Beizer和B样条旋转曲面求交方面就遭遇了很多困难，但困难越多，最终取得的收获也越多。事实上Beizer和B样条旋转曲面求交是我这次大作业中体会最深刻、收获相关知识最多的部分。与此同时，由于时间和精力所限，仍留下了不少遗憾，比如心心念念的SPPM算法因为调试Beizer和B样条旋转曲面花费太多时间而最终没有实现等，希望未来能有机会继续完善改进这一项目；再比如电脑在渲染时频繁因为过热而进入死机状态，导致渲染时间十分有限，最终图片的质量仍然不完全尽如人意，只能说是尽力而为了。

在完成大作业中，我参考了不少资料，包括Peter shirley的Ray Tracing in one weekend, the next week, the rest of the life系列，往届学长的实验报告，知乎的专栏文章，相关博客等等。在此向这些学习资源的提供者、分享者真诚致谢，也向计算机学界的开源分享精神致敬！

最后，感谢胡老师和助教一个学期以来的指导教学和及时答疑！您们的出色工作让我深深领略到了到图形学的浪漫与魅力！