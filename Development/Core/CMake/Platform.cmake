# Check for existence of 'ShmitWare-target-platform' target
if(TARGET ShmitWare-target-platform)
    # Target checks pass, create alias library to reference 'ShmitWare-target-platform'
    add_library(ShmitCore-target-platform ALIAS ShmitWare-target-platform)
else()
    # 'ShmitWare-target-platform' does not exist, create default target for native configuration
    add_library(ShmitCore-target-platform INTERFACE)
    target_compile_definitions(ShmitCore-target-platform
        INTERFACE
        NATIVE_SHMIT
        SHMIT_PLATFORM=native
    )
endif()
