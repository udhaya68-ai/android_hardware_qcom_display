ifeq ($(TARGET_QCOM_DISPLAY_VARIANT),)

# TODO:  Find a better way to separate build configs for ADP vs non-ADP devices
ifneq ($(TARGET_BOARD_AUTO),true)
  ifneq ($(filter msm8084 msm8x84,$(TARGET_BOARD_PLATFORM)),)
    #This is for 8084 based platforms
    include $(call all-named-subdir-makefiles,msm8084)
  else
    ifneq ($(filter msm8226 msm8x26,$(TARGET_BOARD_PLATFORM)),)
      include $(call all-named-subdir-makefiles,msm8226)
    else
      ifneq ($(filter msm8960,$(TARGET_BOARD_PLATFORM)),)
        include $(call all-named-subdir-makefiles,msm8960)
      else
        ifneq ($(filter msm8952 msm8994 msm8992,$(TARGET_BOARD_PLATFORM)),)
          include $(call all-named-subdir-makefiles,msm8994)
        else
          ifneq ($(filter msm8909,$(TARGET_BOARD_PLATFORM)),)
            include $(call all-named-subdir-makefiles,msm8909)
          else
            ifneq ($(filter msm8996,$(TARGET_BOARD_PLATFORM)),)
              include $(call all-named-subdir-makefiles,msm8996)
            else
              ifneq ($(filter msm8998,$(TARGET_BOARD_PLATFORM)),)
                include $(call all-named-subdir-makefiles,msm8998)
              endif
            endif
          endif
        endif
      endif
    endif
  endif
endif

endif
