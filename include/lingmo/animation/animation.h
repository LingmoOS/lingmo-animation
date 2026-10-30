// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// The animation primitives the Lingmo desktop shares. Nothing here knows
// about windows, toolkits or the compositor: a driver calls advance() with
// the time that passed, and the primitives do the rest.

#include "lingmo/animation/advanceable.h"
#include "lingmo/animation/curve.h"
#include "lingmo/animation/hop.h"
#include "lingmo/animation/smoothing.h"
#include "lingmo/animation/spring.h"
#include "lingmo/animation/stepper.h"
#include "lingmo/animation/timeline.h"
#include "lingmo/animation/tween.h"
