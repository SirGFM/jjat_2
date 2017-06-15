/**
 * @file misc/auto/collisioncases.c
 *
 * File generated from 'misc/collision.json' to simplify handling collisions
 *
 * DO NOT EDIT MANUALLY
 */

/* Avoid error if it's a self collision */erv = ERR_OK;
/* Merge both types into a single one, so it's easier to compare */
switch (MERGE_TYPES(node1.type, node2.type)) {
    /* Collision group 'floor_collision' */ 
    CASE(T_BLUE_PLATFORM, T_EN_SPIKY)
    CASE(T_BLUE_PLATFORM, T_EN_WALKY)
    CASE(T_BLUE_PLATFORM, T_EN_TURRET)
    CASE(T_BLUE_PLATFORM, T_EN_G_WALKY)
    CASE(T_BLUE_PLATFORM, T_GUNNY)
    CASE(T_BLUE_PLATFORM, T_SWORDY)
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
    CASE(T_DOOR, T_EN_SPIKY)
    CASE(T_DOOR, T_EN_WALKY)
    CASE(T_DOOR, T_EN_TURRET)
    CASE(T_DOOR, T_EN_G_WALKY)
    CASE(T_DOOR, T_GUNNY)
    CASE(T_DOOR, T_SWORDY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _defaultFloorCollision(&node1, &node2);
        }
    break;
    CASE(T_EN_SPIKY, T_BLUE_PLATFORM)
    CASE(T_EN_SPIKY, T_FLOOR)
    CASE(T_EN_SPIKY, T_FLOOR_NOTP)
    CASE(T_EN_SPIKY, T_DOOR)
    CASE(T_EN_WALKY, T_BLUE_PLATFORM)
    CASE(T_EN_WALKY, T_FLOOR)
    CASE(T_EN_WALKY, T_FLOOR_NOTP)
    CASE(T_EN_WALKY, T_DOOR)
    CASE(T_EN_TURRET, T_BLUE_PLATFORM)
    CASE(T_EN_TURRET, T_FLOOR)
    CASE(T_EN_TURRET, T_FLOOR_NOTP)
    CASE(T_EN_TURRET, T_DOOR)
    CASE(T_EN_G_WALKY, T_BLUE_PLATFORM)
    CASE(T_EN_G_WALKY, T_FLOOR)
    CASE(T_EN_G_WALKY, T_FLOOR_NOTP)
    CASE(T_EN_G_WALKY, T_DOOR)
    CASE(T_GUNNY, T_BLUE_PLATFORM)
    CASE(T_GUNNY, T_FLOOR)
    CASE(T_GUNNY, T_FLOOR_NOTP)
    CASE(T_GUNNY, T_DOOR)
    CASE(T_SWORDY, T_BLUE_PLATFORM)
    CASE(T_SWORDY, T_FLOOR)
    CASE(T_SWORDY, T_FLOOR_NOTP)
    CASE(T_SWORDY, T_DOOR)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _defaultFloorCollision(&node2, &node1);
        }
    break;
    CASE(T_BLUE_PLATFORM, T_EN_G_WALKY_ATK)
    CASE(T_FLOOR, T_EN_G_WALKY_ATK)
    CASE(T_FLOOR_NOTP, T_EN_G_WALKY_ATK)
    CASE(T_DOOR, T_EN_G_WALKY_ATK)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _floorProjectileCollision(&node1, &node2);
        }
    break;
    CASE(T_EN_G_WALKY_ATK, T_BLUE_PLATFORM)
    CASE(T_EN_G_WALKY_ATK, T_FLOOR)
    CASE(T_EN_G_WALKY_ATK, T_FLOOR_NOTP)
    CASE(T_EN_G_WALKY_ATK, T_DOOR)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _floorProjectileCollision(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_BLUE_PLATFORM, T_ATK_SWORD)
    IGNORE(T_BLUE_PLATFORM, T_CHECKPOINT)
    IGNORE(T_BLUE_PLATFORM, T_LOADZONE)
    IGNORE(T_BLUE_PLATFORM, T_DUMMY_SWORDY)
    IGNORE(T_BLUE_PLATFORM, T_DUMMY_GUNNY)
    IGNORE(T_BLUE_PLATFORM, T_SPIKE)
    SELFCASE(T_BLUE_PLATFORM)
    IGNORE(T_BLUE_PLATFORM, T_FLOOR)
    IGNORE(T_BLUE_PLATFORM, T_FLOOR_NOTP)
    IGNORE(T_BLUE_PLATFORM, T_EN_G_WALKY_VIEW)
    IGNORE(T_BLUE_PLATFORM, T_PRESSURE_PAD)
    IGNORE(T_FLOOR, T_ATK_SWORD)
    IGNORE(T_FLOOR, T_CHECKPOINT)
    IGNORE(T_FLOOR, T_LOADZONE)
    IGNORE(T_FLOOR, T_DUMMY_SWORDY)
    IGNORE(T_FLOOR, T_DUMMY_GUNNY)
    IGNORE(T_FLOOR, T_SPIKE)
    IGNORE(T_FLOOR, T_BLUE_PLATFORM)
    SELFCASE(T_FLOOR)
    IGNORE(T_FLOOR, T_FLOOR_NOTP)
    IGNORE(T_FLOOR, T_EN_G_WALKY_VIEW)
    IGNORE(T_FLOOR, T_PRESSURE_PAD)
    IGNORE(T_FLOOR_NOTP, T_ATK_SWORD)
    IGNORE(T_FLOOR_NOTP, T_CHECKPOINT)
    IGNORE(T_FLOOR_NOTP, T_LOADZONE)
    IGNORE(T_FLOOR_NOTP, T_DUMMY_SWORDY)
    IGNORE(T_FLOOR_NOTP, T_DUMMY_GUNNY)
    IGNORE(T_FLOOR_NOTP, T_SPIKE)
    IGNORE(T_FLOOR_NOTP, T_BLUE_PLATFORM)
    IGNORE(T_FLOOR_NOTP, T_FLOOR)
    SELFCASE(T_FLOOR_NOTP)
    IGNORE(T_FLOOR_NOTP, T_EN_G_WALKY_VIEW)
    IGNORE(T_FLOOR_NOTP, T_PRESSURE_PAD)
    IGNORE(T_DOOR, T_ATK_SWORD)
    IGNORE(T_DOOR, T_CHECKPOINT)
    IGNORE(T_DOOR, T_LOADZONE)
    IGNORE(T_DOOR, T_DUMMY_SWORDY)
    IGNORE(T_DOOR, T_DUMMY_GUNNY)
    IGNORE(T_DOOR, T_SPIKE)
    IGNORE(T_DOOR, T_BLUE_PLATFORM)
    IGNORE(T_DOOR, T_FLOOR)
    IGNORE(T_DOOR, T_FLOOR_NOTP)
    IGNORE(T_DOOR, T_EN_G_WALKY_VIEW)
    IGNORE(T_DOOR, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'common_attacks' */ 
    CASE(T_EN_G_WALKY_ATK, T_SWORDY)
    CASE(T_EN_G_WALKY_ATK, T_GUNNY)
    CASE(T_EN_G_WALKY_ATK, T_EN_SPIKY)
    CASE(T_EN_G_WALKY_ATK, T_EN_WALKY)
    CASE(T_EN_G_WALKY_ATK, T_EN_G_WALKY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _attackEntity(&node1, &node2);
        }
    break;
    CASE(T_SWORDY, T_EN_G_WALKY_ATK)
    CASE(T_GUNNY, T_EN_G_WALKY_ATK)
    CASE(T_EN_SPIKY, T_EN_G_WALKY_ATK)
    CASE(T_EN_WALKY, T_EN_G_WALKY_ATK)
    CASE(T_EN_G_WALKY, T_EN_G_WALKY_ATK)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _attackEntity(&node2, &node1);
        }
    break;
    CASE(T_EN_G_WALKY_ATK, T_EN_TURRET)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _denyProjectile(&node1, &node2);
        }
    break;
    CASE(T_EN_TURRET, T_EN_G_WALKY_ATK)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _denyProjectile(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_EN_G_WALKY_ATK, T_LOADZONE)
    IGNORE(T_EN_G_WALKY_ATK, T_TEL_BULLET)
    IGNORE(T_EN_G_WALKY_ATK, T_CHECKPOINT)
    IGNORE(T_EN_G_WALKY_ATK, T_DUMMY_SWORDY)
    IGNORE(T_EN_G_WALKY_ATK, T_DUMMY_GUNNY)
    SELFCASE(T_EN_G_WALKY_ATK)
    IGNORE(T_EN_G_WALKY_ATK, T_EN_G_WALKY_VIEW)
    IGNORE(T_EN_G_WALKY_ATK, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'collision_less' */ 
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_SWORD_FX, T_EN_SPIKY)
    IGNORE(T_SWORD_FX, T_EN_WALKY)
    IGNORE(T_SWORD_FX, T_EN_G_WALKY)
    IGNORE(T_SWORD_FX, T_EN_G_WALKY_ATK)
    IGNORE(T_SWORD_FX, T_EN_G_WALKY_VIEW)
    IGNORE(T_SWORD_FX, T_EN_TURRET)
    IGNORE(T_SWORD_FX, T_SWORDY)
    IGNORE(T_SWORD_FX, T_GUNNY)
    IGNORE(T_SWORD_FX, T_FLOOR)
    IGNORE(T_SWORD_FX, T_FLOOR_NOTP)
    IGNORE(T_SWORD_FX, T_SPIKE)
    IGNORE(T_SWORD_FX, T_ATK_SWORD)
    IGNORE(T_SWORD_FX, T_TEL_BULLET)
    IGNORE(T_SWORD_FX, T_LOADZONE)
    IGNORE(T_SWORD_FX, T_BLUE_PLATFORM)
    IGNORE(T_SWORD_FX, T_DUMMY_GUNNY)
    IGNORE(T_SWORD_FX, T_DUMMY_SWORDY)
    IGNORE(T_SWORD_FX, T_CHECKPOINT)
    IGNORE(T_SWORD_FX, T_DOOR)
    IGNORE(T_SWORD_FX, T_PRESSURE_PAD)
    IGNORE(T_FX, T_EN_SPIKY)
    IGNORE(T_FX, T_EN_WALKY)
    IGNORE(T_FX, T_EN_G_WALKY)
    IGNORE(T_FX, T_EN_G_WALKY_ATK)
    IGNORE(T_FX, T_EN_G_WALKY_VIEW)
    IGNORE(T_FX, T_EN_TURRET)
    IGNORE(T_FX, T_SWORDY)
    IGNORE(T_FX, T_GUNNY)
    IGNORE(T_FX, T_FLOOR)
    IGNORE(T_FX, T_FLOOR_NOTP)
    IGNORE(T_FX, T_SPIKE)
    IGNORE(T_FX, T_ATK_SWORD)
    IGNORE(T_FX, T_TEL_BULLET)
    IGNORE(T_FX, T_LOADZONE)
    IGNORE(T_FX, T_BLUE_PLATFORM)
    IGNORE(T_FX, T_DUMMY_GUNNY)
    IGNORE(T_FX, T_DUMMY_SWORDY)
    IGNORE(T_FX, T_CHECKPOINT)
    IGNORE(T_FX, T_DOOR)
    IGNORE(T_FX, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_EN_SPIKY, T_SWORD_FX)
    IGNORE(T_EN_SPIKY, T_FX)
    IGNORE(T_EN_WALKY, T_SWORD_FX)
    IGNORE(T_EN_WALKY, T_FX)
    IGNORE(T_EN_G_WALKY, T_SWORD_FX)
    IGNORE(T_EN_G_WALKY, T_FX)
    IGNORE(T_EN_G_WALKY_ATK, T_SWORD_FX)
    IGNORE(T_EN_G_WALKY_ATK, T_FX)
    IGNORE(T_EN_G_WALKY_VIEW, T_SWORD_FX)
    IGNORE(T_EN_G_WALKY_VIEW, T_FX)
    IGNORE(T_EN_TURRET, T_SWORD_FX)
    IGNORE(T_EN_TURRET, T_FX)
    IGNORE(T_SWORDY, T_SWORD_FX)
    IGNORE(T_SWORDY, T_FX)
    IGNORE(T_GUNNY, T_SWORD_FX)
    IGNORE(T_GUNNY, T_FX)
    IGNORE(T_FLOOR, T_SWORD_FX)
    IGNORE(T_FLOOR, T_FX)
    IGNORE(T_FLOOR_NOTP, T_SWORD_FX)
    IGNORE(T_FLOOR_NOTP, T_FX)
    IGNORE(T_SPIKE, T_SWORD_FX)
    IGNORE(T_SPIKE, T_FX)
    IGNORE(T_ATK_SWORD, T_SWORD_FX)
    IGNORE(T_ATK_SWORD, T_FX)
    IGNORE(T_TEL_BULLET, T_SWORD_FX)
    IGNORE(T_TEL_BULLET, T_FX)
    IGNORE(T_LOADZONE, T_SWORD_FX)
    IGNORE(T_LOADZONE, T_FX)
    IGNORE(T_BLUE_PLATFORM, T_SWORD_FX)
    IGNORE(T_BLUE_PLATFORM, T_FX)
    IGNORE(T_DUMMY_GUNNY, T_SWORD_FX)
    IGNORE(T_DUMMY_GUNNY, T_FX)
    IGNORE(T_DUMMY_SWORDY, T_SWORD_FX)
    IGNORE(T_DUMMY_SWORDY, T_FX)
    IGNORE(T_CHECKPOINT, T_SWORD_FX)
    IGNORE(T_CHECKPOINT, T_FX)
    IGNORE(T_DOOR, T_SWORD_FX)
    IGNORE(T_DOOR, T_FX)
    IGNORE(T_PRESSURE_PAD, T_SWORD_FX)
    IGNORE(T_PRESSURE_PAD, T_FX)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    SELFCASE(T_SWORD_FX)
    IGNORE(T_SWORD_FX, T_FX)
    IGNORE(T_FX, T_SWORD_FX)
    SELFCASE(T_FX)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'sword_attack' */ 
    CASE(T_ATK_SWORD, T_EN_G_WALKY_ATK)
    CASE(T_ATK_SWORD, T_TEL_BULLET)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _swordReflectProjectile(&node1, &node2);
        }
    break;
    CASE(T_EN_G_WALKY_ATK, T_ATK_SWORD)
    CASE(T_TEL_BULLET, T_ATK_SWORD)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _swordReflectProjectile(&node2, &node1);
        }
    break;
    CASE(T_ATK_SWORD, T_EN_SPIKY)
    CASE(T_ATK_SWORD, T_EN_WALKY)
    CASE(T_ATK_SWORD, T_EN_G_WALKY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _attackEntity(&node1, &node2);
        }
    break;
    CASE(T_EN_SPIKY, T_ATK_SWORD)
    CASE(T_EN_WALKY, T_ATK_SWORD)
    CASE(T_EN_G_WALKY, T_ATK_SWORD)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _attackEntity(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_ATK_SWORD, T_EN_TURRET)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_EN_TURRET, T_ATK_SWORD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_ATK_SWORD, T_SWORDY)
    IGNORE(T_ATK_SWORD, T_GUNNY)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_SWORDY, T_ATK_SWORD)
    IGNORE(T_GUNNY, T_ATK_SWORD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    SELFCASE(T_ATK_SWORD)
    IGNORE(T_ATK_SWORD, T_CHECKPOINT)
    IGNORE(T_ATK_SWORD, T_FLOOR)
    IGNORE(T_ATK_SWORD, T_FLOOR_NOTP)
    IGNORE(T_ATK_SWORD, T_SPIKE)
    IGNORE(T_ATK_SWORD, T_LOADZONE)
    IGNORE(T_ATK_SWORD, T_EN_G_WALKY_VIEW)
    IGNORE(T_ATK_SWORD, T_BLUE_PLATFORM)
    IGNORE(T_ATK_SWORD, T_DUMMY_SWORDY)
    IGNORE(T_ATK_SWORD, T_DUMMY_GUNNY)
    IGNORE(T_ATK_SWORD, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'environmental_harm' */ 
    CASE(T_SPIKE, T_EN_TURRET)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _defaultFloorCollision(&node1, &node2);
        }
    break;
    CASE(T_EN_TURRET, T_SPIKE)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _defaultFloorCollision(&node2, &node1);
        }
    break;
    CASE(T_SPIKE, T_EN_SPIKY)
    CASE(T_SPIKE, T_EN_WALKY)
    CASE(T_SPIKE, T_EN_G_WALKY)
    CASE(T_SPIKE, T_GUNNY)
    CASE(T_SPIKE, T_SWORDY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _environmentalHarmEntity(&node1, &node2);
        }
    break;
    CASE(T_EN_SPIKY, T_SPIKE)
    CASE(T_EN_WALKY, T_SPIKE)
    CASE(T_EN_G_WALKY, T_SPIKE)
    CASE(T_GUNNY, T_SPIKE)
    CASE(T_SWORDY, T_SPIKE)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _environmentalHarmEntity(&node2, &node1);
        }
    break;
    CASE(T_SPIKE, T_EN_G_WALKY_ATK)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _floorProjectileCollision(&node1, &node2);
        }
    break;
    CASE(T_EN_G_WALKY_ATK, T_SPIKE)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _floorProjectileCollision(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_SPIKE, T_FLOOR)
    IGNORE(T_SPIKE, T_FLOOR_NOTP)
    IGNORE(T_SPIKE, T_LOADZONE)
    IGNORE(T_SPIKE, T_BLUE_PLATFORM)
    IGNORE(T_SPIKE, T_ATK_SWORD)
    IGNORE(T_SPIKE, T_CHECKPOINT)
    IGNORE(T_SPIKE, T_DUMMY_SWORDY)
    IGNORE(T_SPIKE, T_DUMMY_GUNNY)
    SELFCASE(T_SPIKE)
    IGNORE(T_SPIKE, T_EN_G_WALKY_VIEW)
    IGNORE(T_SPIKE, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'entity_collision' */ 
    SELFCASE(T_EN_SPIKY)
    CASE(T_EN_SPIKY, T_EN_WALKY)
    CASE(T_EN_SPIKY, T_EN_G_WALKY)
    CASE(T_EN_SPIKY, T_EN_TURRET)
    CASE(T_EN_SPIKY, T_SWORDY)
    CASE(T_EN_SPIKY, T_GUNNY)
    CASE(T_EN_WALKY, T_EN_SPIKY)
    SELFCASE(T_EN_WALKY)
    CASE(T_EN_WALKY, T_EN_G_WALKY)
    CASE(T_EN_WALKY, T_EN_TURRET)
    CASE(T_EN_WALKY, T_SWORDY)
    CASE(T_EN_WALKY, T_GUNNY)
    CASE(T_EN_G_WALKY, T_EN_SPIKY)
    CASE(T_EN_G_WALKY, T_EN_WALKY)
    SELFCASE(T_EN_G_WALKY)
    CASE(T_EN_G_WALKY, T_EN_TURRET)
    CASE(T_EN_G_WALKY, T_SWORDY)
    CASE(T_EN_G_WALKY, T_GUNNY)
    CASE(T_EN_TURRET, T_EN_SPIKY)
    CASE(T_EN_TURRET, T_EN_WALKY)
    CASE(T_EN_TURRET, T_EN_G_WALKY)
    SELFCASE(T_EN_TURRET)
    CASE(T_EN_TURRET, T_SWORDY)
    CASE(T_EN_TURRET, T_GUNNY)
    CASE(T_SWORDY, T_EN_SPIKY)
    CASE(T_SWORDY, T_EN_WALKY)
    CASE(T_SWORDY, T_EN_G_WALKY)
    CASE(T_SWORDY, T_EN_TURRET)
    SELFCASE(T_SWORDY)
    CASE(T_SWORDY, T_GUNNY)
    CASE(T_GUNNY, T_EN_SPIKY)
    CASE(T_GUNNY, T_EN_WALKY)
    CASE(T_GUNNY, T_EN_G_WALKY)
    CASE(T_GUNNY, T_EN_TURRET)
    CASE(T_GUNNY, T_SWORDY)
    SELFCASE(T_GUNNY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _entityCollision(&node1, &node2);
        }
    break;
    CASE(T_EN_SPIKY, T_PRESSURE_PAD)
    CASE(T_EN_WALKY, T_PRESSURE_PAD)
    CASE(T_EN_G_WALKY, T_PRESSURE_PAD)
    CASE(T_EN_TURRET, T_PRESSURE_PAD)
    CASE(T_SWORDY, T_PRESSURE_PAD)
    CASE(T_GUNNY, T_PRESSURE_PAD)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _onPressurePad(&node1, &node2);
        }
    break;
    CASE(T_EN_SPIKY, T_EN_G_WALKY_VIEW)
    CASE(T_EN_WALKY, T_EN_G_WALKY_VIEW)
    CASE(T_EN_G_WALKY, T_EN_G_WALKY_VIEW)
    CASE(T_EN_TURRET, T_EN_G_WALKY_VIEW)
    CASE(T_SWORDY, T_EN_G_WALKY_VIEW)
    CASE(T_GUNNY, T_EN_G_WALKY_VIEW)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _gWalkyViewEntityCollision(&node1, &node2);
        }
    break;
    CASE(T_EN_G_WALKY_VIEW, T_EN_SPIKY)
    CASE(T_EN_G_WALKY_VIEW, T_EN_WALKY)
    CASE(T_EN_G_WALKY_VIEW, T_EN_G_WALKY)
    CASE(T_EN_G_WALKY_VIEW, T_EN_TURRET)
    CASE(T_EN_G_WALKY_VIEW, T_SWORDY)
    CASE(T_EN_G_WALKY_VIEW, T_GUNNY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _gWalkyViewEntityCollision(&node2, &node1);
        }
    break;
    /* Collision group 'dummy_collision' */ 
    CASE(T_DUMMY_GUNNY, T_SWORDY)
    CASE(T_DUMMY_GUNNY, T_GUNNY)
    CASE(T_DUMMY_SWORDY, T_SWORDY)
    CASE(T_DUMMY_SWORDY, T_GUNNY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _collidePlayerDummy(&node1, &node2);
        }
    break;
    CASE(T_SWORDY, T_DUMMY_GUNNY)
    CASE(T_SWORDY, T_DUMMY_SWORDY)
    CASE(T_GUNNY, T_DUMMY_GUNNY)
    CASE(T_GUNNY, T_DUMMY_SWORDY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _collidePlayerDummy(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_DUMMY_GUNNY, T_DUMMY_SWORDY)
    SELFCASE(T_DUMMY_GUNNY)
    IGNORE(T_DUMMY_GUNNY, T_HAZARD)
    IGNORE(T_DUMMY_GUNNY, T_PLAYER)
    IGNORE(T_DUMMY_GUNNY, T_FLOOR)
    IGNORE(T_DUMMY_GUNNY, T_ENEMY)
    IGNORE(T_DUMMY_GUNNY, T_FLOOR_NOTP)
    IGNORE(T_DUMMY_GUNNY, T_ATK_SWORD)
    IGNORE(T_DUMMY_GUNNY, T_TEL_BULLET)
    IGNORE(T_DUMMY_GUNNY, T_CHECKPOINT)
    IGNORE(T_DUMMY_GUNNY, T_SPIKE)
    IGNORE(T_DUMMY_GUNNY, T_EN_WALKY)
    IGNORE(T_DUMMY_GUNNY, T_EN_G_WALKY)
    IGNORE(T_DUMMY_GUNNY, T_EN_G_WALKY_ATK)
    IGNORE(T_DUMMY_GUNNY, T_EN_G_WALKY_VIEW)
    IGNORE(T_DUMMY_GUNNY, T_EN_SPIKY)
    IGNORE(T_DUMMY_GUNNY, T_EN_TURRET)
    IGNORE(T_DUMMY_GUNNY, T_BLUE_PLATFORM)
    IGNORE(T_DUMMY_GUNNY, T_PRESSURE_PAD)
    SELFCASE(T_DUMMY_SWORDY)
    IGNORE(T_DUMMY_SWORDY, T_DUMMY_GUNNY)
    IGNORE(T_DUMMY_SWORDY, T_HAZARD)
    IGNORE(T_DUMMY_SWORDY, T_PLAYER)
    IGNORE(T_DUMMY_SWORDY, T_FLOOR)
    IGNORE(T_DUMMY_SWORDY, T_ENEMY)
    IGNORE(T_DUMMY_SWORDY, T_FLOOR_NOTP)
    IGNORE(T_DUMMY_SWORDY, T_ATK_SWORD)
    IGNORE(T_DUMMY_SWORDY, T_TEL_BULLET)
    IGNORE(T_DUMMY_SWORDY, T_CHECKPOINT)
    IGNORE(T_DUMMY_SWORDY, T_SPIKE)
    IGNORE(T_DUMMY_SWORDY, T_EN_WALKY)
    IGNORE(T_DUMMY_SWORDY, T_EN_G_WALKY)
    IGNORE(T_DUMMY_SWORDY, T_EN_G_WALKY_ATK)
    IGNORE(T_DUMMY_SWORDY, T_EN_G_WALKY_VIEW)
    IGNORE(T_DUMMY_SWORDY, T_EN_SPIKY)
    IGNORE(T_DUMMY_SWORDY, T_EN_TURRET)
    IGNORE(T_DUMMY_SWORDY, T_BLUE_PLATFORM)
    IGNORE(T_DUMMY_SWORDY, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'g_walky_view' */ 
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_EN_G_WALKY_VIEW, T_FLOOR)
    IGNORE(T_EN_G_WALKY_VIEW, T_FLOOR_NOTP)
    IGNORE(T_EN_G_WALKY_VIEW, T_LOADZONE)
    IGNORE(T_EN_G_WALKY_VIEW, T_BLUE_PLATFORM)
    IGNORE(T_EN_G_WALKY_VIEW, T_ATK_SWORD)
    IGNORE(T_EN_G_WALKY_VIEW, T_TEL_BULLET)
    IGNORE(T_EN_G_WALKY_VIEW, T_CHECKPOINT)
    IGNORE(T_EN_G_WALKY_VIEW, T_DUMMY_SWORDY)
    IGNORE(T_EN_G_WALKY_VIEW, T_DUMMY_GUNNY)
    IGNORE(T_EN_G_WALKY_VIEW, T_SPIKE)
    IGNORE(T_EN_G_WALKY_VIEW, T_EN_G_WALKY_ATK)
    SELFCASE(T_EN_G_WALKY_VIEW)
    IGNORE(T_EN_G_WALKY_VIEW, T_DOOR)
    IGNORE(T_EN_G_WALKY_VIEW, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'checkpoint_collision' */ 
    CASE(T_CHECKPOINT, T_GUNNY)
    CASE(T_CHECKPOINT, T_SWORDY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _checkpointCollision(&node1, &node2);
        }
    break;
    CASE(T_GUNNY, T_CHECKPOINT)
    CASE(T_SWORDY, T_CHECKPOINT)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _checkpointCollision(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_CHECKPOINT, T_FLOOR)
    IGNORE(T_CHECKPOINT, T_FLOOR_NOTP)
    IGNORE(T_CHECKPOINT, T_LOADZONE)
    IGNORE(T_CHECKPOINT, T_BLUE_PLATFORM)
    IGNORE(T_CHECKPOINT, T_ATK_SWORD)
    IGNORE(T_CHECKPOINT, T_TEL_BULLET)
    SELFCASE(T_CHECKPOINT)
    IGNORE(T_CHECKPOINT, T_DUMMY_SWORDY)
    IGNORE(T_CHECKPOINT, T_DUMMY_GUNNY)
    IGNORE(T_CHECKPOINT, T_SPIKE)
    IGNORE(T_CHECKPOINT, T_EN_WALKY)
    IGNORE(T_CHECKPOINT, T_EN_G_WALKY)
    IGNORE(T_CHECKPOINT, T_EN_G_WALKY_ATK)
    IGNORE(T_CHECKPOINT, T_EN_G_WALKY_VIEW)
    IGNORE(T_CHECKPOINT, T_EN_SPIKY)
    IGNORE(T_CHECKPOINT, T_EN_TURRET)
    IGNORE(T_CHECKPOINT, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'teleport_collision' */ 
    CASE(T_TEL_BULLET, T_EN_TURRET)
    CASE(T_TEL_BULLET, T_SPIKE)
    CASE(T_TEL_BULLET, T_FLOOR_NOTP)
    CASE(T_TEL_BULLET, T_DOOR)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _ignoreTeleportBullet(&node1, &node2);
        }
    break;
    CASE(T_EN_TURRET, T_TEL_BULLET)
    CASE(T_SPIKE, T_TEL_BULLET)
    CASE(T_FLOOR_NOTP, T_TEL_BULLET)
    CASE(T_DOOR, T_TEL_BULLET)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _ignoreTeleportBullet(&node2, &node1);
        }
    break;
    CASE(T_TEL_BULLET, T_EN_SPIKY)
    CASE(T_TEL_BULLET, T_EN_WALKY)
    CASE(T_TEL_BULLET, T_SWORDY)
    CASE(T_TEL_BULLET, T_EN_G_WALKY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _setTeleportEntity(&node1, &node2);
        }
    break;
    CASE(T_EN_SPIKY, T_TEL_BULLET)
    CASE(T_EN_WALKY, T_TEL_BULLET)
    CASE(T_SWORDY, T_TEL_BULLET)
    CASE(T_EN_G_WALKY, T_TEL_BULLET)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _setTeleportEntity(&node2, &node1);
        }
    break;
    CASE(T_TEL_BULLET, T_BLUE_PLATFORM)
    CASE(T_TEL_BULLET, T_FLOOR)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _setTeleportFloor(&node1, &node2);
        }
    break;
    CASE(T_BLUE_PLATFORM, T_TEL_BULLET)
    CASE(T_FLOOR, T_TEL_BULLET)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _setTeleportFloor(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    SELFCASE(T_TEL_BULLET)
    IGNORE(T_TEL_BULLET, T_EN_G_WALKY_ATK)
    IGNORE(T_TEL_BULLET, T_CHECKPOINT)
    IGNORE(T_TEL_BULLET, T_LOADZONE)
    IGNORE(T_TEL_BULLET, T_EN_G_WALKY_VIEW)
    IGNORE(T_TEL_BULLET, T_DUMMY_SWORDY)
    IGNORE(T_TEL_BULLET, T_DUMMY_GUNNY)
    IGNORE(T_TEL_BULLET, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_TEL_BULLET, T_GUNNY)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_GUNNY, T_TEL_BULLET)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'loadzone_collision' */ 
    CASE(T_LOADZONE, T_SWORDY)
    CASE(T_LOADZONE, T_GUNNY)
    CASE(T_LOADZONE, T_DUMMY_GUNNY)
    CASE(T_LOADZONE, T_DUMMY_SWORDY)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _collideLoadzonePlayer(&node1, &node2);
        }
    break;
    CASE(T_SWORDY, T_LOADZONE)
    CASE(T_GUNNY, T_LOADZONE)
    CASE(T_DUMMY_GUNNY, T_LOADZONE)
    CASE(T_DUMMY_SWORDY, T_LOADZONE)
        if (node1.pChild != node2.pChild) {
            /* Filter out self collision */
            erv = _collideLoadzonePlayer(&node2, &node1);
        }
    break;
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_LOADZONE, T_FLOOR)
    IGNORE(T_LOADZONE, T_FLOOR_NOTP)
    SELFCASE(T_LOADZONE)
    IGNORE(T_LOADZONE, T_BLUE_PLATFORM)
    IGNORE(T_LOADZONE, T_ATK_SWORD)
    IGNORE(T_LOADZONE, T_TEL_BULLET)
    IGNORE(T_LOADZONE, T_CHECKPOINT)
    IGNORE(T_LOADZONE, T_SPIKE)
    IGNORE(T_LOADZONE, T_EN_WALKY)
    IGNORE(T_LOADZONE, T_EN_G_WALKY)
    IGNORE(T_LOADZONE, T_EN_G_WALKY_ATK)
    IGNORE(T_LOADZONE, T_EN_G_WALKY_VIEW)
    IGNORE(T_LOADZONE, T_EN_SPIKY)
    IGNORE(T_LOADZONE, T_EN_TURRET)
    IGNORE(T_LOADZONE, T_PRESSURE_PAD)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* Collision group 'non_player_entity_collision' */ 
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
    IGNORE(T_EN_SPIKY, T_LOADZONE)
    IGNORE(T_EN_SPIKY, T_CHECKPOINT)
    IGNORE(T_EN_SPIKY, T_DUMMY_SWORDY)
    IGNORE(T_EN_SPIKY, T_DUMMY_GUNNY)
    IGNORE(T_EN_WALKY, T_LOADZONE)
    IGNORE(T_EN_WALKY, T_CHECKPOINT)
    IGNORE(T_EN_WALKY, T_DUMMY_SWORDY)
    IGNORE(T_EN_WALKY, T_DUMMY_GUNNY)
    IGNORE(T_EN_G_WALKY, T_LOADZONE)
    IGNORE(T_EN_G_WALKY, T_CHECKPOINT)
    IGNORE(T_EN_G_WALKY, T_DUMMY_SWORDY)
    IGNORE(T_EN_G_WALKY, T_DUMMY_GUNNY)
    IGNORE(T_EN_TURRET, T_LOADZONE)
    IGNORE(T_EN_TURRET, T_CHECKPOINT)
    IGNORE(T_EN_TURRET, T_DUMMY_SWORDY)
    IGNORE(T_EN_TURRET, T_DUMMY_GUNNY)
        erv = ERR_OK;
    break;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    /* On Linux, a SIGINT is raised any time a unhandled collision
     * happens. When debugging, GDB will stop here and allow the user to
     * check which types weren't handled */
    default: {
#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))
        /* Unfiltered collision, do something about it */
        raise(SIGINT);
        erv = ERR_UNHANDLED_COLLISION;
#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */
    }
} /* switch (MERGE_TYPES(node1.type, node2.type)) */
ASSERT(erv == ERR_OK, erv);
