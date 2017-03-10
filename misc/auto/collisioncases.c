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
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _defaultFloorCollision(&node1, &node2);
    }
    else {
        erv = _defaultFloorCollision(&node2, &node1);
    }
break;
CASE(T_FLOOR, T_EN_G_WALKY_ATK)
CASE(T_FLOOR_NOTP, T_EN_G_WALKY_ATK)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _floorProjectileCollision(&node1, &node2);
    }
    else {
        erv = _floorProjectileCollision(&node2, &node1);
    }
break;
/* Collision group 'common_attacks' */ 
CASE(T_EN_G_WALKY_ATK, T_SWORDY)
CASE(T_EN_G_WALKY_ATK, T_GUNNY)
CASE(T_EN_G_WALKY_ATK, T_EN_SPIKY)
CASE(T_EN_G_WALKY_ATK, T_EN_WALKY)
CASE(T_EN_G_WALKY_ATK, T_EN_G_WALKY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _attackEntity(&node1, &node2);
    }
    else {
        erv = _attackEntity(&node2, &node1);
    }
break;
CASE(T_EN_G_WALKY_ATK, T_EN_TURRET)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _denyProjectile(&node1, &node2);
    }
    else {
        erv = _denyProjectile(&node2, &node1);
    }
break;
/* Collision group 'sword_attack' */ 
CASE(T_ATK_SWORD, T_EN_G_WALKY_ATK)
CASE(T_ATK_SWORD, T_TEL_BULLET)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _swordReflectProjectile(&node1, &node2);
    }
    else {
        erv = _swordReflectProjectile(&node2, &node1);
    }
break;
CASE(T_ATK_SWORD, T_EN_SPIKY)
CASE(T_ATK_SWORD, T_EN_WALKY)
CASE(T_ATK_SWORD, T_EN_G_WALKY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _attackEntity(&node1, &node2);
    }
    else {
        erv = _attackEntity(&node2, &node1);
    }
break;
/* Collision group 'environmental_harm' */ 
CASE(T_SPIKE, T_EN_TURRET)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
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
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _environmentalHarmEntity(&node1, &node2);
    }
    else {
        erv = _environmentalHarmEntity(&node2, &node1);
    }
break;
/* Collision group 'entity_collision' */ 
SELFCASE(T_EN_SPIKY)
CASE(T_EN_SPIKY, T_EN_WALKY)
CASE(T_EN_SPIKY, T_EN_G_WALKY)
CASE(T_EN_SPIKY, T_EN_TURRET)
CASE(T_EN_SPIKY, T_SWORDY)
CASE(T_EN_SPIKY, T_GUNNY)
SELFCASE(T_EN_WALKY)
CASE(T_EN_WALKY, T_EN_G_WALKY)
CASE(T_EN_WALKY, T_EN_TURRET)
CASE(T_EN_WALKY, T_SWORDY)
CASE(T_EN_WALKY, T_GUNNY)
SELFCASE(T_EN_G_WALKY)
CASE(T_EN_G_WALKY, T_EN_TURRET)
CASE(T_EN_G_WALKY, T_SWORDY)
CASE(T_EN_G_WALKY, T_GUNNY)
SELFCASE(T_EN_TURRET)
CASE(T_EN_TURRET, T_SWORDY)
CASE(T_EN_TURRET, T_GUNNY)
SELFCASE(T_SWORDY)
CASE(T_SWORDY, T_GUNNY)
SELFCASE(T_GUNNY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _entityCollision(&node1, &node2);
    }
    else {
        erv = _entityCollision(&node2, &node1);
    }
break;
/* Collision group 'dummy_collision' */ 
CASE(T_SWORDY, T_DUMMY_GUNNY)
CASE(T_SWORDY, T_DUMMY_SWORDY)
CASE(T_GUNNY, T_DUMMY_GUNNY)
CASE(T_GUNNY, T_DUMMY_SWORDY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _collidePlayerDummy(&node1, &node2);
    }
    else {
        erv = _collidePlayerDummy(&node2, &node1);
    }
break;
/* Collision group 'checkpoint_collision' */ 
CASE(T_CHECKPOINT, T_GUNNY)
CASE(T_CHECKPOINT, T_SWORDY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _checkpointCollision(&node1, &node2);
    }
    else {
        erv = _checkpointCollision(&node2, &node1);
    }
break;
/* Collision group 'teleport_collision' */ 
CASE(T_TEL_BULLET, T_EN_TURRET)
CASE(T_TEL_BULLET, T_SPIKE)
CASE(T_TEL_BULLET, T_FLOOR_NOTP)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
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
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _setTeleportEntity(&node1, &node2);
    }
    else {
        erv = _setTeleportEntity(&node2, &node1);
    }
break;
CASE(T_TEL_BULLET, T_FLOOR)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _setTeleportFloor(&node1, &node2);
    }
    else {
        erv = _setTeleportFloor(&node2, &node1);
    }
break;
/* Collision group 'loadzone_collision' */ 
CASE(T_LOADZONE, T_SWORDY)
CASE(T_LOADZONE, T_GUNNY)
CASE(T_LOADZONE, T_DUMMY_GUNNY)
CASE(T_LOADZONE, T_DUMMY_SWORDY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _collideLoadzonePlayer(&node1, &node2);
    }
    else {
        erv = _collideLoadzonePlayer(&node2, &node1);
    }
break;
/* Collision group 'gwalky_view' */ 
CASE(T_EN_G_WALKY_VIEW, T_EN_SPIKY)
CASE(T_EN_G_WALKY_VIEW, T_EN_WALKY)
CASE(T_EN_G_WALKY_VIEW, T_EN_TURRET)
CASE(T_EN_G_WALKY_VIEW, T_SWORDY)
CASE(T_EN_G_WALKY_VIEW, T_GUNNY)
    if (node1.pChild == node2.pChild) {
        /* Filter out self collision */
    }
    else if (isFirstCase) {
        erv = _gWalkyViewEntityCollision(&node1, &node2);
    }
    else {
        erv = _gWalkyViewEntityCollision(&node2, &node1);
    }
break;
} /* switch (MERGE_TYPES(node1.type, node2.type)) */
ASSERT(erv == ERR_OK, erv);
