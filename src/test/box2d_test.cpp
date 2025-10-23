#include <box2d/box2d.h>
#include <iostream>

int main()
{
    // Create world using C API
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){ 0.0f, -10.0f };
    b2WorldId world = b2CreateWorld(&worldDef);

    // Ground body
    b2BodyDef groundDef = b2DefaultBodyDef();
    groundDef.position = (b2Vec2){ 0.0f, -10.0f };
    b2BodyId ground = b2CreateBody(world, &groundDef);

    // Ground box shape (static)
    b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.density = 0.0f;
    groundShapeDef.material.friction = 0.0f;
    b2CreatePolygonShape(ground, &groundShapeDef, &groundBox);

    // Dynamic body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){ 0.0f, 4.0f };
    b2BodyId body = b2CreateBody(world, &bodyDef);

    // Dynamic box shape
    b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);
    b2ShapeDef dynamicShapeDef = b2DefaultShapeDef();
    dynamicShapeDef.density = 1.0f;
    dynamicShapeDef.material.friction = 0.3f;
    b2ShapeId dynamicShape = b2CreatePolygonShape(body, &dynamicShapeDef, &dynamicBox);

    // Update mass from shapes so the body has correct mass
    b2Body_ApplyMassFromShapes(body);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4; // use multiple substeps for accuracy

    for (int i = 0; i < 60; ++i)
    {
        b2World_Step(world, timeStep, subStepCount);

        b2Vec2 position = b2Body_GetPosition(body);
        b2Rot rotation = b2Body_GetRotation(body);
        float angle = b2Atan2(rotation.s, rotation.c);

        std::cout << "Step " << i << ": (" << position.x << ", " << position.y << "), angle: " << angle << std::endl;
    }

    b2DestroyWorld(world);
    return 0;
}