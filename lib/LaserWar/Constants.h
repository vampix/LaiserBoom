#ifndef LWConstants_h
#define LWConstants_h

#define LW_CMD_END 0xe8

enum LwPreset {
    Assault = 0,
    Hostage = 1,
    LastHero = 2,
    Swat = 3
};

enum LWColor {
    Red = 0,
    Blue = 1,
    Yellow = 2,
    Green = 3
};

enum LwSetting {
    ChangeColor = 0xA9,
    RespawnColor = 0xB4,
    PauseColor = 0xB5,
    KillColor = 0xB6,
    FullAmmoColor = 0xB7,

    AddHealth = 0x80,
    AddAmmo = 0x81,
    AddMagazines = 0x8A,
    AddHealthMedic = 0x8B,
    RadiationDamage = 0xA0,
    AnomalyAddHealth = 0xA1,
    PlaySound = 0xAA,
    RespawnId = 0xB8,
    PauseId = 0xB9,
    KillId = 0xBA,
    FullAmmoId = 0xBB,

    ApplyPreset = 0xA8,
    AdminCommand = 0x83
};

enum LwAdminSetting {
    Kill = 0x00,
    PauseResume = 0x01,
    StartGame = 0x02,
    DefaultSettings = 0x03,
    Respawn = 0x04,
    NewGame = 0x05,
    FullAmmo = 0x06,
    EndGame = 0x07,
    ClearTime = 0x08,
    SwapColor = 0x09,
    InitPlayer = 0x0A,
    Explode = 0x0B,
    NewGame2 = 0x0C,
    FullHealth = 0x0D,
    SwapCapacity = 0x0E,
    FullAmmoPlayer = 0x0F,
    DoubleHealth = 0x10,
    CheckpointCaptured = 0x11,
    BombDeactivated = 0x12,
    ClearStatistics = 0x14,
    HeadbandTest = 0x15,
    StunPlayer = 0xE8,
    RemovePlayerWeapon = 0x17,
    IncreaseDamage = 0x20,
    Speed750 = 0x21,
    DoubleHealthOnce = 0x22
};

unsigned const char dmg_list[16] = { 1, 2, 4, 5, 7, 10, 15, 17, 20, 25, 30, 35, 40, 50, 75, 100 };
const char* color_list[4] = {"RED", "BLUE", "YELLOW", "GREEN"};
const char* preset_list[5] = {"ASSAULT", "HOSTAGE", "LAST HERO", "ZOMBIE", "SWAT"};

typedef struct {
    LwSetting code;
    const char* description;
} CommandDescription;

typedef struct {
    LwAdminSetting code;
    const char* description;
} AdminSettingDescription;

const CommandDescription colorCommands[5] = {
    { LwSetting::ChangeColor, "Change color " },
    { LwSetting::RespawnColor, "Respawn by color " },
    { LwSetting::PauseColor, "Pause by color " },
    { LwSetting::KillColor, "Kill by color " },
    { LwSetting::FullAmmoColor, "Full ammo by color " }
};

const CommandDescription dataCommands[11] = {
    { LwSetting::AddHealth, "Add health " },
    { LwSetting::AddAmmo, "Add ammo " },
    { LwSetting::AddMagazines, "Add magazines " },
    { LwSetting::AddHealthMedic, "Medic kit. Add health " },
    { LwSetting::RadiationDamage, "Radiation damage " },
    { LwSetting::AnomalyAddHealth, "Anomaly. Add health " },
    { LwSetting::PlaySound, "Play sound " },
    { LwSetting::RespawnId, "Respawn by ID " },
    { LwSetting::PauseId, "Pause by ID " },
    { LwSetting::KillId, "Kill by ID " },
    { LwSetting::FullAmmoId, "Full ammo by ID " }
};

const AdminSettingDescription commands83[26] = {
    { LwAdminSetting::Kill, "Kill" },
    { LwAdminSetting::PauseResume, "Pause/Resume" },
    { LwAdminSetting::StartGame, "Start game" },
    { LwAdminSetting::DefaultSettings, "Default settings" },
    { LwAdminSetting::Respawn, "Respawn" },
    { LwAdminSetting::NewGame, "New game" },
    { LwAdminSetting::FullAmmo, "Full ammo" },
    { LwAdminSetting::EndGame, "End game" },
    { LwAdminSetting::ClearTime, "Clear time" },
    { LwAdminSetting::SwapColor, "Change color Red <> Blue, Green/Yellow -> Red" },
    { LwAdminSetting::InitPlayer, "Init player" },
    { LwAdminSetting::Explode, "Explode player" },
    { LwAdminSetting::NewGame2, "New game" },
    { LwAdminSetting::FullHealth, "Full health" },
    { LwAdminSetting::SwapCapacity, "Capacity: <50% -> 99%, >50% -> 49%" },
    { LwAdminSetting::FullAmmoPlayer, "Full ammo" },
    { LwAdminSetting::DoubleHealth, "Double health until death" },
    { LwAdminSetting::CheckpointCaptured, "Checkpoint captured" },
    { LwAdminSetting::BombDeactivated, "Bomb deactivated" },
    { LwAdminSetting::ClearStatistics, "Clear statistics" },
    { LwAdminSetting::HeadbandTest, "Handband test" },
    { LwAdminSetting::StunPlayer, "Deactivate player" },
    { LwAdminSetting::RemovePlayerWeapon, "Remove weapon" },
    { LwAdminSetting::IncreaseDamage, "Increase damage until death" },
    { LwAdminSetting::Speed750, "Set 750 shoots per minute" },
    { LwAdminSetting::DoubleHealthOnce, "Double health once" }
};

#endif