#pragma once

class cltSystemMessage {
public:
    static int SetSystemMessage(cltSystemMessage* self, const char* msg, int a3, int a4, int a5);
};

extern cltSystemMessage g_clSysemMessage;
