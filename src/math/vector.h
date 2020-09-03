
#ifndef _VECTOR_H
#define _VECTOR_H

struct Vector3 {
    float x, y, z;
};

extern struct Vector3 gRight;
extern struct Vector3 gUp;
extern struct Vector3 gForward;

void vector3Negate(struct Vector3* in, struct Vector3* out);
void vector3Scale(struct Vector3* in, struct Vector3* out, float scale);

#endif