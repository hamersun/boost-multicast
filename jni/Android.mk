LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := sender.arm
LOCAL_SRC_FILES := ../sender.cpp
LOCAL_CFLAGS := -fexceptions -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie
#LOCAL_C_INCLUDES += # need to input boost path

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := receiver.arm
LOCAL_SRC_FILES := ../receiver.cpp
LOCAL_CFLAGS := -fexceptions -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie
#LOCAL_C_INCLUDES += # need to input boost path

include $(BUILD_EXECUTABLE)