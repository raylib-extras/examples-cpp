# fps collsion example
A very simple demo that shows a simple version of FPS style collisions.

The world is made of obstacles that are transformed in 3d space. Collisions are done on the axis alligned bounding box (AABB) for each object.
Colliding things are transformed into the rotated object's space and then tested, that way simple AABB tests can be used for rotated objects.
Both cylinder and ray collisions are shown in the example