/*
 * Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <cutils/log.h>
#include <gralloc_priv.h>
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>
#include "qdMetaData.h"

int setMetaData(private_handle_t *handle, DispParamType paramType,
                                                    void *param) {
    if (private_handle_t::validate(handle)) {
        ALOGE("%s: Private handle is invalid! handle=%p", __func__, handle);
        return -1;
    }
    if (handle->fd_metadata == -1) {
        ALOGE("%s: Bad fd for extra data!", __func__);
        return -1;
    }
    unsigned long size = ROUND_UP_PAGESIZE(sizeof(MetaData_t));
    void *base = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
        handle->fd_metadata, 0);
    if (base == reinterpret_cast<void*>(MAP_FAILED)) {
        ALOGE("%s: mmap() failed: error is %s!", __func__, strerror(errno));
        return -1;
    }
    MetaData_t *data = reinterpret_cast <MetaData_t *>(base);
    // If parameter is NULL reset the specific MetaData Key
    if (!param) {
       data->operation &= ~paramType;
       return munmap(base, size);
    }

    data->operation |= paramType;
    switch (paramType) {
        case PP_PARAM_INTERLACED:
            data->interlaced = *((int32_t *)param);
            break;
        case UPDATE_BUFFER_GEOMETRY:
            data->bufferDim = *((BufferDim_t *)param);
            break;
        case UPDATE_REFRESH_RATE:
            data->refreshrate = *((float *)param);
            break;
        case UPDATE_COLOR_SPACE:
            data->colorSpace = *((ColorSpace_t *)param);
            break;
        case MAP_SECURE_BUFFER:
            data->mapSecureBuffer = *((int32_t *)param);
            break;
        case S3D_FORMAT:
            data->s3dFormat = *((uint32_t *)param);
            break;
        case LINEAR_FORMAT:
            data->linearFormat = *((uint32_t *)param);
            break;
        case SET_IGC:
            data->igc = *((IGC_t *)param);
            break;
        case SET_SINGLE_BUFFER_MODE:
            data->isSingleBufferMode = *((uint32_t *)param);
            break;
        case SET_S3D_COMP:
            data->s3dComp = *((S3DGpuComp_t *)param);
            break;
        case SET_VT_TIMESTAMP:
            data->vtTimeStamp = *((uint64_t *)param);
            break;
#ifdef USE_COLOR_METADATA
        case COLOR_METADATA:
            data->color = *((ColorMetaData *)param);
#endif
            break;
        default:
            ALOGE("Unknown paramType %d", paramType);
            break;
    }
    if(munmap(base, size))
        ALOGE("%s: failed to unmap ptr %p, err %d", __func__, (void*)base,
                                                                        errno);
    return 0;
}

int clearMetaData(private_handle_t *handle, DispParamType paramType) {
    if (!handle) {
        ALOGE("%s: Private handle is null!", __func__);
        return -1;
    }
    if (handle->fd_metadata == -1) {
        ALOGE("%s: Bad fd for extra data!", __func__);
        return -1;
    }

    unsigned long size = ROUND_UP_PAGESIZE(sizeof(MetaData_t));
    void *base = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
        handle->fd_metadata, 0);
    if (base == reinterpret_cast<void*>(MAP_FAILED)) {
        ALOGE("%s: mmap() failed: error is %s!", __func__, strerror(errno));
        return -1;
    }
    MetaData_t *data = reinterpret_cast <MetaData_t *>(base);
    data->operation &= ~paramType;
    switch (paramType) {
        case SET_S3D_COMP:
            data->s3dComp.displayId = -1;
            data->s3dComp.s3dMode = 0;
            break;
        default:
            ALOGE("Unknown paramType %d", paramType);
            break;
    }
    if(munmap(base, size))
        ALOGE("%s: failed to unmap ptr %p, err %d", __func__, (void*)base,
                                                                        errno);
    return 0;
}

int getMetaData(private_handle_t *handle, DispFetchParamType paramType,
                                                    void *param) {
    int ret = -1;
    if (!handle) {
        ALOGE("%s: Private handle is null!", __func__);
        return -1;
    }
    if (handle->fd_metadata == -1) {
        ALOGE("%s: Bad fd for extra data!", __func__);
        return -1;
    }
    if (!param) {
        ALOGE("%s: input param is null!", __func__);
        return -1;
    }
    unsigned long size = ROUND_UP_PAGESIZE(sizeof(MetaData_t));
    void *base = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
        handle->fd_metadata, 0);
    if (base == reinterpret_cast<void*>(MAP_FAILED)) {
        ALOGE("%s: mmap() failed: error is %s!", __func__, strerror(errno));
        return -1;
    }

    MetaData_t *data = reinterpret_cast <MetaData_t *>(base);
    switch (paramType) {
        case GET_PP_PARAM_INTERLACED:
            if (data->operation & PP_PARAM_INTERLACED) {
                *((int32_t *)param) = data->interlaced;
                ret = 0;
            }
            break;
        case GET_BUFFER_GEOMETRY:
            if (data->operation & UPDATE_BUFFER_GEOMETRY) {
                *((BufferDim_t *)param) = data->bufferDim;
                ret = 0;
            }
            break;
        case GET_REFRESH_RATE:
            if (data->operation & UPDATE_REFRESH_RATE) {
                *((float *)param) = data->refreshrate;
                ret = 0;
            }
            break;
        case GET_COLOR_SPACE:
            if (data->operation & UPDATE_COLOR_SPACE) {
                *((ColorSpace_t *)param) = data->colorSpace;
                ret = 0;
            }
            break;
        case GET_MAP_SECURE_BUFFER:
            if (data->operation & MAP_SECURE_BUFFER) {
                *((int32_t *)param) = data->mapSecureBuffer;
                ret = 0;
            }
            break;
        case GET_S3D_FORMAT:
            if (data->operation & S3D_FORMAT) {
                *((uint32_t *)param) = data->s3dFormat;
                ret = 0;
            }
            break;
        case GET_LINEAR_FORMAT:
            if (data->operation & LINEAR_FORMAT) {
                *((uint32_t *)param) = data->linearFormat;
                ret = 0;
            }
            break;
        case GET_IGC:
            if (data->operation & SET_IGC) {
                *((IGC_t *)param) = data->igc;
                ret = 0;
            }
            break;
        case GET_SINGLE_BUFFER_MODE:
            if (data->operation & SET_SINGLE_BUFFER_MODE) {
                *((uint32_t *)param) = data->isSingleBufferMode;
                ret = 0;
            }
            break;
        case GET_S3D_COMP:
            if (data->operation & SET_S3D_COMP) {
                *((S3DGpuComp_t *)param) = data->s3dComp;
                ret = 0;
            }
            break;
        case GET_VT_TIMESTAMP:
            if (data->operation & SET_VT_TIMESTAMP) {
                *((uint64_t *)param) = data->vtTimeStamp;
                ret = 0;
            }
            break;
#ifdef USE_COLOR_METADATA
        case GET_COLOR_METADATA:
            if (data->operation & COLOR_METADATA) {
                *((ColorMetaData *)param) = data->color;
                ret = 0;
            }
#endif
            break;
        default:
            ALOGE("Unknown paramType %d", paramType);
            break;
    }
    if(munmap(base, size))
        ALOGE("%s: failed to unmap ptr %p, err %d", __func__, (void*)base,
                                                                        errno);
    return ret;
}

int copyMetaData(struct private_handle_t *src, struct private_handle_t *dst) {
    if (!src || !dst) {
        ALOGE("%s: Private handle is null!", __func__);
        return -1;
    }
    if (src->fd_metadata == -1) {
        ALOGE("%s: Bad fd for src extra data!", __func__);
        return -1;
    }
    if (dst->fd_metadata == -1) {
        ALOGE("%s: Bad fd for dst extra data!", __func__);
        return -1;
    }

    unsigned long size = ROUND_UP_PAGESIZE(sizeof(MetaData_t));

    void *base_src = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
        src->fd_metadata, 0);
    if (base_src == reinterpret_cast<void*>(MAP_FAILED)) {
        ALOGE("%s: src mmap() failed: error is %s!", __func__, strerror(errno));
        return -1;
    }

    void *base_dst = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,
        dst->fd_metadata, 0);
    if (base_dst == reinterpret_cast<void*>(MAP_FAILED)) {
        ALOGE("%s: dst mmap() failed: error is %s!", __func__, strerror(errno));
        if(munmap(base_src, size))
            ALOGE("%s: failed to unmap src ptr %p, err %d", __func__,
                                             (void*)base_src, errno);
        return -1;
    }

    memcpy(base_dst, base_src, size);

    if(munmap(base_src, size))
        ALOGE("%s: failed to unmap src ptr %p, err %d", __func__, (void*)base_src,
                                                                        errno);
    if(munmap(base_dst, size))
        ALOGE("%s: failed to unmap src ptr %p, err %d", __func__, (void*)base_dst,
                                                                        errno);
    return 0;
}
