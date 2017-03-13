/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku source-code, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: Architecture.h
 *
 * --------------------------------------------------------------------------*/
 /*
 * Copyright 2013, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Distributed under the terms of the MIT License.
 */
#include "Architecture.h"

#include "StringList.h"
#include "StringClass.h"
#include "Errors.h"

namespace bhapi {
static const size_t k_MaxArchitectureCount = 16;
static status_t string_array_to_string_list(const char* const* architectures, size_t count, BStringList& _architectures);
}

static status_t bhapi::string_array_to_string_list(const char* const* o_architectures, size_t a_count, BStringList& a_architectures)
{
    a_architectures.MakeEmpty();

    for (size_t i = 0; i < a_count; i++) {
        BString architecture(o_architectures[i]);
        if (architecture.IsEmpty() || !a_architectures.Add(architecture)) {
            a_architectures.MakeEmpty();
            return B_NO_MEMORY;
        }
    }
    return B_OK;
}

status_t bhapi::__get_secondary_architectures(BStringList& a_architectures)
{
    const char* architectures[k_MaxArchitectureCount];
    size_t count = get_secondary_architectures(architectures,
        k_MaxArchitectureCount);
    return string_array_to_string_list(architectures,
        std::min(count, k_MaxArchitectureCount), a_architectures);
}

status_t bhapi::__get_architectures(BStringList& a_architectures)
{
    const char* architectures[k_MaxArchitectureCount];
    size_t count = get_architectures(architectures, k_MaxArchitectureCount);
    return string_array_to_string_list(architectures,
        std::min(count, k_MaxArchitectureCount), a_architectures);
}
