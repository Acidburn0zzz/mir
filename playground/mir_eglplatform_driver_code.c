#include "mir_eglplatform_driver_code.h"
#include "mir_toolkit/mir_client_library.h"
#include <stdlib.h>
#include <string.h>

//Information the driver will have to maintain
typedef struct
{
    MirConnection* connection;      //EGLNativeDisplayType
    MirRenderSurface* surface;      //EGLNativeWindowType
    MirBufferStream* stream;        //the internal semantics a driver might want to use...
                                    //could be MirPresentationChain as well
    int current_physical_width;     //The driver is in charge of the physical width
    int current_physical_height;    //The driver is in charge of the physical height
} DriverInfo;
static DriverInfo* info = NULL;

//note that this function only has information available to the driver at the time
//that eglCreateWindowSurface will be called.
EGLSurface future_driver_eglCreateWindowSurface(
    EGLDisplay display, EGLConfig config, MirRenderSurface* surface) //available from signature of EGLCreateWindowSurface
{
    if (info->surface)
    {
        printf("shim only supports one surface at the moment");
        return EGL_NO_SURFACE;
    }

    info->surface = surface;
    mir_render_surface_logical_size(surface,
        &info->current_physical_width, &info->current_physical_height);

    //TODO: the driver needs to be selecting a pixel format that's acceptable based on
    //      the EGLConfig. mir_connection_get_egl_pixel_format
    //      needs to be deprecated once the drivers support the Mir EGL platform.
    MirPixelFormat pixel_format = mir_connection_get_egl_pixel_format(info->connection, display, config);
    //this particular [silly] driver has chosen the buffer stream as the way it wants to post
    //its hardware content. I'd think most drivers would want MirPresentationChain for flexibility
    //FIXME: We don't have to match this create call with a free call?
    info->stream = mir_render_surface_create_buffer_stream_sync(
        surface,
        info->current_physical_width, info->current_physical_height,
        pixel_format,
        mir_buffer_usage_hardware);

    printf("The driver chose pixel format %d.\n", pixel_format);
    return eglCreateWindowSurface(display, config, (EGLNativeWindowType) surface, NULL);
}

EGLBoolean future_driver_eglSwapBuffers(
    EGLDisplay display, EGLSurface surface) //parameters given to swapbuffers
{
    int width = -1;
    int height = -1;
    mir_render_surface_logical_size(info->surface, &width, &height);
    if (width != info->current_physical_width || height != info->current_physical_height)
    {
        //note that this affects the next buffer that we get after swapbuffers.
        mir_buffer_stream_set_size(info->stream, width, height);
        info->current_physical_width = width;
        info->current_physical_height = height;
    } 
    return eglSwapBuffers(display, surface);
}

EGLDisplay future_driver_eglGetDisplay(MirConnection* connection)
{
    if (info)
    {
        printf("shim only supports one display connection at the moment");
        return EGL_NO_DISPLAY;
    }

    info = malloc(sizeof(DriverInfo));
    memset(info, 0, sizeof(*info));
    info->connection = connection;
    return eglGetDisplay(mir_connection_get_egl_native_display(connection));
}

EGLBoolean future_driver_eglTerminate(EGLDisplay display)
{
    if (info)
        free(info);
    return eglTerminate(display);
}
