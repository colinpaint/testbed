#pragma once
//{{{  includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <map>
#include <vector>
#include <algorithm>
#include <va/va.h>
#include "DecodeParamBuffer.h"
//}}}

namespace mvaccel {
  class VDecAccelVAImpl {
  public:
    VDecAccelVAImpl (void* device);
    VDecAccelVAImpl();
    virtual ~VDecAccelVAImpl();

    // VDecAccel interface
    virtual int         Open();
    virtual void        Close();
    virtual uint32_t    GetSurfaceID (uint32_t index);
    bool                DecodePicture();
    void                bind_buffer (uint8_t* base);

  protected:
    // GfxSurfaceAccess interface
    uint32_t    get_width (VASurfaceID id);
    uint32_t    get_height (VASurfaceID id);
    uint32_t    get_stride (VASurfaceID id);
    uint8_t*    lock_surface (VASurfaceID id, bool write);
    void        unlock_surface (VASurfaceID id);
    void*       get_raw (VASurfaceID id);
    void*       get_device (VASurfaceID id);

    typedef std::vector<VAConfigAttrib> VAConfigAttribArray;
    typedef std::vector<VASurfaceAttrib> VASurfaceAttribArray;

    // Member functions inherit by children
    virtual bool    is_config_compatible (DecodeDesc& desc);
    virtual bool    is_rt_foramt_supported (DecodeDesc& desc);
    virtual void    prepare_config_attribs (DecodeDesc& desc, VAConfigAttribArray& attribs);
    virtual void    prepare_surface_attribs (DecodeDesc& desc, VASurfaceAttribArray& attribs,
                                             bool bDecodeDownsamplingHinted);
    // Member fucntions NOT inherit by children
    bool        is_slice_mode_supported (DecodeDesc& desc);
    bool        is_encryption_supported (DecodeDesc& desc);
    bool        is_sfc_config_supported (DecodeDesc& desc);
    VAStatus    render_picture (VAContextID& vaContextID);
    VAStatus    query_status();
    VAStatus    create_surface (uint32_t width, uint32_t height, VASurfaceAttribArray& vaAttribs, VASurfaceID& vaID);
    void        delete_surface (VASurfaceID& vaID);
    void        create_decode_desc();
    int         check_process_pipeline_caps (DecodeDesc& desc);
    int         create_resources();

    int drm_fd = -1;
    // VA ID & Handles
    VADisplay       m_vaDisplay;    /**< @brief VA hardware device */
    VAProfile       m_vaProfile;    /**< @brief Video decoder profile */
    VAEntrypoint    m_vaEntrypoint; /**< @brief VA entry point */
    VAConfigID      m_vaConfigID;   /**< @brief VA decode config id*/
    VAContextID     m_vaContextID;  /**< @brief Video decoder context */

    // VASurfaces id and attributes
    std::vector<VASurfaceID> m_vaIDs;
    VASurfaceAttribArray m_vaSurfAttribs;
    uint32_t m_surfaceType;   /**< @brief surface type */

    // Gfx surface access management
    std::map<VASurfaceID, VAImage> m_images;   /**< @brief buf pointer */

    //{{{
    enum SFC {
        MAX_IN_W,
        MAX_IN_H,
        MIN_IN_W,
        MIN_IN_H,
        MAX_OUT_W,
        MAX_OUT_H,
        MIN_OUT_W,
        MIN_OUT_H,
        NEW_W,
        NEW_H,
    };
    //}}}

    DecodeDesc                    m_DecodeDesc{};     /**< @brief decode discription */
    VARectangle                   m_rectSrc{};        /**< @brief Rectangle for source input */
    VARectangle                   m_rectSFC{};        /**< @brief Rectangle for SFC output */
    std::vector<uint32_t>         m_in4CC{};          /**< @brief input FOURCC */
    std::vector<uint32_t>         m_out4CC{};         /**< @brief output FOURCC */
    std::map<SFC, uint32_t>        m_sfcSize{};       /**< @brief SFC sizes */
    std::vector<VASurfaceID>      m_sfcIDs{};         /**< @brief sfc surfaces */
    VAProcPipelineParameterBuffer m_vaProcBuffer{};   /**< @brief sfc pipeline buffer */
    };
} 
