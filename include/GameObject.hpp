#ifndef _GAMEOBJECT_H
#define _GAMEOBJECT_H

#include "Position.hpp"
#include "Direction.hpp"
#include "Animation.hpp"
#include "TowerRange.hpp"
#include <cmath>
#include <boost/serialization/base_object.hpp>

class GameObject {
public:
    GameObject(): position(nullptr), direction(nullptr), animation(nullptr), objectId(0), boundingBoxRadius(0.0) {}

    GameObject(Position* position, Direction* direction, Animation* animation, 
        unsigned int objectId, float radius) : objectId(objectId), boundingBoxRadius(radius) {
        this->position = position;
        this->direction = direction;
        this->animation = animation;
    }

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & position;
        ar & direction;
        ar & animation;
        ar & objectId;
    }

    ~GameObject() {
        delete position;
        delete direction;
        delete animation;
    }

    float distanceTo(Position& pos) {
        return std::sqrt((position->x - pos.x) * (position->x - pos.x) + (position->z - pos.z) * (position->z - pos.z));
    
    }

    float distanceTo(GameObject* that) {
        //return std::sqrt((position->x - that->position->x) * (position->x - that->position->x) + (position->z - that->position->z) * (position->z - that->position->z));
        return distanceTo(*(that->position));
    }

    /*
    bool collideWith(GameObject* that) {
        float distance = this->distanceTo(that);
        return distance <= boundingBoxRadius + that->boundingBoxRadius;
    }
    */

    bool collideWith(GameObject* that) {
        float thisMinX = position->x - boundingBoxRadius;
        float thisMaxX = position->x + boundingBoxRadius;
        float thisMinZ = position->z - boundingBoxRadius;
        float thisMaxZ = position->z + boundingBoxRadius;

        float thatMinX = that->position->x - that->boundingBoxRadius;
        float thatMaxX = that->position->x + that->boundingBoxRadius;
        float thatMinZ = that->position->z - that->boundingBoxRadius;
        float thatMaxZ = that->position->z + that->boundingBoxRadius;

        float d1x = thatMinX - thisMaxX;
        float d1z = thatMinZ - thisMaxZ;
        float d2x = thisMinX - thatMaxX;
        float d2z = thisMinZ - thatMaxZ;

        if (d1x > 0.0f || d1z > 0.0f)
            return false;

        if (d2x > 0.0f || d2z > 0.0f)
            return false;

        return true;
    }

    bool highlightCollideWith(GameObject* that) {
        float distance = this->distanceTo(that);
        return distance <= boundingBoxRadius + that->boundingBoxRadius + 1.0f;
    }

    Position* position; // Plant position
    Direction* direction; // Direction the  facing
    Animation* animation; // type of animation
    float boundingBoxRadius; // radius of the Circle Bounding Box
    unsigned int objectId;
};

#endif
