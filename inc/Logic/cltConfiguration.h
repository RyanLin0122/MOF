#pragma once

class cltConfiguration {
public:
    cltConfiguration() = default;
    ~cltConfiguration() = default;

    // Reads the persisted help-message visibility state into *outState.
    void GetHelpState(int* outState);
};

extern cltConfiguration* g_clConfig;
