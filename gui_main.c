#include <stdlib.h>
#include <time.h>
#include "gui.h"
int main(void) {
    srand((unsigned int)time(NULL));
    return run_gui();
}
