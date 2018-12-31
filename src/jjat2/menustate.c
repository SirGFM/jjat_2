#include <base/error.h>
#include <jjat2/menu_input.h>
#include <jjat2/menustate.h>

/** Initialize the menustate. */
err initMenustate() {
    return ERR_OK;
}

/** If the menustate has been initialized, properly free it up. */
void freeMenustate() {
}

/** Setup the loadstate so it may start to be executed */
err loadMenustate() {
    err erv;

    erv = setDefaultMenuInput();
    ASSERT(erv == ERR_OK, erv);

    return erv;
}

/** Update the menustate */
err updateMenustate() {
    return ERR_OK;
}

/** Draw the menustate */
err drawMenustate() {
    return ERR_OK;
}
