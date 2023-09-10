#pragma once

#include <ntifs.h>
#include <ntddk.h>
#include <windef.h>
#include <ntimage.h>
#include <tuple>
#include <memory>
#include <communication.h>
#include <crypt.h>

#include "structs.h"
#include "definitions.h"

#include "utilities/raii.h"
#include "utilities/stl.h"
#include "utilities/util.h"
#include "utilities/memory.h"