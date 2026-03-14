#include "Logic/cltMyCharData.h"
#include "global.h"

cltMyCharData g_clMyCharData{};

int cltMyCharData::SetMyAccount(cltMyCharData* self, int account) {
    (void)self;
    g_dwMyAccountID = static_cast<unsigned int>(account);
    return 0;
}
