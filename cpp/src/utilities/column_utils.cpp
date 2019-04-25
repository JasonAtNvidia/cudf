/*
 * Copyright 2019 BlazingDB, Inc.
 *     Copyright 2019 Eyal Rozenberg <eyalroz@blazingdb.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "column_utils.hpp"
#include <utilities/error_utils.hpp>

// This should go up into more general utility code...
static constexpr inline bool logical_xor(bool x, bool y) { return (x and (not y)) or ((not x) and y) ; }

namespace cudf {

/**
 * @brief Ensures a gdf_column is valid, i.e. that its fields are consistent
 * with each other, and logical in themselves, in representing a proper column.
 */
void validate(const gdf_column& column)
{
    CUDF_EXPECTS ( (column.data != nullptr or column.size == 0)          , "No column data despite there supposedly being rows.");
    CUDF_EXPECTS ( (column.dtype != GDF_invalid)                         , "Cannot use the 'invalid' column element type.");
    CUDF_EXPECTS ( (column.dtype < N_GDF_TYPES)                          , "Unknown column data type");
    CUDF_EXPECTS ( (is_nullable(column) or (column.null_count == 0))     , "Column supposedly has null values, but no place to expose its record");
    CUDF_EXPECTS ( (column.null_count <= column.size)                    , "Column claims to have more null elements than elements overall");
}

void validate(const gdf_column* column_ptr)
{
    CUDF_EXPECTS ( (column_ptr != nullptr) , "Null gdf_column pointer");
    validate(*column_ptr);
}

namespace detail {

bool extra_type_info_is_compatible(
    const gdf_dtype& common_dtype,
    const gdf_dtype_extra_info& lhs_extra_type_info,
    const gdf_dtype_extra_info& rhs_extra_type_info) noexcept
{
    switch(common_dtype) {
    // Skipping this check, for now, due to the "hackiness" of
    // how the category type is currently supported
    // case GDF_CATEGORY:  return lhs_extra_type_info.category == rhs_extra_type_info.category;
    case GDF_TIMESTAMP: return lhs_extra_type_info.time_unit == rhs_extra_type_info.time_unit;
    default:            return true;
    }
}

} // namespace detail


bool have_same_type(const gdf_column& validated_column_1, const gdf_column& validated_column_2, bool ignore_extra_type_info) noexcept
{
    if (validated_column_1.dtype != validated_column_2.dtype) { return false; }
    if (logical_xor(is_nullable(validated_column_1), is_nullable(validated_column_2))) { return false; }
    if (ignore_extra_type_info) { return true; }
    auto common_dtype = validated_column_1.dtype;
    return detail::extra_type_info_is_compatible(common_dtype, validated_column_1.dtype_info, validated_column_2.dtype_info);
}

bool have_same_type(const gdf_column* validated_column_ptr_1, const gdf_column* validated_column_ptr_2, bool ignore_extra_type_info) noexcept
{
    return have_same_type(*validated_column_ptr_1, *validated_column_ptr_2);
}

} // namespace cudf
