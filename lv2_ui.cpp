/*
 * MODGUI X11UI, based on Carla code
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

#include "CarlaExternalUI.hpp"
#include "CarlaPipeUtils.cpp"

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

// -----------------------------------------------------------------------
// C++ class to handle stuff from the host

class MODEmbedExternalUI : public CarlaExternalUI
{
public:
    MODEmbedExternalUI(const LV2UI_Controller controller, const LV2UI_Write_Function writeFunction, const LV2UI_Resize* resize,
          const char* const bundlePath, const char* const pluginURI, const uintptr_t parentId) noexcept
        : CarlaExternalUI(),
          fController(controller),
          fWriteFunction(writeFunction),
          fResize(resize)
    {
        setData(CarlaString(bundlePath) + CARLA_OS_SEP_STR "modgui-x11", pluginURI, CarlaString(parentId));
    }

    void lv2ui_port_event(uint32_t portIndex, uint32_t bufferSize, uint32_t format, const void* buffer) const
    {
        if (format != 0 || bufferSize != sizeof(float) || buffer == nullptr)
            return;

        const float value(*(const float*)buffer);

        writeControlMessage(portIndex, value);
    }

    int lv2ui_idle()
    {
        CARLA_SAFE_ASSERT_RETURN(isPipeRunning(), 1);

        idlePipe();

        switch (getAndResetUiState())
        {
        case CarlaExternalUI::UiCrashed:
        case CarlaExternalUI::UiHide:
            return 1;
        default:
            return 0;
        }
    }

    int lv2ui_show()
    {
        writeShowMessage();
        return 0;
    }

    int lv2ui_hide()
    {
        writeHideMessage();
        return 0;
    }

protected:
    // -------------------------------------------------------------------
    // Pipe Server calls

    bool msgReceived(const char* const msg) noexcept override
    {
        if (CarlaExternalUI::msgReceived(msg))
            return true;

        if (std::strcmp(msg, "control") == 0)
        {
            uint32_t param;
            float value;

            CARLA_SAFE_ASSERT_RETURN(readNextLineAsUInt(param), true);
            CARLA_SAFE_ASSERT_RETURN(readNextLineAsFloat(value), true);

            fWriteFunction(fController, param, sizeof(float), 0, &value);

            return true;
        }

        if (std::strcmp(msg, "size") == 0)
        {
            uint width, height;

            CARLA_SAFE_ASSERT_RETURN(readNextLineAsUInt(width), true);
            CARLA_SAFE_ASSERT_RETURN(readNextLineAsUInt(height), true);

            if (fResize != nullptr)
                fResize->ui_resize(fResize->handle, width, height);

            return true;
        }

        return false;
    }

private:
    const LV2UI_Controller     fController;
    const LV2UI_Write_Function fWriteFunction;
    const LV2UI_Resize*        fResize;
};

// -----------------------------------------------------------------------
// LV2 UI descriptor functions

static LV2UI_Handle lv2ui_instantiate(const LV2UI_Descriptor*, const char* pluginURI, const char* bundlePath,
                                      LV2UI_Write_Function writeFunction, LV2UI_Controller controller,
                                      LV2UI_Widget* widget, const LV2_Feature* const* features)
{
    CARLA_SAFE_ASSERT_RETURN(writeFunction != nullptr, nullptr);

    const LV2UI_Resize* resize   = nullptr;
    /* */ uintptr_t     parentId = 0;

    for (int i=0; features[i] != nullptr; ++i)
    {
        if (std::strcmp(features[i]->URI, LV2_UI__parent) == 0)
        {
            parentId = (uintptr_t)features[i]->data;
        }
        else if (std::strcmp(features[i]->URI, LV2_UI__resize) == 0)
        {
            resize = (const LV2UI_Resize*)features[i]->data;
            resize->ui_resize(resize->handle, 1, 1);
        }
    }

    CARLA_SAFE_ASSERT_RETURN(parentId != 0, nullptr);

    MODEmbedExternalUI* const thing(new MODEmbedExternalUI(controller, writeFunction, resize,
                                                           bundlePath, pluginURI, parentId));

    if (! thing->startPipeServer(false))
    {
        delete thing;
        return nullptr;
    }

    if (widget != nullptr)
        *widget = nullptr;

    return thing;
}

#define uiPtr ((MODEmbedExternalUI*)ui)

static void lv2ui_port_event(LV2UI_Handle ui, uint32_t portIndex, uint32_t bufferSize, uint32_t format, const void* buffer)
{
    uiPtr->lv2ui_port_event(portIndex, bufferSize, format, buffer);
}

static void lv2ui_cleanup(LV2UI_Handle ui)
{
    delete uiPtr;
}

static int lv2ui_idle(LV2UI_Handle ui)
{
    return uiPtr->lv2ui_idle();
}

static int lv2ui_show(LV2UI_Handle ui)
{
    return uiPtr->lv2ui_show();
}

static int lv2ui_hide(LV2UI_Handle ui)
{
    return uiPtr->lv2ui_hide();
}

static const void* lv2ui_extension_data(const char* uri)
{
    static const LV2UI_Idle_Interface uiidle = { lv2ui_idle };
    static const LV2UI_Show_Interface uishow = { lv2ui_show, lv2ui_hide };

    if (std::strcmp(uri, LV2_UI__idleInterface) == 0)
        return &uiidle;
    if (std::strcmp(uri, LV2_UI__showInterface) == 0)
        return &uishow;

    return nullptr;
}

#undef uiPtr

// -----------------------------------------------------------------------
// Startup code

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index)
{
    static const LV2UI_Descriptor lv2UiDesc = {
    /* URI            */ "http://portalmod.com/ns/modgui#X11UI",
    /* instantiate    */ lv2ui_instantiate,
    /* cleanup        */ lv2ui_cleanup,
    /* port_event     */ lv2ui_port_event,
    /* extension_data */ lv2ui_extension_data
    };

    return (index == 0) ? &lv2UiDesc : nullptr;
};

// -----------------------------------------------------------------------
