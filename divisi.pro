QT -= gui
CONFIG += core

TEMPLATE = lib
CONFIG += static
DEFINES += DIVISI_LIBRARY

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

LANGSCORE_PATH = $$absolute_path(..\, $$PWD)

HEADERS += \
    deserializer.h \
    divisi.h \
    ./writer/writerbase.h \
    ./writer/csvwriter.h

SOURCES += \
    deserializer.cpp \
    divisi.cpp \
    ./writer/writerbase.cpp \
    ./writer/csvwriter.cpp \

# 置換関数の宣言（使うところより前）
defineReplace(makeCopyCommand){
    DEPEND_FILE = $$1
    win32:{
        CONFIG(debug,debug|release){
            APP_BUILD_DIR=$$shadowed($${PWD})/debug
        }else{
            APP_BUILD_DIR=$$shadowed($${PWD})/release
        }
        DEPEND_FILE ~= s|/|\|gi
        APP_BUILD_DIR ~= s|/|\|gi
    }else{
        APP_BUILD_DIR=$$shadowed($${PWD})
    }
    return($(COPY_DIR) $$DEPEND_FILE $$APP_BUILD_DIR)
}

# なんかのファイル1個目
ruby.commands += $$makeCopyCommand($${LANGSCORE_PATH}/ruby)
QMAKE_EXTRA_TARGETS += ruby
# なんかのファイル2個目
rv2tojson.commands += $$makeCopyCommand($${LANGSCORE_PATH}/rvdata2json)
QMAKE_EXTRA_TARGETS += rv2tojson
