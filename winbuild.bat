cl /I"C:\OpenSSL-Win32\include" /I"C:\Qt\5.9.1\msvc2017_64\include" /I"C:\Qt\5.9.1\msvc2017_64\include\QtCore" /I"C:\Qt\5.9.1\msvc2017_64\include\QtGui" /I"C:\Qt\5.9.1\msvc2017_64\include\QtWidgets" /EHsc /std:c++17 *.cpp libeay32.lib C:\Qt\5.9.1\msvc2017_64\lib\Qt5Core.lib C:\Qt\5.9.1\msvc2017_64\lib\Qt5Widgets.lib C:\Qt\5.9.1\msvc2017_64\lib\Qt5Gui.lib /link /out:Passwords.exe