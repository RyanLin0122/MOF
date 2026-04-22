# CUI classes reverse-engineering order (derived from mofclient.c)

- Source: `mofclient.c` function definition ranges and constructor calls.
- `lines` = sum of decompiled function block lines for each class in `mofclient.c`.
- `parents` = inferred base class from constructor call (`Parent::Parent(...)`).
- `depends_on_text` = `Y` if the class uses `DCTTextManager` (reads `MoFTexts.txt`) or reads from a `clt*Info` / `clt*KindInfo` static.
- `text_sources` = `MoFTexts.txt` when `DCTTextManager` is used, plus any `clt*Info` / `clt*KindInfo` classes the code reads from.

| order | class | lines | parents | depends_on_text | text_sources |
|---:|---|---:|---|:---:|---|
| 1 | CUIBase | 893 | - | Y | MoFTexts.txt |
| 2 | CUIClassTree | 31 | - | N | - |
| 3 | CUIHelper | 721 | - | N | - |
| 4 | CUIManager | 2677 | - | Y | cltMapCollisonInfo |
| 5 | CUIPet | 40 | - | N | - |
| 6 | CUIAcademy | 1024 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo, cltLessonKindInfo |
| 7 | CUIBasic | 15494 | CUIBase | Y | MoFTexts.txt, cltCharKindInfo, cltClassKindInfo, cltClientCharKindInfo, cltClientPortalInfo, cltEmblemKindInfo, cltItemKindInfo, cltMapUseItemInfoKindInfo, cltPetKindInfo, cltSpecialtyKindInfo |
| 8 | CUIBoard | 357 | CUIBase | N | - |
| 9 | CUIBuySpecialty | 500 | CUIBase | Y | MoFTexts.txt, cltSpecialtyKindInfo |
| 10 | CUICallPartyMember | 330 | CUIBase | N | - |
| 11 | CUICashShop | 4405 | CUIBase | Y | MoFTexts.txt, cltEmoticonKindInfo, cltItemKindInfo, cltMapUseItemInfoKindInfo, cltPetKindInfo, cltPetSkillKindInfo |
| 12 | CUICashShopConfirm | 1517 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 13 | CUICashShopHelp | 125 | CUIBase | N | - |
| 14 | CUICashShopPackageItemInfo | 529 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 15 | CUICashShopResult | 291 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 16 | CUIChangeSex | 319 | CUIBase | N | - |
| 17 | CUICharInfo | 1224 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo |
| 18 | CUIChatWindowRegulation | 77 | CUIBase | N | - |
| 19 | CUICircle | 1898 | CUIBase | Y | MoFTexts.txt |
| 20 | CUICommonWeddingWindow | 576 | CUIBase | Y | MoFTexts.txt |
| 21 | CUIDiary | 6411 | CUIBase | Y | MoFTexts.txt, cltEmblemKindInfo, cltItemKindInfo, cltMonsterAniInfo, cltNPCInfo, cltPKRankKindInfo, cltQuestKindInfo, cltSpecialtyKindInfo |
| 22 | CUIDied | 233 | CUIBase | N | - |
| 23 | CUIDivideMap | 2036 | CUIBase | Y | MoFTexts.txt, cltMapInfo, cltNPCInfo, cltQuestKindInfo, cltRegenMonsterKindInfo, cltWorldMapColorInfo |
| 24 | CUIEmoticon | 595 | CUIBase | Y | cltEmoticonKindInfo |
| 25 | CUIEnchantHelp | 155 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 26 | CUIEnchantItem | 922 | CUIBase | Y | MoFTexts.txt, cltEnchantKindInfo, cltItemKindInfo |
| 27 | CUIEquip | 1289 | CUIBase | Y | MoFTexts.txt, cltCoupleRingKindInfo, cltItemKindInfo |
| 28 | CUIFightMakeRoom | 1283 | CUIBase | Y | MoFTexts.txt |
| 29 | CUIFightRoom | 2372 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo, cltPKRankKindInfo |
| 30 | CUIFightRoomChannel | 1566 | CUIBase | Y | MoFTexts.txt |
| 31 | CUIFightRoomResult | 583 | CUIBase | Y | MoFTexts.txt |
| 32 | CUIFriend | 3336 | CUIBase | Y | MoFTexts.txt |
| 33 | CUIGetIndunInfo | 321 | CUIBase | Y | MoFTexts.txt, cltInstantDungeonKindInfo, cltItemKindInfo |
| 34 | CUIHelp | 463 | CUIBase | N | - |
| 35 | CUIInventory | 945 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 36 | CUIItemSeal | 450 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 37 | CUIJobChange | 823 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo, cltItemKindInfo |
| 38 | CUIKeyMap | 441 | CUIBase | N | - |
| 39 | CUILobbyLogin | 564 | CUIBase | Y | MoFTexts.txt |
| 40 | CUILobbyMakeChar | 650 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo |
| 41 | CUILobbySelChannel | 1475 | CUIBase | Y | MoFTexts.txt |
| 42 | CUILobbySelChar | 944 | CUIBase | Y | MoFTexts.txt |
| 43 | CUIMakeAvi | 198 | CUIBase | Y | MoFTexts.txt |
| 44 | CUIMaking | 963 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo, cltMakingItemKindInfo, cltSpecialtyKindInfo |
| 45 | CUIMapConqueror | 337 | CUIBase | Y | cltClassKindInfo |
| 46 | CUIMapConquest | 234 | CUIBase | Y | MoFTexts.txt |
| 47 | CUIMarryBoard | 820 | CUIBase | Y | MoFTexts.txt |
| 48 | CUIMarryPetition | 748 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo, cltItemKindInfo, cltWeddingHallKindInfo |
| 49 | CUIMenu | 432 | CUIBase | Y | MoFTexts.txt |
| 50 | CUIMessageBoxBase | 180 | CUIBase | N | - |
| 51 | CUIMyItem | 281 | CUIBase | Y | MoFTexts.txt, cltMyItemKindInfo |
| 52 | CUINPC | 3158 | CUIBase | Y | MoFTexts.txt, cltInstantDungeonKindInfo, cltItemKindInfo, cltNPCInfo, cltQuestKindInfo |
| 53 | CUINPCLessonEnd | 131 | CUIBase | Y | MoFTexts.txt |
| 54 | CUINPCMarketPetInfo | 728 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltItemKindInfo, cltPetKindInfo, cltPetSkillKindInfo |
| 55 | CUINPCMeritorious | 351 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 56 | CUINPCPetInfo | 900 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltItemKindInfo, cltPetKindInfo, cltPetSkillKindInfo |
| 57 | CUINPCPetShop | 549 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 58 | CUINPCQuest | 869 | CUIBase | Y | MoFTexts.txt, cltNPCInfo, cltQuestKindInfo |
| 59 | CUINPCTest | 642 | CUIBase | Y | MoFTexts.txt, cltNPCInfo, cltQuestionKindInfo |
| 60 | CUIOption | 1106 | CUIBase | Y | MoFTexts.txt |
| 61 | CUIPartyInfo | 793 | CUIBase | Y | MoFTexts.txt, cltSkillKindInfo |
| 62 | CUIPetInfo | 1657 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltItemKindInfo, cltPetKindInfo, cltPetSkillKindInfo |
| 63 | CUIPetKeeping | 507 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltPetKindInfo |
| 64 | CUIPetMarketInfo | 1041 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltPetKindInfo |
| 65 | CUIPetMarketRegister | 473 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltPetKindInfo |
| 66 | CUIPetRelease | 347 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltPetKindInfo |
| 67 | CUIPetSellInfo | 303 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltPetKindInfo |
| 68 | CUIPopMenu | 542 | CUIBase | N | - |
| 69 | CUIPostIt | 497 | CUIBase | N | - |
| 70 | CUIPrivateShop | 1648 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 71 | CUIQuest | 1550 | CUIBase | Y | MoFTexts.txt, cltNPCInfo, cltQuestKindInfo, cltRegenMonsterKindInfo |
| 72 | CUIQuestAlarm | 1172 | CUIBase | Y | MoFTexts.txt, cltInstantDungeonKindInfo, cltInstantDungeonMapKindInfo, cltItemKindInfo, cltQuestKindInfo |
| 73 | CUIQuickChatting | 1170 | CUIBase | Y | MoFTexts.txt |
| 74 | CUIRecall | 253 | CUIBase | Y | MoFTexts.txt |
| 75 | CUIReceivePet | 282 | CUIBase | Y | MoFTexts.txt, cltClientPetKindInfo, cltPetKindInfo |
| 76 | CUISalesAgencyMyList | 290 | CUIBase | Y | cltItemKindInfo |
| 77 | CUISalesAgencyReceive | 383 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 78 | CUISalesAgencyRegist | 571 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 79 | CUISalesAgencySearch | 1214 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 80 | CUIScholarShip | 250 | CUIBase | Y | MoFTexts.txt |
| 81 | CUISetEmoticon | 226 | CUIBase | Y | cltEmoticonKindInfo |
| 82 | CUIShopNPC | 3302 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo, cltItemKindInfo |
| 83 | CUIShopNPCEmblem | 393 | CUIBase | Y | MoFTexts.txt, cltEmblemKindInfo |
| 84 | CUISkill | 412 | CUIBase | Y | MoFTexts.txt, cltSkillKindInfo, cltSpecialtyKindInfo |
| 85 | CUISkillShop | 561 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo, cltSkillKindInfo |
| 86 | CUIStorage | 1838 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 87 | CUITitleMsgBox | 169 | CUIBase | Y | MoFTexts.txt |
| 88 | CUITradeUser | 865 | CUIBase | Y | MoFTexts.txt, cltItemKindInfo |
| 89 | CUIUserInfo | 290 | CUIBase | Y | MoFTexts.txt, cltClassKindInfo, cltPKRankKindInfo |
| 90 | CUIViewInstanceProject | 611 | CUIBase | Y | MoFTexts.txt, cltInstantDungeonKindInfo, cltInstantDungeonMapKindInfo, cltItemKindInfo |
| 91 | CUIWarMeritorious | 580 | CUIBase | Y | MoFTexts.txt |
| 92 | CUIWeddingInvitation | 516 | CUIBase | Y | MoFTexts.txt |
| 93 | CUIWishTree | 82 | CUIBase | N | - |
| 94 | CUIWorldMap | 1521 | CUIBase | Y | cltMapInfo, cltNPCInfo, cltQuestKindInfo, cltRegenMonsterKindInfo |
| 95 | CUIMessageBoxCalc | 469 | CUIMessageBoxBase | Y | MoFTexts.txt |
| 96 | CUIMessageBoxEdit | 604 | CUIMessageBoxBase | N | - |
| 97 | CUIMessageBoxList | 529 | CUIMessageBoxBase | N | - |
| 98 | CUIMessageBoxMultLineOK | 172 | CUIMessageBoxBase | Y | MoFTexts.txt |
| 99 | CUIMessageBoxMultLineOKLarge | 172 | CUIMessageBoxBase | Y | MoFTexts.txt |
| 100 | CUIMessageBoxNotice | 95 | CUIMessageBoxBase | N | - |
| 101 | CUIMessageBoxOK | 530 | CUIMessageBoxBase | Y | MoFTexts.txt |
| 102 | CUIMessageBoxRadioList | 203 | CUIMessageBoxBase | Y | MoFTexts.txt |
| 103 | CUIMessageBoxSortList | 199 | CUIMessageBoxBase | N | - |
| 104 | CUITutorial | 330 | CUIMessageBoxBase | Y | MoFTexts.txt |
