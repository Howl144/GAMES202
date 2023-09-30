#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object *> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    std::string method;
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;
    if (splitMethod == SplitMethod::NAIVE) {
        root = recursiveBuild(primitives);
        method = "BVH";
    }
    else if(splitMethod == SplitMethod::SAH){
        root = recursiveBuildSAH(primitives);
        method = "SAH";
    }
    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\r%s Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        method.c_str(),hrs, mins, secs);
}

double BVHAccel::computeObjSurArea(const std::vector<Object *>& objects){
    double area = 0;
    for(const auto& obj:objects){
        area += obj->getBounds().SurfaceArea();
    }
    return area;
}

BVHBuildNode * BVHAccel::recursiveBuildSAH(std::vector<Object *> objects){
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = objects[0]->getArea();
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuildSAH({objects[0]});
        node->right = recursiveBuildSAH({objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
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
            node->left = recursiveBuildSAH(std::move(leftshapes));
            node->right = recursiveBuildSAH(std::move(rightshapes));
            node->bounds = Union(node->left->bounds, node->right->bounds);
            node->area = node->left->area + node->right->area;
        }
		else {
            int bestChoice=0;
            double minCost=std::numeric_limits<double >::max();
            double objSize =(double)objects.size();
            double nums[]={1.0/12,2.0/12,3.0/12,4.0/12,5.0/12,6.0/12,7.0/12,8.0/12,9.0/12,10.0/12,11.0/12};
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
            node->left = recursiveBuildSAH(std::move(leftshapes));
            node->right = recursiveBuildSAH(std::move(rightshapes));
            node->bounds = Union(node->left->bounds, node->right->bounds);
            node->area = node->left->area + node->right->area;

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
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = objects[0]->getArea();
        return node;
    }
    else if (objects.size() == 2)
    {
        node->left = recursiveBuild({objects[0]});
        node->right = recursiveBuild({objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
        return node;
    }
    else
    {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim)
        {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().x <
                               f2->getBounds().Centroid().x; });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().y <
                               f2->getBounds().Centroid().y; });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2)
                      { return f1->getBounds().Centroid().z <
                               f2->getBounds().Centroid().z; });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object *>(beginning, middling);
        auto rightshapes = std::vector<Object *>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(std::move(leftshapes));
        node->right = recursiveBuild(std::move(rightshapes));

        node->bounds = Union(node->left->bounds, node->right->bounds);
        node->area = node->left->area + node->right->area;
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

    std::array<int, 3> dirIsNeg;
    dirIsNeg[0] = ray.direction.x < 0;
    dirIsNeg[1] = ray.direction.y < 0;
    dirIsNeg[2] = ray.direction.z < 0;

    if (!node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg))
        return isect;

    if (node->left == nullptr && node->right == nullptr)
    {
        return node->object->getIntersection(ray);
    }

    Intersection hitLeft = getIntersection(node->left, ray);
    Intersection hitRight = getIntersection(node->right, ray);

    return hitLeft.distance < hitRight.distance ? hitLeft : hitRight;
}

void BVHAccel::getSample(BVHBuildNode *node, double p, Intersection &pos, double &pdf)
{
    if (node->left == nullptr || node->right == nullptr)
    {
        node->object->Sample(pos, pdf);
        //pdf是1 / root->area,这里乘以node->area是为了保证pdf返回时一定为1.
        pdf *= node->area;
        return;
    }
    if (p < node->left->area)
        getSample(node->left, p, pos, pdf);
    else
        getSample(node->right, p - node->left->area, pos, pdf);
}

void BVHAccel::Sample(Intersection &pos, double &pdf)
{
    double p = get_random_double() * root->area;
    getSample(root, p, pos, pdf);
    Vector3f lightNormal = pos.normal;
    Vector3f lightDir = pos.coords - pos.tcoords;
    double distance = lightDir.norm();
    //光源面积转立体角
    pdf = root->area * std::fmax(dotProduct(-lightDir.normalized(), lightNormal),0.0) / (distance * distance);

    pdf = !pdf ? 0.0 : 1.0 / pdf;
    //dw转dA用下面的公式
    //pdf = 1.0 / root->area;
}