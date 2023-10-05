#===============================================================================
# 如果未指定平台，则使用当前系统平台
ifeq ($(PLATFORM),)
	ifeq ($(shell cat /proc/version | grep x86_64),)
		PLATFORM=i386-linux
	else
		PLATFORM=x64-linux
	endif
endif

#===============================================================================
# 编译类型
ifeq ($(r),1)
	RELEASE=1
endif

ifeq ($(u),1)
	UNICODE=1
endif

ifeq ($(g),1)
	GINFO=1
endif

#===============================================================================
# 得到顶层目录【ep.rules目录的父目录】绝对路径
TOPDIR=$(shell sh -c '\
		dir=`pwd|sed s/[/]$$//`; \
		while test $$dir; do \
			if test -d $$dir/ep.rules; then \
				echo $$dir; \
				exit; \
			fi; \
			dir=`echo $$dir|sed s/[/][^/]*$$//`; \
		done; \
		if test -d /usr/soedk/ep.rules; then \
			echo /usr/soedk; \
			exit; \
		fi \
	')

# 用户设定

#===============================================================================
# 最终目标类型及文件名
# Linux目标文件命名规则: 静态库为xxx.a，动态库为libxxx.so，执行文件不要前/后缀以及扩展名

MAKE_STATIC_LIB := 1
#MAKE_SHARED_LIB := 1
EXECUTABLE := jsoncpp.a

#===============================================================================
# C/C++程序的包含文件搜索路径，如-I/usr/soedk/include
INCLUDE_PATH := $(SYS_INCLUDE_PATH) \
				-I. \
				-I./include

				
#===============================================================================

#===============================================================================
# C/C++程序预定义全局宏，如-DECRT_DECL表示定义全局宏ECRT_DECL
DEFINES :=
#===============================================================================

SOEDK_LIB_PATH := /usr/soedk/lib
SOEDK_SO_PATH := $(SOEDK_LIB_PATH)
ifeq ($(RELEASE),)
	SOEDK_SO_PATH := $(SOEDK_SO_PATH)/so_d
endif

#===============================================================================
# 目标链接时搜索的动态库(.so)路径，如：-L./ -L./source
# SYS_LIBRARY_PATH 为系统lib文件路径的环境变量
LIBRARY_PATH := $(SYS_LIBRARY_PATH)

#===============================================================================
# 目标程序需要链接的动态库(.so)列表（如 stdc++, pthread等）
# 要带-l前缀，只写该动态库的link名，例如-lstdc++表示会link在LIBRARY_PATH路径下的libstdc++.so
LIBS := -lstdc++ -lpthread -lrt -ldl 

# 目标程序需要连接的静态库(.a)
# 不带-l前缀，且需要写出该静态库的全路径，例如xxx/debug.$(PLATFORM)/xxx.a
USERLIBS :=

# 不同编译版本静态库设置
ifeq ($(RELEASE),1)
	USERLIBS += 
else
	USERLIBS += 
endif
#===============================================================================

#===============================================================================
# 本工程目标的源文件设置，默认已添加了Makefile文件所在目录的./source下的所有C/C++源文件
SOURCE := $(wildcard ./*.c) $(wildcard ./*.cpp)
SOURCE += $(wildcard src/lib_json/*.cpp)
_SOURCE := 

# 源文件最终集合 = [SOURCE集] - [_SOURCE集]
SOURCE := $(filter-out $(_SOURCE),$(SOURCE))

# 需要添加NO_GB2312=1才能编译的源文件（注：需要在$(_SOURCE)里添加，以在$(SOURCE)变量排除这些文件）
SOURCE_NO_GB2312 :=

#===============================================================================
# 源文件特别编译选项，支持两组（XSRC1/XOPT1、XSRC2/XOPT2），每组里可以有多个源文件，若不够可修改"rules.gcc"增加
#XSRC1 := ./source/xxxx.cpp
#XOPT1 := 针对$(XSRC1)的特定编译选项

#===============================================================================
# C++预编译设置，C语言不支持预编译
#PRECOMPILE_HEADER := ./StdAfx.h

#===============================================================================
# 编译输出目录
ifeq ($(RELEASE),1)
	OUTPUT_DIR := release.$(PLATFORM)
else
	OUTPUT_DIR := debug.$(PLATFORM)
endif
ifeq ($(GINFO),1)
	OUTPUT_DIR := g.$(OUTPUT_DIR)
endif
ifeq ($(UNICODE),1)
	OUTPUT_DIR := u.$(OUTPUT_DIR)
endif

#===============================================================================
# 编译后处理命令
ifeq ($(MAKE_STATIC_LIB),)
	ifeq ($(MAKE_SHARED_LIB),)
		POST_BUILD_CMD := 
	endif
endif
#===============================================================================

include $(TOPDIR)/ep.rules/rules.$(PLATFORM)
