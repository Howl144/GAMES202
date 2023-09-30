#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);
    // root = recursiveBuildSAH(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

double BVHAccel::computeObjSurArea(std::vector<Object *> objects){
    double area = 0;
    for(auto obj:objects){
        area += obj->getBounds().SurfaceArea();
    }
    return area;
}

BVHBuildNode * BVHAccel::recursiveBuildSAH(std::vector<Object *> objects){
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in SAH node
    if (objects.size() == 1) {
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuildSAH(std::vector{objects[0]});
        node->right = recursiveBuildSAH(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 bounds;
        for (int i = 0; i < objects.size(); ++i)
            bounds = Union(bounds, objects[i]->getBounds());
        double boundsSurfaceAreaInv = 1.0f / bounds.SurfaceArea();

        auto beginning=objects.begin();
        auto ending=objects.end();
        if(objects.size() < 4){
            auto middling = objects.begin() +objects.size()/2;
            auto leftshapes = std::vector<Object*>(beginning, middling);
            auto rightshapes = std::vector<Object*>(middling, ending);
            node->left = recursiveBuildSAH(leftshapes);
            node->right = recursiveBuildSAH(rightshapes);
            node->bounds = Union(node->left->bounds, node->right->bounds);
        }
		else {
            int bestChoice=0;
            double minCost=std::numeric_limits<double >::max();
            float objSize =(float)objects.size();
            float nums[]={1.0/12,2.0/12,3.0/12,4.0/12,5.0/12,6.0/12,7.0/12,8.0/12,9.0/12,10.0/12,11.0/12};
            for(int i = 0;i < 11;i++)
                nums[i]*= objSize;
            int dim = bounds.maxExtent();
            switch (dim){
                case 0://x
                    std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                            { return f1->getBounds().Centroid().x <
                                    f2->getBounds().Centroid().x; });
                    break;
                case 1://y
                    std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                            { return f1->getBounds().Centroid().y <
                                    f2->getBounds().Centroid().y; });
                    break;
                case 2://z
                    std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                            { return f1->getBounds().Centroid().z <
                                    f2->getBounds().Centroid().z; });
                    break;
            }
            
            for(int i = 0;i < 11;i++){
                auto middling = objects.begin() + (int)nums[i];
                auto leftshapes = std::vector<Object*>(beginning, middling);
                auto rightshapes = std::vector<Object*>(middling, ending);
                double leftBoxArea = computeObjSurArea(leftshapes);
                double rightBoxArea = computeObjSurArea(rightshapes);
                double cost = 0.125f+
                ( leftBoxArea * leftshapes.size() + rightBoxArea * rightshapes.size() )
                 * boundsSurfaceAreaInv;
                if(cost < minCost){   
                    minCost = cost;
                    bestChoice=(int)nums[i];
                }
            }
            
            auto middling = objects.begin() + bestChoice;
            auto leftshapes = std::vector<Object*>(beginning, middling);
            auto rightshapes = std::vector<Object*>(middling, ending);
            node->left = recursiveBuildSAH(leftshapes);
            node->right = recursiveBuildSAH(rightshapes);
            node->bounds = Union(node->left->bounds, node->right->bounds);
        }
        return node;
    }
}

BVHBuildNode *BVHAccel::recursiveBuild(std::vector<Object *> objects)
{
    BVHBuildNode *node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    
    if (objects.size() == 1)
    {
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2)
    {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else
    {
        Bounds3 centroidBounds;
        //创建可以包围所有三角形包围盒中心点的包围盒
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        //计算包围盒的最大轴
        int dim = centroidBounds.maxExtent();
        switch (dim)
        {
        case 0://x
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().x <
                               f2->getBounds().Centroid().x; });
            break;
        case 1://y
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().y <
                               f2->getBounds().Centroid().y; });
            break;
        case 2://z
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().z <
                               f2->getBounds().Centroid().z; });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();
        //[beg,end)
        std::vector<Object *> leftshapes = std::vector<Object *>(beginning, middling);
        std::vector<Object *> rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);
        //该节点的包围盒大小是左右子节点包围盒的并集
        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray &ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode *node, const Ray &ray) const
{
    // TODO Traverse the BVH to find intersection
    Intersection isect;
    if(!node) return isect;

    std::array<int, 3> dirIsNeg;
    dirIsNeg[0] = ray.direction.x < 0;
    dirIsNeg[1] = ray.direction.y < 0;
    dirIsNeg[2] = ray.direction.z < 0;

    //光线与包围盒求交
    if (!node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg))
        return isect;

    //如果是叶子节点，则判断光线与物体（MeshTriangle and Triangle）是否相交。
    //场景内bvh的叶子节点是一个整个物体(MeshTriangle)，getIntersection会去调用物体(MeshTriangle)内bvh->Intersect()，最后会调用到三角形（Triangle）的getIntersection函数。
    if (node->left == nullptr && node->right == nullptr)
    {
        return node->object->getIntersection(ray);
    }

    Intersection hitLeft = getIntersection(node->left, ray);
    Intersection hitRight = getIntersection(node->right, ray);

    //返回距离最近的交点
    return hitLeft.distance < hitRight.distance ? hitLeft : hitRight;
}