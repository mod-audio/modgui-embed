/*
 * Utilities for MODGUI X11UI, based on Carla code
 * Copyright (C) 2015 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#include "CarlaPipeUtils.hpp"
#include "CarlaThread.hpp"

// -------------------------------------------------------------------------------------------------------------------

typedef void* CarlaPipeClientHandle;
typedef void (*CarlaPipeCallbackFunc)(void* ptr, const char* msg);

// -------------------------------------------------------------------------------------------------------------------

class CarlaPipeClientPlugin : public CarlaPipeClient
{
public:
    CarlaPipeClientPlugin(const CarlaPipeCallbackFunc callbackFunc, void* const callbackPtr) noexcept
        : CarlaPipeClient(),
          fCallbackFunc(callbackFunc),
          fCallbackPtr(callbackPtr),
          leakDetector_CarlaPipeClientPlugin()
    {
        CARLA_SAFE_ASSERT(fCallbackFunc != nullptr);
    }

    const char* readlineblock(const uint timeout) noexcept
    {
        return CarlaPipeClient::_readlineblock(timeout);
    }

    bool msgReceived(const char* const msg) noexcept
    {
        if (fCallbackFunc != nullptr)
        {
            try {
                fCallbackFunc(fCallbackPtr, msg);
            } CARLA_SAFE_EXCEPTION("msgReceived");
        }

        return true;
    }

private:
    const CarlaPipeCallbackFunc fCallbackFunc;
    void* const fCallbackPtr;

    CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CarlaPipeClientPlugin)
};

// -------------------------------------------------------------------------------------------------------------------

CARLA_EXPORT void carla_set_process_name(const char* name)
{
    carla_debug("carla_set_process_name(\"%s\")", name);

    CarlaThread::setCurrentThreadName(name);
}

// -------------------------------------------------------------------------------------------------------------------

CARLA_EXPORT CarlaPipeClientHandle carla_pipe_client_new(const char* argv[], CarlaPipeCallbackFunc callbackFunc, void* callbackPtr)
{
    carla_debug("carla_pipe_client_new(%p, %p, %p)", argv, callbackFunc, callbackPtr);

    CarlaPipeClientPlugin* const pipe(new CarlaPipeClientPlugin(callbackFunc, callbackPtr));

    if (! pipe->initPipeClient(argv))
    {
        delete pipe;
        return nullptr;
    }

    return pipe;
}

CARLA_EXPORT void carla_pipe_client_idle(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr,);

    ((CarlaPipeClientPlugin*)handle)->idlePipe();
}

CARLA_EXPORT bool carla_pipe_client_is_running(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr, false);

    return ((CarlaPipeClientPlugin*)handle)->isPipeRunning();
}

CARLA_EXPORT void carla_pipe_client_lock(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr,);

    return ((CarlaPipeClientPlugin*)handle)->lockPipe();
}

CARLA_EXPORT void carla_pipe_client_unlock(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr,);

    return ((CarlaPipeClientPlugin*)handle)->unlockPipe();
}

CARLA_EXPORT const char* carla_pipe_client_readlineblock(CarlaPipeClientHandle handle, uint timeout)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr, nullptr);

    return ((CarlaPipeClientPlugin*)handle)->readlineblock(timeout);
}

CARLA_EXPORT bool carla_pipe_client_write_msg(CarlaPipeClientHandle handle, const char* msg)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr, false);

    return ((CarlaPipeClientPlugin*)handle)->writeMessage(msg);
}

CARLA_EXPORT bool carla_pipe_client_write_and_fix_msg(CarlaPipeClientHandle handle, const char* msg)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr, false);

    return ((CarlaPipeClientPlugin*)handle)->writeAndFixMessage(msg);
}

CARLA_EXPORT bool carla_pipe_client_flush(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr, false);

    return ((CarlaPipeClientPlugin*)handle)->flushMessages();
}

CARLA_EXPORT bool carla_pipe_client_flush_and_unlock(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr, false);

    CarlaPipeClientPlugin* const pipe((CarlaPipeClientPlugin*)handle);
    const bool ret(pipe->flushMessages());
    pipe->unlockPipe();
    return ret;
}

CARLA_EXPORT void carla_pipe_client_destroy(CarlaPipeClientHandle handle)
{
    CARLA_SAFE_ASSERT_RETURN(handle != nullptr,);
    carla_debug("carla_pipe_client_destroy(%p)", handle);

    CarlaPipeClientPlugin* const pipe((CarlaPipeClientPlugin*)handle);
    pipe->closePipeClient();
    delete pipe;
}

// -------------------------------------------------------------------------------------------------------------------

#include "CarlaPipeUtils.cpp"

// -------------------------------------------------------------------------------------------------------------------
