# CMakeList.txt : divisi �� CMake �v���W�F�N�g�B�\�[�X���܂߂āA�����`���܂�:
# �v���W�F�N�g��p���W�b�N�͂�����ł��B
#

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

include_directories(
	"../"
	"../json/single_include"
)

set(lib_files 
	"divisi.cpp" 
	"../include/divisi.h"
	"deserializer.cpp"
	"deserializer.h"
	"writer/csvwriter.cpp"
	"writer/csvwriter.h"
	"writer/writerbase.cpp"
	"writer/writerbase.h"
)

# �\�[�X�����̃v���W�F�N�g�̎��s�\�t�@�C���ɒǉ����܂��B
add_library (divisi_lib STATIC ${lib_files})

target_include_directories(divisi_lib PUBLIC 
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/json
)

# TODO: �e�X�g��ǉ����A�K�v�ȏꍇ�́A�^�[�Q�b�g���C���X�g�[�����܂��B