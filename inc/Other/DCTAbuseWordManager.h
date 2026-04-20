#pragma once

// mofclient.c references &g_DCTAbuseWordManager and calls
// DCTAbuseWordManager::ChangeString(src, dst) to replace profanity in a chat
// line before it is displayed or posted.  The replacement table itself lives
// in a data file we have not restored, so ChangeString here is a pass-through
// copy — matching the behaviour of an empty abuse table.

class DCTAbuseWordManager {
public:
    DCTAbuseWordManager() = default;
    ~DCTAbuseWordManager() = default;

    // Copies src into dst verbatim.  src/dst may point at the same buffer in
    // the ground truth (the chat manager sometimes round-trips through a
    // temporary), so we treat it as strcpy semantics when distinct.
    void ChangeString(const char* src, char* dst);
};

extern DCTAbuseWordManager g_DCTAbuseWordManager;
