# MOF
Network MMORPG example for Windows Platform

# Dependency
1. fmod: 負責音效播放，library已經附在程式碼裡。
2. d3dx9: 需要安裝舊版DirectX SDK，安裝方法參考:
	- 下載網址: https://www.microsoft.com/en-us/download/details.aspx?id=6812
	- 安裝前要先解除安裝: Microsoft Visual C++ 2010 x64 Redistribute

# Notice
1. 第一次執行要先把遊戲資源打包成mof.pak和mof.paki
	- 先把mofdata.zip解壓縮，放入專案根目錄。
	- 下載網址:https://drive.google.com/file/d/1ju1TbYRCycJYOiEkXaXSy9MshcT1ANUA/view?usp=sharing
	- 把 main.cpp 的 create_vfs_archive(); 解除註解。
	- 執行一次打包流程。
