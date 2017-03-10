/**
 * @file misc/auto/collisioncases.c
 *
 * File generated from 'misc/collision.json' to simplify handling collisions
 *
 * DO NOT EDIT MANUALLY
 */

switch (MERGE_TYPES(node1.type, node2.type)) {
/* Collision group 'floor_collision' */ 
CASE(T_FLOOR, T_EN_SPIKY)
CASE(T_FLOOR, T_EN_WALKY)
CASE(T_FLOOR, T_EN_TURRET)
CASE(T_FLOOR, T_EN_G_WALKY)
CASE(T_FLOOR, T_GUNNY)
CASE(T_FLOOR, T_SWORDY)
CASE(T_FLOOR_NOTP, T_EN_SPIKY)
CASE(T_FLOOR_NOTP, T_EN_WALKY)
CASE(T_FLOOR_NOTP, T_EN_TURRET)
CASE(T_FLOOR_NOTP, T_EN_G_WALKY)
CASE(T_FLOOR_NOTP, T_GUNNY)
CASE(T_FLOOR_NOTP, T_SWORDY)
    if (isFirstCase) {
        erv = _defaultFloorCollision(&node1, &node2);
    }
    else {
        erv = _defaultFloorCollision(&node2, &node1);
    }
break;
CASE(T_FLOOR, T_EN_G_WALKY_ATK)
CASE(T_FLOOR_NOTP, T_EN_G_WALKY_ATK)
    if (isFirstCase) {
        erv = _floorProjectileCollision(&node1, &node2);
    }
    else {
        erv = _floorProjectileCollision(&node2, &node1);
    }
break;
/* Collision group 'loadzone_collision' */ 
CASE(T_LOADZONE, T_SWORDY)
CASE(T_LOADZONE, T_GUNNY)
CASE(T_LOADZONE, T_DUMMY_GUNNY)
CASE(T_LOADZONE, T_DUMMY_SWORDY)
    if (isFirstCase) {
        erv = _collideLoadzonePlayer(&node1, &node2);
    }
    else {
        erv = _collideLoadzonePlayer(&node2, &node1);
    }
break;
/* Collision group 'environmental_harm' */ 
CASE(T_SPIKE, T_EN_TURRET)
    if (isFirstCase) {
        erv = _defaultFloorCollision(&node1, &node2);
    }
    else {
        erv = _defaultFloorCollision(&node2, &node1);
    }
break;
CASE(T_SPIKE, T_EN_SPIKY)
CASE(T_SPIKE, T_EN_WALKY)
CASE(T_SPIKE, T_EN_G_WALKY)
CASE(T_SPIKE, T_GUNNY)
CASE(T_SPIKE, T_SWORDY)
    if (isFirstCase) {
        erv = _environmentalHarmEntity(&node1, &node2);
    }
    else {
        erv = _environmentalHarmEntity(&node2, &node1);
    }
break;
/* Collision group 'dummy_collision' */ 
CASE(T_SWORDY, T_DUMMY_GUNNY)
CASE(T_SWORDY, T_DUMMY_SWORDY)
CASE(T_GUNNY, T_DUMMY_GUNNY)
CASE(T_GUNNY, T_DUMMY_SWORDY)
    if (isFirstCase) {
        erv = _collidePlayerDummy(&node1, &node2);
    }
    else {
        erv = _collidePlayerDummy(&node2, &node1);
    }
break;
/* Collision group 'teleport_collision' */ 
CASE(T_TEL_BULLET, T_EN_TURRET)
CASE(T_TEL_BULLET, T_SPIKE)
CASE(T_TEL_BULLET, T_FLOOR_NOTP)
    if (isFirstCase) {
        erv = _ignoreTeleportBullet(&node1, &node2);
    }
    else {
        erv = _ignoreTeleportBullet(&node2, &node1);
    }
break;
CASE(T_TEL_BULLET, T_EN_SPIKY)
CASE(T_TEL_BULLET, T_EN_WALKY)
CASE(T_TEL_BULLET, T_SWORDY)
CASE(T_TEL_BULLET, T_EN_G_WALKY)
    if (isFirstCase) {
        erv = _setTeleportEntity(&node1, &node2);
    }
    else {
        erv = _setTeleportEntity(&node2, &node1);
    }
break;
CASE(T_TEL_BULLET, T_FLOOR)
    if (isFirstCase) {
        erv = _setTeleportFloor(&node1, &node2);
    }
    else {
        erv = _setTeleportFloor(&node2, &node1);
    }
break;
} /* switch (MERGE_TYPES(node1.type, node2.type)) */
ASSERT(erv == ERR_OK, erv);
