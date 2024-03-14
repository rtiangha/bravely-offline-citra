// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/settings.h"

namespace GameSettings {

void LoadOverrides(u64 title_id) {
    const std::array<u64, 10> core_ticks_hack_ids = {
            0x0004000000030500, // Super Street Fighter IV: 3D Edition
            0x0004000000032D00, // Super Street Fighter IV: 3D Edition
            0x0004000000033C00, // Super Street Fighter IV: 3D Edition
            0x0004000000060200, // Resident Evil: Revelations
            0x000400000005EE00, // Resident Evil: Revelations
            0x0004000000035900, // Resident Evil: The Mercenaries 3D
            0x0004000000038B00, // Resident Evil: The Mercenaries 3D
            0x00040000000C8100, // Paper Mario: Sticker Star
            0x00040000000A5E00, // Paper Mario: Sticker Star
            0x00040000000A5F00, // Paper Mario: Sticker Star
    };
    for (auto id : core_ticks_hack_ids) {
        if (title_id == id) {
            Settings::SetFMVHack(true);
            break;
        }
    }

    const std::array<u64, 10> accurate_mul_ids = {
            0x0004000000033400, // The Legend of Zelda: Ocarina of Time 3D
            0x0004000000033500, // The Legend of Zelda: Ocarina of Time 3D
            0x0004000000033600, // The Legend of Zelda: Ocarina of Time 3D
            0x000400000008F800, // The Legend of Zelda: Ocarina of Time 3D
            0x000400000008F900, // The Legend of Zelda: Ocarina of Time 3D
            0x00040000001B8F00, // Mario & Luigi: Superstar Saga + Bowsers Minions
            0x00040000001B9000, // Mario & Luigi: Superstar Saga + Bowsers Minions
            0x0004000000194B00, // Mario & Luigi: Superstar Saga + Bowsers Minions
            0x00040000001D1400, // Mario & Luigi: Bowsers Inside Story + Bowser Jrs Journey
            0x00040000001D1500, // Mario & Luigi: Bowsers Inside Story + Bowser Jrs Journey
    };
    for (auto id : accurate_mul_ids) {
        if (title_id == id) {
            Settings::values.shaders_accurate_mul = true;
            break;
        }
    }

    const std::array<u64, 12> cpu_limit_ids = {
            0x000400000007C700, // Mario Tennis Open
            0x000400000007C800, // Mario Tennis Open
            0x0004000000064D00, // Mario Tennis Open
            0x00040000000B9100, // Mario Tennis Open
            0x00040000000DCD00, // Mario Golf: World Tour
            0x00040000000A5300, // Mario Golf: World Tour
            0x00040000000DCE00, // Mario Golf: World Tour
            0x00040000001CCD00, // The Alliance Alive
            0x00040000001B4500, // The Alliance Alive
            0x0004000000120900, // Lord of Magna: Maiden Heaven
            0x0004000000164300, // Lord of Magna: Maiden Heaven
            0x000400000008FE00, // 1001 Spikes
    };
    for (auto id : cpu_limit_ids) {
        if (title_id == id) {
            Settings::values.core_downcount_hack = true;
            break;
        }
    }
}

} // namespace GameSettings
