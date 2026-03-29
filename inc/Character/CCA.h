#pragma once

struct CEffectBase;

// CCA：角色動畫核心（size = 292 bytes in binary）
class CCA
{
public:
    CCA();
    ~CCA();

    void LoadCA(const char* path, CEffectBase** a2 = nullptr, CEffectBase** a3 = nullptr);
    void Play(int motion, bool loop);
    void Process();
    void Draw();
    void InitItem(unsigned char sex, unsigned short a3, unsigned short a4, unsigned int a5);
    void ResetItem(unsigned char sex, unsigned short a3, unsigned short a4, unsigned char a5);
    void SetItemID(unsigned short itemId, unsigned char sex, int a4, int a5, int a6, unsigned char a7);
    void BegineEmoticon(int a2);
    void EndEmoticon(unsigned short a2, unsigned char a3);

private:
    char m_data[292];
};

// 外部函式（定義在 Character 模組）
unsigned char ExGetIllustCharSexCode(char charKind);
