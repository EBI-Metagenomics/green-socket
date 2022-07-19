#include "gs/gs.h"
#include "helper.h"

int main(void)
{
    gs_init();
    if (gs_start()) ERROR;
    return 0;
}
