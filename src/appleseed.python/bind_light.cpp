
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2012-2013 Esteban Tovagliari, Jupiter Jazz Limited
// Copyright (c) 2014 Esteban Tovagliari, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Has to be first, to avoid redefinition warnings.
#include "bind_auto_release_ptr.h"

// appleseed.python headers.
#include "bind_typed_entity_containers.h"
#include "dict2dict.h"
#include "unaligned_transformd44.h"

// appleseed.renderer headers.
#include "renderer/api/light.h"

namespace bpy = boost::python;
using namespace foundation;
using namespace renderer;

namespace detail
{
    auto_release_ptr<Light> create_light(const std::string& light_type,
                                         const std::string& name,
                                         const bpy::dict& params)
    {
        LightFactoryRegistrar factories;
        const ILightFactory* factory = factories.lookup(light_type.c_str());

        if (factory)
            return factory->create(name.c_str(), bpy_dict_to_param_array(params));
        else
        {
            PyErr_SetString(PyExc_RuntimeError, "Light type not found");
            bpy::throw_error_already_set();
        }

        return auto_release_ptr<Light>();
    }

    UnalignedTransformd44 light_get_transform(const Light* l)
    {
        return UnalignedTransformd44(l->get_transform());
    }

    void light_set_transform(Light* l, const UnalignedTransformd44& xform)
    {
        l->set_transform(xform.as_foundation_transform());
    }
}

void bind_light()
{
    bpy::class_<Light, auto_release_ptr<Light>, bpy::bases<ConnectableEntity>, boost::noncopyable>("Light", bpy::no_init)
        .def("__init__", bpy::make_constructor(detail::create_light))
        .def("set_transform", &detail::light_set_transform)
        .def("get_transform", &detail::light_get_transform);

    bind_typed_entity_vector<Light>("LightContainer");
}
