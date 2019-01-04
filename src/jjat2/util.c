#include <base/error.h>
#include <base/game.h>
#include <jjat2/util.h>
#include <stdlib.h>
#include <string.h>

err setInputStr(const char *str, int len) {
    if (game.inputMapSize < len + 1)
        game.curInputMap = realloc(game.curInputMap, len + 1);
    ASSERT(game.curInputMap, ERR_BUFFERTOOSMALL);

    memcpy(game.curInputMap, str, len);
    game.curInputMap[len] = '\0';

    return ERR_OK;
}
