#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_ai_script_commands.h"
#include "battle_arena.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_main.h"
#include "battle_message.h"
#include "battle_pyramid.h"
#include "battle_scripts.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "battle_util.h"
#include "berry.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "dma3.h"
#include "event_data.h"
#include "evolution_scene.h"
#include "graphics.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item.h"
#include "link.h"
#include "link_rfu.h"
#include "load_save.h"
#include "main.h"
#include "malloc.h"
#include "m4a.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "recorded_battle.h"
#include "roamer.h"
#include "safari_zone.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "trig.h"
#include "tv.h"
#include "util.h"
#include "wild_encounter.h"
#include "window.h"
#include "constants/abilities.h"
#include "constants/battle_config.h"
#include "constants/battle_move_effects.h"
#include "constants/battle_string_ids.h"
#include "constants/hold_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "cable_club.h"
#include "debug.h"
#include "done_button.h"
#include "speedchoice.h"

extern struct MusicPlayerInfo gMPlayInfo_SE1;
extern struct MusicPlayerInfo gMPlayInfo_SE2;

extern const struct BgTemplate gBattleBgTemplates[];
extern const struct WindowTemplate *const gBattleWindowTemplates[];

// this file's functions
static void CB2_InitBattleInternal(void);
static void CB2_PreInitMultiBattle(void);
static void CB2_PreInitIngamePlayerPartnerBattle(void);
static void CB2_HandleStartMultiPartnerBattle(void);
static void CB2_HandleStartMultiBattle(void);
static void CB2_HandleStartBattle(void);
static void TryCorrectShedinjaLanguage(struct Pokemon *mon);
static u8 CreateNPCTrainerParty(struct Pokemon *party, u16 trainerNum, bool8 firstTrainer);
static void BattleMainCB1(void);
static void sub_8038538(struct Sprite *sprite);
static void CB2_EndLinkBattle(void);
static void EndLinkBattleInSteps(void);
static void sub_80392A8(void);
static void sub_803937C(void);
static void sub_803939C(void);
static void SpriteCb_MoveWildMonToRight(struct Sprite *sprite);
static void SpriteCb_WildMonShowHealthbox(struct Sprite *sprite);
static void SpriteCb_WildMonAnimate(struct Sprite *sprite);
static void sub_80398D0(struct Sprite *sprite);
static void SpriteCB_AnimFaintOpponent(struct Sprite *sprite);
static void SpriteCb_BlinkVisible(struct Sprite *sprite);
static void SpriteCallbackDummy_3(struct Sprite *sprite);
static void SpriteCB_BattleSpriteSlideLeft(struct Sprite *sprite);
static void TurnValuesCleanUp(bool8 var0);
static void SpriteCB_BounceEffect(struct Sprite *sprite);
static void BattleStartClearSetData(void);
static void DoBattleIntro(void);
static void TryDoEventsBeforeFirstTurn(void);
static void HandleTurnActionSelectionState(void);
static void RunTurnActionsFunctions(void);
static void SetActionsAndBattlersTurnOrder(void);
static void sub_803CDF8(void);
static bool8 AllAtActionConfirmed(void);
static void CheckFocusPunch_ClearVarsBeforeTurnStarts(void);
static void CheckMegaEvolutionBeforeTurn(void);
static void CheckQuickClaw_CustapBerryActivation(void);
static void FreeResetData_ReturnToOvOrDoEvolutions(void);
static void ReturnFromBattleToOverworld(void);
static void TryEvolvePokemon(void);
static void WaitForEvoSceneToFinish(void);
static void HandleEndTurn_ContinueBattle(void);
static void HandleEndTurn_BattleWon(void);
static void HandleEndTurn_BattleLost(void);
static void HandleEndTurn_RanFromBattle(void);
static void HandleEndTurn_MonFled(void);
static void HandleEndTurn_FinishBattle(void);

// EWRAM vars
EWRAM_DATA u16 gBattle_BG0_X = 0;
EWRAM_DATA u16 gBattle_BG0_Y = 0;
EWRAM_DATA u16 gBattle_BG1_X = 0;
EWRAM_DATA u16 gBattle_BG1_Y = 0;
EWRAM_DATA u16 gBattle_BG2_X = 0;
EWRAM_DATA u16 gBattle_BG2_Y = 0;
EWRAM_DATA u16 gBattle_BG3_X = 0;
EWRAM_DATA u16 gBattle_BG3_Y = 0;
EWRAM_DATA u16 gBattle_WIN0H = 0;
EWRAM_DATA u16 gBattle_WIN0V = 0;
EWRAM_DATA u16 gBattle_WIN1H = 0;
EWRAM_DATA u16 gBattle_WIN1V = 0;
EWRAM_DATA u8 gDisplayedStringBattle[400] = {0};
EWRAM_DATA u8 gBattleTextBuff1[TEXT_BUFF_ARRAY_COUNT] = {0};
EWRAM_DATA u8 gBattleTextBuff2[TEXT_BUFF_ARRAY_COUNT] = {0};
EWRAM_DATA u8 gBattleTextBuff3[30] = {0};   //expanded for stupidly long z move names
EWRAM_DATA u32 gBattleTypeFlags = 0;
EWRAM_DATA u8 gBattleTerrain = 0;
EWRAM_DATA u32 gUnusedFirstBattleVar1 = 0; // Never read
EWRAM_DATA struct UnknownPokemonStruct4 gMultiPartnerParty[MULTI_PARTY_SIZE] = {0};
EWRAM_DATA static struct UnknownPokemonStruct4* sMultiPartnerPartyBuffer = NULL;
EWRAM_DATA u8 *gUnknown_0202305C = NULL;
EWRAM_DATA u8 *gUnknown_02023060 = NULL;
EWRAM_DATA u8 gActiveBattler = 0;
EWRAM_DATA u32 gBattleControllerExecFlags = 0;
EWRAM_DATA u8 gBattlersCount = 0;
EWRAM_DATA u16 gBattlerPartyIndexes[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerPositions[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gActionsByTurnOrder[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerByTurnOrder[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gCurrentTurnActionNumber = 0;
EWRAM_DATA u8 gCurrentActionFuncId = 0;
EWRAM_DATA struct BattlePokemon gBattleMons[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerSpriteIds[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gCurrMovePos = 0;
EWRAM_DATA u8 gChosenMovePos = 0;
EWRAM_DATA u16 gCurrentMove = 0;
EWRAM_DATA u16 gChosenMove = 0;
EWRAM_DATA u16 gCalledMove = 0;
EWRAM_DATA s32 gBattleMoveDamage = 0;
EWRAM_DATA s32 gHpDealt = 0;
EWRAM_DATA s32 gTakenDmg[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastUsedItem = 0;
EWRAM_DATA u16 gLastUsedAbility = 0;
EWRAM_DATA u8 gBattlerAttacker = 0;
EWRAM_DATA u8 gBattlerTarget = 0;
EWRAM_DATA u8 gBattlerFainted = 0;
EWRAM_DATA u8 gEffectBattler = 0;
EWRAM_DATA u8 gPotentialItemEffectBattler = 0;
EWRAM_DATA u8 gAbsentBattlerFlags = 0;
EWRAM_DATA u8 gIsCriticalHit = FALSE;
EWRAM_DATA u8 gMultiHitCounter = 0;
EWRAM_DATA const u8 *gBattlescriptCurrInstr = NULL;
EWRAM_DATA u8 gChosenActionByBattler[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA const u8 *gSelectionBattleScripts[MAX_BATTLERS_COUNT] = {NULL};
EWRAM_DATA const u8 *gPalaceSelectionBattleScripts[MAX_BATTLERS_COUNT] = {NULL};
EWRAM_DATA u16 gLastPrintedMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastLandedMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastHitByType[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastResultingMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLockedMoves[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gLastUsedMove = 0;
EWRAM_DATA u8 gLastHitBy[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gChosenMoveByBattler[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gMoveResultFlags = 0;
EWRAM_DATA u32 gHitMarker = 0;
EWRAM_DATA u8 gTakenDmgByBattler[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gUnusedFirstBattleVar2 = 0; // Never read
EWRAM_DATA u32 gSideStatuses[2] = {0};
EWRAM_DATA struct SideTimer gSideTimers[2] = {0};
EWRAM_DATA u32 gStatuses3[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA struct DisableStruct gDisableStructs[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gPauseCounterBattle = 0;
EWRAM_DATA u16 gPaydayMoney = 0;
EWRAM_DATA u16 gRandomTurnNumber = 0;
EWRAM_DATA u8 gBattleCommunication[BATTLE_COMMUNICATION_ENTRIES_COUNT] = {0};
EWRAM_DATA u8 gBattleOutcome = 0;
EWRAM_DATA struct ProtectStruct gProtectStructs[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA struct SpecialStatus gSpecialStatuses[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u16 gBattleWeather = 0;
EWRAM_DATA struct WishFutureKnock gWishFutureKnock = {0};
EWRAM_DATA u16 gIntroSlideFlags = 0;
EWRAM_DATA u8 gSentPokesToOpponent[2] = {0};
EWRAM_DATA u16 gExpShareExp = 0;
EWRAM_DATA struct BattleEnigmaBerry gEnigmaBerries[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA struct BattleScripting gBattleScripting = {0};
EWRAM_DATA struct BattleStruct *gBattleStruct = NULL;
EWRAM_DATA u8 *gLinkBattleSendBuffer = NULL;
EWRAM_DATA u8 *gLinkBattleRecvBuffer = NULL;
EWRAM_DATA struct BattleResources *gBattleResources = NULL;
EWRAM_DATA u8 gActionSelectionCursor[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gMoveSelectionCursor[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerStatusSummaryTaskId[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gBattlerInMenuId = 0;
EWRAM_DATA bool8 gDoingBattleAnim = FALSE;
EWRAM_DATA u32 gTransformedPersonalities[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u8 gPlayerDpadHoldFrames = 0;
EWRAM_DATA struct BattleSpriteData *gBattleSpritesDataPtr = NULL;
EWRAM_DATA struct MonSpritesGfx *gMonSpritesGfxPtr = NULL;
EWRAM_DATA struct BattleHealthboxInfo *gBattleControllerOpponentHealthboxData = NULL; // Never read
EWRAM_DATA struct BattleHealthboxInfo *gBattleControllerOpponentFlankHealthboxData = NULL; // Never read
EWRAM_DATA u16 gBattleMovePower = 0;
EWRAM_DATA u16 gMoveToLearn = 0;
EWRAM_DATA u8 gBattleMonForms[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA u32 gFieldStatuses = 0;
EWRAM_DATA struct FieldTimer gFieldTimers = {0};
EWRAM_DATA u8 gBattlerAbility = 0;
EWRAM_DATA u16 gPartnerSpriteId = 0;
EWRAM_DATA struct TotemBoost gTotemBoosts[MAX_BATTLERS_COUNT] = {0};
EWRAM_DATA bool8 gHasFetchedBall = FALSE;
EWRAM_DATA u8 gLastUsedBall = 0;

// IWRAM common vars
void (*gPreBattleCallback1)(void);
void (*gBattleMainFunc)(void);
struct BattleResults gBattleResults;
u8 gLeveledUpInBattle;
void (*gBattlerControllerFuncs[MAX_BATTLERS_COUNT])(void);
u8 gHealthboxSpriteIds[MAX_BATTLERS_COUNT];
u8 gMultiUsePlayerCursor;
u8 gNumberOfMovesToChoose;
u8 gBattleControllerData[MAX_BATTLERS_COUNT]; // Used by the battle controllers to store misc sprite/task IDs for each battler

// rom const data
static const struct ScanlineEffectParams sIntroScanlineParams16Bit =
{
    (void *)REG_ADDR_BG3HOFS, SCANLINE_EFFECT_DMACNT_16BIT, 1
};

// unused
static const struct ScanlineEffectParams sIntroScanlineParams32Bit =
{
    (void *)REG_ADDR_BG3HOFS, SCANLINE_EFFECT_DMACNT_32BIT, 1
};

const struct SpriteTemplate gUnknown_0831AC88 =
{
    .tileTag = 0,
    .paletteTag = 0,
    .oam = &gDummyOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = sub_8038528,
};

static const u8 sText_ShedinjaJpnName[] = _("ヌケニン"); // Nukenin

const struct OamData gOamData_BattleSpriteOpponentSide =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 0,
    .affineParam = 0,
};

const struct OamData gOamData_BattleSpritePlayerSide =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_NORMAL,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 2,
    .affineParam = 0,
};

static const s8 gUnknown_0831ACE0[] ={-32, -16, -16, -32, -32, 0, 0, 0};

const u8 gTypeNames[NUMBER_OF_MON_TYPES][TYPE_NAME_LENGTH + 1] =
{
    [TYPE_NORMAL] = _("NORMAL"),
    [TYPE_FIGHTING] = _("FIGHT"),
    [TYPE_FLYING] = _("FLYING"),
    [TYPE_POISON] = _("POISON"),
    [TYPE_GROUND] = _("GROUND"),
    [TYPE_ROCK] = _("ROCK"),
    [TYPE_BUG] = _("BUG"),
    [TYPE_GHOST] = _("GHOST"),
    [TYPE_STEEL] = _("STEEL"),
    [TYPE_MYSTERY] = _("???"),
    [TYPE_FIRE] = _("FIRE"),
    [TYPE_WATER] = _("WATER"),
    [TYPE_GRASS] = _("GRASS"),
    [TYPE_ELECTRIC] = _("ELECTR"),
    [TYPE_PSYCHIC] = _("PSYCHC"),
    [TYPE_ICE] = _("ICE"),
    [TYPE_DRAGON] = _("DRAGON"),
    [TYPE_DARK] = _("DARK"),
    [TYPE_FAIRY] = _("FAIRY"),
};

// This is a factor in how much money you get for beating a trainer.
const struct TrainerMoney gTrainerMoneyTable[] =
{
    {TRAINER_CLASS_TEAM_AQUA, 5},
    {TRAINER_CLASS_AQUA_ADMIN, 10},
    {TRAINER_CLASS_AQUA_LEADER, 20},
    {TRAINER_CLASS_AROMA_LADY, 10},
    {TRAINER_CLASS_RUIN_MANIAC, 15},
    {TRAINER_CLASS_INTERVIEWER, 12},
    {TRAINER_CLASS_TUBER_F, 1},
    {TRAINER_CLASS_TUBER_M, 1},
    {TRAINER_CLASS_SIS_AND_BRO, 3},
    {TRAINER_CLASS_COOLTRAINER, 12},
    {TRAINER_CLASS_HEX_MANIAC, 6},
    {TRAINER_CLASS_LADY, 50},
    {TRAINER_CLASS_BEAUTY, 20},
    {TRAINER_CLASS_RICH_BOY, 50},
    {TRAINER_CLASS_POKEMANIAC, 15},
    {TRAINER_CLASS_SWIMMER_M, 2},
    {TRAINER_CLASS_BLACK_BELT, 8},
    {TRAINER_CLASS_GUITARIST, 8},
    {TRAINER_CLASS_KINDLER, 8},
    {TRAINER_CLASS_CAMPER, 4},
    {TRAINER_CLASS_OLD_COUPLE, 10},
    {TRAINER_CLASS_BUG_MANIAC, 15},
    {TRAINER_CLASS_PSYCHIC, 6},
    {TRAINER_CLASS_GENTLEMAN, 20},
    {TRAINER_CLASS_ELITE_FOUR, 25},
    {TRAINER_CLASS_LEADER, 25},
    {TRAINER_CLASS_SCHOOL_KID, 5},
    {TRAINER_CLASS_SR_AND_JR, 4},
    {TRAINER_CLASS_POKEFAN, 20},
    {TRAINER_CLASS_EXPERT, 10},
    {TRAINER_CLASS_YOUNGSTER, 4},
    {TRAINER_CLASS_CHAMPION, 50},
    {TRAINER_CLASS_FISHERMAN, 10},
    {TRAINER_CLASS_TRIATHLETE, 10},
    {TRAINER_CLASS_DRAGON_TAMER, 12},
    {TRAINER_CLASS_BIRD_KEEPER, 8},
    {TRAINER_CLASS_NINJA_BOY, 3},
    {TRAINER_CLASS_BATTLE_GIRL, 6},
    {TRAINER_CLASS_PARASOL_LADY, 10},
    {TRAINER_CLASS_SWIMMER_F, 2},
    {TRAINER_CLASS_PICNICKER, 4},
    {TRAINER_CLASS_TWINS, 3},
    {TRAINER_CLASS_SAILOR, 8},
    {TRAINER_CLASS_COLLECTOR, 15},
    {TRAINER_CLASS_PKMN_TRAINER_3, 15},
    {TRAINER_CLASS_PKMN_BREEDER, 10},
    {TRAINER_CLASS_PKMN_RANGER, 12},
    {TRAINER_CLASS_TEAM_MAGMA, 5},
    {TRAINER_CLASS_MAGMA_ADMIN, 10},
    {TRAINER_CLASS_MAGMA_LEADER, 20},
    {TRAINER_CLASS_LASS, 4},
    {TRAINER_CLASS_BUG_CATCHER, 4},
    {TRAINER_CLASS_HIKER, 10},
    {TRAINER_CLASS_YOUNG_COUPLE, 8},
    {TRAINER_CLASS_WINSTRATE, 10},
    {0xFF, 5},
};

#include "data/text/abilities.h"

static void (* const sTurnActionsFuncsTable[])(void) =
{
    [B_ACTION_USE_MOVE] = HandleAction_UseMove,
    [B_ACTION_USE_ITEM] = HandleAction_UseItem,
    [B_ACTION_SWITCH] = HandleAction_Switch,
    [B_ACTION_RUN] = HandleAction_Run,
    [B_ACTION_SAFARI_WATCH_CAREFULLY] = HandleAction_WatchesCarefully,
    [B_ACTION_SAFARI_BALL] = HandleAction_SafariZoneBallThrow,
    [B_ACTION_SAFARI_POKEBLOCK] = HandleAction_ThrowPokeblock,
    [B_ACTION_SAFARI_GO_NEAR] = HandleAction_GoNear,
    [B_ACTION_SAFARI_RUN] = HandleAction_SafariZoneRun,
    [B_ACTION_WALLY_THROW] = HandleAction_WallyBallThrow,
    [B_ACTION_EXEC_SCRIPT] = HandleAction_RunBattleScript,
    [B_ACTION_TRY_FINISH] = HandleAction_TryFinish,
    [B_ACTION_FINISHED] = HandleAction_ActionFinished,
    [B_ACTION_NOTHING_FAINTED] = HandleAction_NothingIsFainted,
    [B_ACTION_THROW_BALL] = HandleAction_ThrowBall,
};

static void (* const sEndTurnFuncsTable[])(void) =
{
    [0] = HandleEndTurn_ContinueBattle, //B_OUTCOME_NONE?
    [B_OUTCOME_WON] = HandleEndTurn_BattleWon,
    [B_OUTCOME_LOST] = HandleEndTurn_BattleLost,
    [B_OUTCOME_DREW] = HandleEndTurn_BattleLost,
    [B_OUTCOME_RAN] = HandleEndTurn_RanFromBattle,
    [B_OUTCOME_PLAYER_TELEPORTED] = HandleEndTurn_FinishBattle,
    [B_OUTCOME_MON_FLED] = HandleEndTurn_MonFled,
    [B_OUTCOME_CAUGHT] = HandleEndTurn_FinishBattle,
    [B_OUTCOME_NO_SAFARI_BALLS] = HandleEndTurn_FinishBattle,
    [B_OUTCOME_FORFEITED] = HandleEndTurn_FinishBattle,
    [B_OUTCOME_MON_TELEPORTED] = HandleEndTurn_FinishBattle,
};

const u8 gStatusConditionString_PoisonJpn[8] = _("どく$$$$$");
const u8 gStatusConditionString_SleepJpn[8] = _("ねむり$$$$");
const u8 gStatusConditionString_ParalysisJpn[8] = _("まひ$$$$$");
const u8 gStatusConditionString_BurnJpn[8] = _("やけど$$$$");
const u8 gStatusConditionString_IceJpn[8] = _("こおり$$$$");
const u8 gStatusConditionString_ConfusionJpn[8] = _("こんらん$$$");
const u8 gStatusConditionString_LoveJpn[8] = _("メロメロ$$$");

const u8 * const gStatusConditionStringsTable[7][2] =
{
    {gStatusConditionString_PoisonJpn, gText_Poison},
    {gStatusConditionString_SleepJpn, gText_Sleep},
    {gStatusConditionString_ParalysisJpn, gText_Paralysis},
    {gStatusConditionString_BurnJpn, gText_Burn},
    {gStatusConditionString_IceJpn, gText_Ice},
    {gStatusConditionString_ConfusionJpn, gText_Confusion},
    {gStatusConditionString_LoveJpn, gText_Love}
};

// code
void CB2_InitBattle(void)
{
    MoveSaveBlocks_ResetHeap();
    AllocateBattleResources();
    AllocateBattleSpritesData();
    AllocateMonSpritesGfx();
    RecordedBattle_ClearFrontierPassFlag();
    sInSubMenu = FALSE;
    sInField = FALSE;
    sInBattle = TRUE;

    if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
    {
        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
        {
            CB2_InitBattleInternal();
        }
        else if (!(gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER))
        {
            HandleLinkBattleSetup();
            SetMainCallback2(CB2_PreInitMultiBattle);
        }
        else
        {
            SetMainCallback2(CB2_PreInitIngamePlayerPartnerBattle);
        }
        gBattleCommunication[MULTIUSE_STATE] = 0;
    }
    else
    {
        CB2_InitBattleInternal();
    }
}

static void CB2_InitBattleInternal(void)
{
    s32 i;

    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);

    CpuFill32(0, (void*)(VRAM), VRAM_SIZE);

    SetGpuReg(REG_OFFSET_MOSAIC, 0);
    SetGpuReg(REG_OFFSET_WIN0H, DISPLAY_WIDTH);
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 2 + 1));
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);

    gBattle_WIN0H = DISPLAY_WIDTH;

    if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && gPartnerTrainerId != TRAINER_STEVEN_PARTNER && gPartnerTrainerId < TRAINER_CUSTOM_PARTNER)
    {
        gBattle_WIN0V = DISPLAY_HEIGHT - 1;
        gBattle_WIN1H = DISPLAY_WIDTH;
        gBattle_WIN1V = 32;
    }
    else
    {
        gBattle_WIN0V = WIN_RANGE(DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 2 + 1);
        ScanlineEffect_Clear();

        i = 0;
        while (i < 80)
        {
            gScanlineEffectRegBuffers[0][i] = 0xF0;
            gScanlineEffectRegBuffers[1][i] = 0xF0;
            i++;
        }

        while (i < 160)
        {
            gScanlineEffectRegBuffers[0][i] = 0xFF10;
            gScanlineEffectRegBuffers[1][i] = 0xFF10;
            i++;
        }

        ScanlineEffect_SetParams(sIntroScanlineParams16Bit);
    }

    ResetPaletteFade();
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    gBattle_BG1_X = 0;
    gBattle_BG1_Y = 0;
    gBattle_BG2_X = 0;
    gBattle_BG2_Y = 0;
    gBattle_BG3_X = 0;
    gBattle_BG3_Y = 0;

    gBattleTerrain = BattleSetup_GetTerrainId();
    if (gBattleTypeFlags & BATTLE_TYPE_RECORDED)
        gBattleTerrain = BATTLE_TERRAIN_BUILDING;

    InitBattleBgsVideo();
    LoadBattleTextboxAndBackground();
    ResetSpriteData();
    ResetTasks();
    DrawBattleEntryBackground();
    FreeAllSpritePalettes();
    gReservedSpritePaletteCount = 4;
    SetVBlankCallback(VBlankCB_Battle);
    SetUpBattleVarsAndBirchZigzagoon();

    if (gBattleTypeFlags & BATTLE_TYPE_MULTI && gBattleTypeFlags & BATTLE_TYPE_BATTLE_TOWER)
        SetMainCallback2(CB2_HandleStartMultiPartnerBattle);
    else if (gBattleTypeFlags & BATTLE_TYPE_MULTI && gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
        SetMainCallback2(CB2_HandleStartMultiPartnerBattle);
    else if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
        SetMainCallback2(CB2_HandleStartMultiBattle);
    else
        SetMainCallback2(CB2_HandleStartBattle);

    if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED)))
    {
        CreateNPCTrainerParty(&gEnemyParty[0], gTrainerBattleOpponent_A, TRUE);
        if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS && !BATTLE_TWO_VS_ONE_OPPONENT)
            CreateNPCTrainerParty(&gEnemyParty[3], gTrainerBattleOpponent_B, FALSE);
        SetWildMonHeldItem();
    }

    gMain.inBattle = TRUE;
    gSaveBlock2Ptr->frontier.disableRecordBattle = FALSE;

    for (i = 0; i < PARTY_SIZE; i++)
        AdjustFriendship(&gPlayerParty[i], FRIENDSHIP_EVENT_LEAGUE_BATTLE);

    gBattleCommunication[MULTIUSE_STATE] = 0;
}

#define BUFFER_PARTY_VS_SCREEN_STATUS(party, flags, i)              \
    for ((i) = 0; (i) < PARTY_SIZE; (i)++)                          \
    {                                                               \
        u16 species = GetMonData(&(party)[(i)], MON_DATA_SPECIES2); \
        u16 hp = GetMonData(&(party)[(i)], MON_DATA_HP);            \
        u32 status = GetMonData(&(party)[(i)], MON_DATA_STATUS);    \
                                                                    \
        if (species == SPECIES_NONE)                                \
            continue;                                               \
                                                                    \
        /* Is healthy mon? */                                       \
        if (species != SPECIES_EGG && hp != 0 && status == 0)       \
            (flags) |= 1 << (i) * 2;                                \
                                                                    \
        if (species == SPECIES_NONE) /* Redundant */                \
            continue;                                               \
                                                                    \
        /* Is Egg or statused? */                                   \
        if (hp != 0 && (species == SPECIES_EGG || status != 0))     \
            (flags) |= 2 << (i) * 2;                                \
                                                                    \
        if (species == SPECIES_NONE) /* Redundant */                \
            continue;                                               \
                                                                    \
        /* Is fainted? */                                           \
        if (species != SPECIES_EGG && hp == 0)                      \
            (flags) |= 3 << (i) * 2;                                \
    }

// For Vs Screen at link battle start
static void BufferPartyVsScreenHealth_AtStart(void)
{
    u16 flags = 0;
    s32 i;

    BUFFER_PARTY_VS_SCREEN_STATUS(gPlayerParty, flags, i);
    gBattleStruct->multiBuffer.linkBattlerHeader.vsScreenHealthFlagsLo = flags;
    *(&gBattleStruct->multiBuffer.linkBattlerHeader.vsScreenHealthFlagsHi) = flags >> 8;
    gBattleStruct->multiBuffer.linkBattlerHeader.vsScreenHealthFlagsHi |= FlagGet(FLAG_SYS_FRONTIER_PASS) << 7;
}

static void SetPlayerBerryDataInBattleStruct(void)
{
    s32 i;
    struct BattleStruct *battleStruct = gBattleStruct;
    struct BattleEnigmaBerry *battleBerry = &battleStruct->multiBuffer.linkBattlerHeader.battleEnigmaBerry;

    if (IsEnigmaBerryValid() == TRUE)
    {
        for (i = 0; i < BERRY_NAME_LENGTH; i++)
            battleBerry->name[i] = gSaveBlock1Ptr->enigmaBerry.berry.name[i];
        battleBerry->name[i] = EOS;

        for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; i++)
            battleBerry->itemEffect[i] = gSaveBlock1Ptr->enigmaBerry.itemEffect[i];

        battleBerry->holdEffect = gSaveBlock1Ptr->enigmaBerry.holdEffect;
        battleBerry->holdEffectParam = gSaveBlock1Ptr->enigmaBerry.holdEffectParam;
    }
    else
    {
        const struct Berry *berryData = GetBerryInfo(ItemIdToBerryType(ITEM_ENIGMA_BERRY));

        for (i = 0; i < BERRY_NAME_LENGTH; i++)
            battleBerry->name[i] = berryData->name[i];
        battleBerry->name[i] = EOS;

        for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; i++)
            battleBerry->itemEffect[i] = 0;

        battleBerry->holdEffect = HOLD_EFFECT_NONE;
        battleBerry->holdEffectParam = 0;
    }
}

static void SetAllPlayersBerryData(void)
{
    s32 i;
    s32 j;

    if (!(gBattleTypeFlags & BATTLE_TYPE_LINK))
    {
        if (IsEnigmaBerryValid() == TRUE)
        {
            for (i = 0; i < BERRY_NAME_LENGTH; i++)
            {
                gEnigmaBerries[0].name[i] = gSaveBlock1Ptr->enigmaBerry.berry.name[i];
                gEnigmaBerries[2].name[i] = gSaveBlock1Ptr->enigmaBerry.berry.name[i];
            }
            gEnigmaBerries[0].name[i] = EOS;
            gEnigmaBerries[2].name[i] = EOS;

            for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; i++)
            {
                gEnigmaBerries[0].itemEffect[i] = gSaveBlock1Ptr->enigmaBerry.itemEffect[i];
                gEnigmaBerries[2].itemEffect[i] = gSaveBlock1Ptr->enigmaBerry.itemEffect[i];
            }

            gEnigmaBerries[0].holdEffect = gSaveBlock1Ptr->enigmaBerry.holdEffect;
            gEnigmaBerries[2].holdEffect = gSaveBlock1Ptr->enigmaBerry.holdEffect;
            gEnigmaBerries[0].holdEffectParam = gSaveBlock1Ptr->enigmaBerry.holdEffectParam;
            gEnigmaBerries[2].holdEffectParam = gSaveBlock1Ptr->enigmaBerry.holdEffectParam;
        }
        else
        {
            const struct Berry *berryData = GetBerryInfo(ItemIdToBerryType(ITEM_ENIGMA_BERRY));

            for (i = 0; i < BERRY_NAME_LENGTH; i++)
            {
                gEnigmaBerries[0].name[i] = berryData->name[i];
                gEnigmaBerries[2].name[i] = berryData->name[i];
            }
            gEnigmaBerries[0].name[i] = EOS;
            gEnigmaBerries[2].name[i] = EOS;

            for (i = 0; i < BERRY_ITEM_EFFECT_COUNT; i++)
            {
                gEnigmaBerries[0].itemEffect[i] = 0;
                gEnigmaBerries[2].itemEffect[i] = 0;
            }

            gEnigmaBerries[0].holdEffect = 0;
            gEnigmaBerries[2].holdEffect = 0;
            gEnigmaBerries[0].holdEffectParam = 0;
            gEnigmaBerries[2].holdEffectParam = 0;
        }
    }
    else
    {
        s32 numPlayers;
        struct BattleEnigmaBerry *src;
        u8 battlerId;

        if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_BATTLE_TOWER)
                numPlayers = 2;
            else
                numPlayers = 4;

            for (i = 0; i < numPlayers; i++)
            {
                src = (struct BattleEnigmaBerry *)(gBlockRecvBuffer[i] + 2);
                battlerId = gLinkPlayers[i].id;

                for (j = 0; j < BERRY_NAME_LENGTH; j++)
                    gEnigmaBerries[battlerId].name[j] = src->name[j];
                gEnigmaBerries[battlerId].name[j] = EOS;

                for (j = 0; j < BERRY_ITEM_EFFECT_COUNT; j++)
                    gEnigmaBerries[battlerId].itemEffect[j] = src->itemEffect[j];

                gEnigmaBerries[battlerId].holdEffect = src->holdEffect;
                gEnigmaBerries[battlerId].holdEffectParam = src->holdEffectParam;
            }
        }
        else
        {
            for (i = 0; i < 2; i++)
            {
                src = (struct BattleEnigmaBerry *)(gBlockRecvBuffer[i] + 2);

                for (j = 0; j < BERRY_NAME_LENGTH; j++)
                {
                    gEnigmaBerries[i].name[j] = src->name[j];
                    gEnigmaBerries[i + 2].name[j] = src->name[j];
                }
                gEnigmaBerries[i].name[j] = EOS;
                gEnigmaBerries[i + 2].name[j] = EOS;

                for (j = 0; j < BERRY_ITEM_EFFECT_COUNT; j++)
                {
                    gEnigmaBerries[i].itemEffect[j] = src->itemEffect[j];
                    gEnigmaBerries[i + 2].itemEffect[j] = src->itemEffect[j];
                }

                gEnigmaBerries[i].holdEffect = src->holdEffect;
                gEnigmaBerries[i + 2].holdEffect = src->holdEffect;
                gEnigmaBerries[i].holdEffectParam = src->holdEffectParam;
                gEnigmaBerries[i + 2].holdEffectParam = src->holdEffectParam;
            }
        }
    }
}

// This was inlined in Ruby/Sapphire
static void FindLinkBattleMaster(u8 numPlayers, u8 multiPlayerId)
{
    u8 found = 0;

    // If player 1 is playing the minimum version, player 1 is master.
    if (gBlockRecvBuffer[0][0] == 0x100)
    {
        if (multiPlayerId == 0)
            gBattleTypeFlags |= BATTLE_TYPE_IS_MASTER | BATTLE_TYPE_TRAINER;
        else
            gBattleTypeFlags |= BATTLE_TYPE_TRAINER;
        found++;
    }

    if (found == 0)
    {
        // If multiple different versions are being used, player 1 is master.
        s32 i;

        for (i = 0; i < numPlayers; i++)
        {
            if (gBlockRecvBuffer[0][0] != gBlockRecvBuffer[i][0])
                break;
        }

        if (i == numPlayers)
        {
            if (multiPlayerId == 0)
                gBattleTypeFlags |= BATTLE_TYPE_IS_MASTER | BATTLE_TYPE_TRAINER;
            else
                gBattleTypeFlags |= BATTLE_TYPE_TRAINER;
            found++;
        }

        if (found == 0)
        {
            // Lowest index player with the highest game version is master.
            for (i = 0; i < numPlayers; i++)
            {
                if (gBlockRecvBuffer[i][0] == 0x300 && i != multiPlayerId)
                {
                    if (i < multiPlayerId)
                        break;
                }
                if (gBlockRecvBuffer[i][0] > 0x300 && i != multiPlayerId)
                    break;
            }

            if (i == numPlayers)
                gBattleTypeFlags |= BATTLE_TYPE_IS_MASTER | BATTLE_TYPE_TRAINER;
            else
                gBattleTypeFlags |= BATTLE_TYPE_TRAINER;
        }
    }
}

static void CB2_HandleStartBattle(void)
{
    u8 playerMultiplayerId;
    u8 enemyMultiplayerId;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    playerMultiplayerId = GetMultiplayerId();
    gBattleScripting.multiplayerId = playerMultiplayerId;
    enemyMultiplayerId = playerMultiplayerId ^ BIT_SIDE;

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            ShowBg(0);
            ShowBg(1);
            ShowBg(2);
            ShowBg(3);
            sub_805EF14();
            gBattleCommunication[MULTIUSE_STATE] = 1;
        }
        if (gWirelessCommType)
            LoadWirelessStatusIndicatorSpriteGfx();
        break;
    case 1:
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            if (gReceivedRemoteLinkPlayers != 0)
            {
                if (IsLinkTaskFinished())
                {
                    // 0x300
                    *(&gBattleStruct->multiBuffer.linkBattlerHeader.versionSignatureLo) = 0;
                    *(&gBattleStruct->multiBuffer.linkBattlerHeader.versionSignatureHi) = 3;
                    BufferPartyVsScreenHealth_AtStart();
                    SetPlayerBerryDataInBattleStruct();

                    if (gTrainerBattleOpponent_A == TRAINER_UNION_ROOM)
                    {
                        gLinkPlayers[0].id = 0;
                        gLinkPlayers[1].id = 1;
                    }

                    SendBlock(bitmask_all_link_players_but_self(), &gBattleStruct->multiBuffer.linkBattlerHeader, sizeof(gBattleStruct->multiBuffer.linkBattlerHeader));
                    gBattleCommunication[MULTIUSE_STATE] = 2;
                }
                if (gWirelessCommType)
                    CreateWirelessStatusIndicatorSprite(0, 0);
            }
        }
        else
        {
            if (!(gBattleTypeFlags & BATTLE_TYPE_RECORDED))
                gBattleTypeFlags |= BATTLE_TYPE_IS_MASTER;
            gBattleCommunication[MULTIUSE_STATE] = 15;
            SetAllPlayersBerryData();
        }
        break;
    case 2:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            u8 taskId;

            ResetBlockReceivedFlags();
            FindLinkBattleMaster(2, playerMultiplayerId);
            SetAllPlayersBerryData();
            taskId = CreateTask(InitLinkBattleVsScreen, 0);
            gTasks[taskId].data[1] = 0x10E;
            gTasks[taskId].data[2] = 0x5A;
            gTasks[taskId].data[5] = 0;
            gTasks[taskId].data[3] = gBattleStruct->multiBuffer.linkBattlerHeader.vsScreenHealthFlagsLo | (gBattleStruct->multiBuffer.linkBattlerHeader.vsScreenHealthFlagsHi << 8);
            gTasks[taskId].data[4] = gBlockRecvBuffer[enemyMultiplayerId][1];
            RecordedBattle_SetFrontierPassFlagFromHword(gBlockRecvBuffer[playerMultiplayerId][1]);
            RecordedBattle_SetFrontierPassFlagFromHword(gBlockRecvBuffer[enemyMultiplayerId][1]);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 3:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 4:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            memcpy(gEnemyParty, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 7:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty + 2, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 8:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            memcpy(gEnemyParty + 2, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 11:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty + 4, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 12:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            memcpy(gEnemyParty + 4, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon) * 2);
            TryCorrectShedinjaLanguage(&gEnemyParty[0]);
            TryCorrectShedinjaLanguage(&gEnemyParty[1]);
            TryCorrectShedinjaLanguage(&gEnemyParty[2]);
            TryCorrectShedinjaLanguage(&gEnemyParty[3]);
            TryCorrectShedinjaLanguage(&gEnemyParty[4]);
            TryCorrectShedinjaLanguage(&gEnemyParty[5]);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 15:
        InitBattleControllers();
        sub_8184E58();
        gBattleCommunication[SPRITES_INIT_STATE1] = 0;
        gBattleCommunication[SPRITES_INIT_STATE2] = 0;
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            s32 i;

            for (i = 0; i < 2 && (gLinkPlayers[i].version & 0xFF) == VERSION_EMERALD; i++);

            if (i == 2)
                gBattleCommunication[MULTIUSE_STATE] = 16;
            else
                gBattleCommunication[MULTIUSE_STATE] = 18;
        }
        else
        {
            gBattleCommunication[MULTIUSE_STATE] = 18;
        }
        break;
    case 16:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), &gRecordedBattleRngSeed, sizeof(gRecordedBattleRngSeed));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 17:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (!(gBattleTypeFlags & BATTLE_TYPE_IS_MASTER))
                memcpy(&gRecordedBattleRngSeed, gBlockRecvBuffer[enemyMultiplayerId], sizeof(gRecordedBattleRngSeed));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 18:
        if (BattleInitAllSprites(&gBattleCommunication[SPRITES_INIT_STATE1], &gBattleCommunication[SPRITES_INIT_STATE2]))
        {
            gPreBattleCallback1 = gMain.callback1;
            gMain.callback1 = BattleMainCB1;
            SetMainCallback2(BattleMainCB2);
            if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                gBattleTypeFlags |= BATTLE_TYPE_LINK_IN_BATTLE;
            }
        }
        break;
    case 5:
    case 9:
    case 13:
        gBattleCommunication[MULTIUSE_STATE]++;
        gBattleCommunication[1] = 1;
    case 6:
    case 10:
    case 14:
        if (--gBattleCommunication[1] == 0)
            gBattleCommunication[MULTIUSE_STATE]++;
        break;
    }
}

static void CB2_HandleStartMultiPartnerBattle(void)
{
    u8 playerMultiplayerId;
    u8 enemyMultiplayerId;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    playerMultiplayerId = GetMultiplayerId();
    gBattleScripting.multiplayerId = playerMultiplayerId;
    enemyMultiplayerId = playerMultiplayerId ^ BIT_SIDE;

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            ShowBg(0);
            ShowBg(1);
            ShowBg(2);
            ShowBg(3);
            sub_805EF14();
            gBattleCommunication[MULTIUSE_STATE] = 1;
        }
        if (gWirelessCommType)
            LoadWirelessStatusIndicatorSpriteGfx();
        // fall through
    case 1:
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            if (gReceivedRemoteLinkPlayers != 0)
            {
                u8 language;

                gLinkPlayers[0].id = 0;
                gLinkPlayers[1].id = 2;
                gLinkPlayers[2].id = 1;
                gLinkPlayers[3].id = 3;
                GetFrontierTrainerName(gLinkPlayers[2].name, gTrainerBattleOpponent_A);
                GetFrontierTrainerName(gLinkPlayers[3].name, gTrainerBattleOpponent_B);
                GetBattleTowerTrainerLanguage(&language, gTrainerBattleOpponent_A);
                gLinkPlayers[2].language = language;
                GetBattleTowerTrainerLanguage(&language, gTrainerBattleOpponent_B);
                gLinkPlayers[3].language = language;

                if (IsLinkTaskFinished())
                {
                    // 0x300
                    *(&gBattleStruct->multiBuffer.linkBattlerHeader.versionSignatureLo) = 0;
                    *(&gBattleStruct->multiBuffer.linkBattlerHeader.versionSignatureHi) = 3;
                    BufferPartyVsScreenHealth_AtStart();
                    SetPlayerBerryDataInBattleStruct();
                    SendBlock(bitmask_all_link_players_but_self(), &gBattleStruct->multiBuffer.linkBattlerHeader, sizeof(gBattleStruct->multiBuffer.linkBattlerHeader));
                    gBattleCommunication[MULTIUSE_STATE] = 2;
                }

                if (gWirelessCommType)
                    CreateWirelessStatusIndicatorSprite(0, 0);
            }
        }
        else
        {
            if (!(gBattleTypeFlags & BATTLE_TYPE_RECORDED))
                gBattleTypeFlags |= BATTLE_TYPE_IS_MASTER;
            gBattleCommunication[MULTIUSE_STATE] = 13;
            SetAllPlayersBerryData();
        }
        break;
    case 2:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            u8 taskId;

            ResetBlockReceivedFlags();
            FindLinkBattleMaster(2, playerMultiplayerId);
            SetAllPlayersBerryData();
            taskId = CreateTask(InitLinkBattleVsScreen, 0);
            gTasks[taskId].data[1] = 0x10E;
            gTasks[taskId].data[2] = 0x5A;
            gTasks[taskId].data[5] = 0;
            gTasks[taskId].data[3] = 0x145;
            gTasks[taskId].data[4] = 0x145;
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 3:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 4:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (gLinkPlayers[playerMultiplayerId].id != 0)
            {
                memcpy(gPlayerParty, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon) * 2);
                memcpy(gPlayerParty + MULTI_PARTY_SIZE, gBlockRecvBuffer[playerMultiplayerId], sizeof(struct Pokemon) * 2);
            }
            else
            {
                memcpy(gPlayerParty, gBlockRecvBuffer[playerMultiplayerId], sizeof(struct Pokemon) * 2);
                memcpy(gPlayerParty + MULTI_PARTY_SIZE, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon) * 2);
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 5:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty + 2, sizeof(struct Pokemon));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 6:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (gLinkPlayers[playerMultiplayerId].id != 0)
            {
                memcpy(gPlayerParty + 2, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon));
                memcpy(gPlayerParty + 5, gBlockRecvBuffer[playerMultiplayerId], sizeof(struct Pokemon));
            }
            else
            {
                memcpy(gPlayerParty + 2, gBlockRecvBuffer[playerMultiplayerId], sizeof(struct Pokemon));
                memcpy(gPlayerParty + 5, gBlockRecvBuffer[enemyMultiplayerId], sizeof(struct Pokemon));
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 7:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gEnemyParty, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 8:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (GetMultiplayerId() != 0)
            {
                memcpy(gEnemyParty, gBlockRecvBuffer[0], sizeof(struct Pokemon) * 2);
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 9:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gEnemyParty + 2, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 10:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (GetMultiplayerId() != 0)
            {
                memcpy(gEnemyParty + 2, gBlockRecvBuffer[0], sizeof(struct Pokemon) * 2);
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 11:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gEnemyParty + 4, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 12:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (GetMultiplayerId() != 0)
                memcpy(gEnemyParty + 4, gBlockRecvBuffer[0], sizeof(struct Pokemon) * 2);
            TryCorrectShedinjaLanguage(&gPlayerParty[0]);
            TryCorrectShedinjaLanguage(&gPlayerParty[1]);
            TryCorrectShedinjaLanguage(&gPlayerParty[2]);
            TryCorrectShedinjaLanguage(&gPlayerParty[3]);
            TryCorrectShedinjaLanguage(&gPlayerParty[4]);
            TryCorrectShedinjaLanguage(&gPlayerParty[5]);
            TryCorrectShedinjaLanguage(&gEnemyParty[0]);
            TryCorrectShedinjaLanguage(&gEnemyParty[1]);
            TryCorrectShedinjaLanguage(&gEnemyParty[2]);
            TryCorrectShedinjaLanguage(&gEnemyParty[3]);
            TryCorrectShedinjaLanguage(&gEnemyParty[4]);
            TryCorrectShedinjaLanguage(&gEnemyParty[5]);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 13:
        InitBattleControllers();
        sub_8184E58();
        gBattleCommunication[SPRITES_INIT_STATE1] = 0;
        gBattleCommunication[SPRITES_INIT_STATE2] = 0;
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            gBattleCommunication[MULTIUSE_STATE] = 14;
        }
        else
        {
            gBattleCommunication[MULTIUSE_STATE] = 16;
        }
        break;
    case 14:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), &gRecordedBattleRngSeed, sizeof(gRecordedBattleRngSeed));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 15:
        if ((GetBlockReceivedStatus() & 3) == 3)
        {
            ResetBlockReceivedFlags();
            if (!(gBattleTypeFlags & BATTLE_TYPE_IS_MASTER))
                memcpy(&gRecordedBattleRngSeed, gBlockRecvBuffer[enemyMultiplayerId], sizeof(gRecordedBattleRngSeed));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 16:
        if (BattleInitAllSprites(&gBattleCommunication[SPRITES_INIT_STATE1], &gBattleCommunication[SPRITES_INIT_STATE2]))
        {
            TrySetLinkBattleTowerEnemyPartyLevel();
            gPreBattleCallback1 = gMain.callback1;
            gMain.callback1 = BattleMainCB1;
            SetMainCallback2(BattleMainCB2);
            if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                gBattleTypeFlags |= BATTLE_TYPE_LINK_IN_BATTLE;
            }
        }
        break;
    }
}

static void sub_80379F8(u8 arrayIdPlus)
{
    s32 i;

    for (i = 0; i < (int)ARRAY_COUNT(gMultiPartnerParty); i++)
    {
        gMultiPartnerParty[i].species     = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_SPECIES);
        gMultiPartnerParty[i].heldItem    = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_HELD_ITEM);
        GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_NICKNAME, gMultiPartnerParty[i].nickname);
        gMultiPartnerParty[i].level       = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_LEVEL);
        gMultiPartnerParty[i].hp          = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_HP);
        gMultiPartnerParty[i].maxhp       = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_MAX_HP);
        gMultiPartnerParty[i].status      = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_STATUS);
        gMultiPartnerParty[i].personality = GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_PERSONALITY);
        gMultiPartnerParty[i].gender      = GetMonGender(&gPlayerParty[arrayIdPlus + i]);
        StripExtCtrlCodes(gMultiPartnerParty[i].nickname);
        if (GetMonData(&gPlayerParty[arrayIdPlus + i], MON_DATA_LANGUAGE) != LANGUAGE_JAPANESE)
            PadNameString(gMultiPartnerParty[i].nickname, CHAR_SPACE);
    }
    memcpy(sMultiPartnerPartyBuffer, gMultiPartnerParty, sizeof(gMultiPartnerParty));
}

static void CB2_PreInitMultiBattle(void)
{
    s32 i;
    u8 playerMultiplierId;
    s32 numPlayers = 4;
    u8 r4 = 0xF;
    u32 *savedBattleTypeFlags;
    void (**savedCallback)(void);

    if (gBattleTypeFlags & BATTLE_TYPE_BATTLE_TOWER)
    {
        numPlayers = 2;
        r4 = 3;
    }

    playerMultiplierId = GetMultiplayerId();
    gBattleScripting.multiplayerId = playerMultiplierId;
    savedCallback = &gBattleStruct->savedCallback;
    savedBattleTypeFlags = &gBattleStruct->savedBattleTypeFlags;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        if (gReceivedRemoteLinkPlayers != 0 && IsLinkTaskFinished())
        {
            sMultiPartnerPartyBuffer = Alloc(sizeof(struct UnknownPokemonStruct4) * ARRAY_COUNT(gMultiPartnerParty));
            sub_80379F8(0);
            SendBlock(bitmask_all_link_players_but_self(), sMultiPartnerPartyBuffer, sizeof(struct UnknownPokemonStruct4) * ARRAY_COUNT(gMultiPartnerParty));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 1:
        if ((GetBlockReceivedStatus() & r4) == r4)
        {
            ResetBlockReceivedFlags();
            for (i = 0; i < numPlayers; i++)
            {
                if (i == playerMultiplierId)
                    continue;

                if (numPlayers == MAX_LINK_PLAYERS)
                {
                    if ((!(gLinkPlayers[i].id & 1) && !(gLinkPlayers[playerMultiplierId].id & 1))
                        || (gLinkPlayers[i].id & 1 && gLinkPlayers[playerMultiplierId].id & 1))
                    {
                        memcpy(gMultiPartnerParty, gBlockRecvBuffer[i], sizeof(struct UnknownPokemonStruct4) * ARRAY_COUNT(gMultiPartnerParty));
                    }
                }
                else
                {
                    memcpy(gMultiPartnerParty, gBlockRecvBuffer[i], sizeof(struct UnknownPokemonStruct4) * ARRAY_COUNT(gMultiPartnerParty));
                }
            }
            gBattleCommunication[MULTIUSE_STATE]++;
            *savedCallback = gMain.savedCallback;
            *savedBattleTypeFlags = gBattleTypeFlags;
            gMain.savedCallback = CB2_PreInitMultiBattle;
            ShowPartyMenuToShowcaseMultiBattleParty();
        }
        break;
    case 2:
        if (IsLinkTaskFinished() && !gPaletteFade.active)
        {
            gBattleCommunication[MULTIUSE_STATE]++;
            if (gWirelessCommType)
                SetLinkStandbyCallback();
            else
                SetCloseLinkCallback();
        }
        break;
    case 3:
        if (gWirelessCommType)
        {
            if (IsLinkRfuTaskFinished())
            {
                gBattleTypeFlags = *savedBattleTypeFlags;
                gMain.savedCallback = *savedCallback;
                SetMainCallback2(CB2_InitBattleInternal);
                Free(sMultiPartnerPartyBuffer);
                sMultiPartnerPartyBuffer = NULL;
            }
        }
        else if (gReceivedRemoteLinkPlayers == 0)
        {
            gBattleTypeFlags = *savedBattleTypeFlags;
            gMain.savedCallback = *savedCallback;
            SetMainCallback2(CB2_InitBattleInternal);
            Free(sMultiPartnerPartyBuffer);
            sMultiPartnerPartyBuffer = NULL;
        }
        break;
    }
}

static void CB2_PreInitIngamePlayerPartnerBattle(void)
{
    u32 *savedBattleTypeFlags;
    void (**savedCallback)(void);

    savedCallback = &gBattleStruct->savedCallback;
    savedBattleTypeFlags = &gBattleStruct->savedBattleTypeFlags;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        sMultiPartnerPartyBuffer = Alloc(sizeof(struct UnknownPokemonStruct4) * ARRAY_COUNT(gMultiPartnerParty));
        sub_80379F8(3);
        gBattleCommunication[MULTIUSE_STATE]++;
        *savedCallback = gMain.savedCallback;
        *savedBattleTypeFlags = gBattleTypeFlags;
        gMain.savedCallback = CB2_PreInitIngamePlayerPartnerBattle;
        ShowPartyMenuToShowcaseMultiBattleParty();
        break;
    case 1:
        if (!gPaletteFade.active)
        {
            gBattleCommunication[MULTIUSE_STATE] = 2;
            gBattleTypeFlags = *savedBattleTypeFlags;
            gMain.savedCallback = *savedCallback;
            SetMainCallback2(CB2_InitBattleInternal);
            Free(sMultiPartnerPartyBuffer);
            sMultiPartnerPartyBuffer = NULL;
        }
        break;
    }
}

static void CB2_HandleStartMultiBattle(void)
{
    u8 playerMultiplayerId;
    s32 id;
    u8 var;

    playerMultiplayerId = GetMultiplayerId();
    gBattleScripting.multiplayerId = playerMultiplayerId;

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        if (!IsDma3ManagerBusyWithBgCopy())
        {
            ShowBg(0);
            ShowBg(1);
            ShowBg(2);
            ShowBg(3);
            sub_805EF14();
            gBattleCommunication[MULTIUSE_STATE] = 1;
        }
        if (gWirelessCommType)
            LoadWirelessStatusIndicatorSpriteGfx();
        break;
    case 1:
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            if (gReceivedRemoteLinkPlayers != 0)
            {
                if (IsLinkTaskFinished())
                {
                    // 0x300
                    *(&gBattleStruct->multiBuffer.linkBattlerHeader.versionSignatureLo) = 0;
                    *(&gBattleStruct->multiBuffer.linkBattlerHeader.versionSignatureHi) = 3;
                    BufferPartyVsScreenHealth_AtStart();
                    SetPlayerBerryDataInBattleStruct();

                    SendBlock(bitmask_all_link_players_but_self(), &gBattleStruct->multiBuffer.linkBattlerHeader, sizeof(gBattleStruct->multiBuffer.linkBattlerHeader));
                    gBattleCommunication[MULTIUSE_STATE]++;
                }
                if (gWirelessCommType)
                    CreateWirelessStatusIndicatorSprite(0, 0);
            }
        }
        else
        {
            if (!(gBattleTypeFlags & BATTLE_TYPE_RECORDED))
                gBattleTypeFlags |= BATTLE_TYPE_IS_MASTER;
            gBattleCommunication[MULTIUSE_STATE] = 7;
            SetAllPlayersBerryData();
        }
        break;
    case 2:
        if ((GetBlockReceivedStatus() & 0xF) == 0xF)
        {
            ResetBlockReceivedFlags();
            FindLinkBattleMaster(4, playerMultiplayerId);
            SetAllPlayersBerryData();
            var = CreateTask(InitLinkBattleVsScreen, 0);
            gTasks[var].data[1] = 0x10E;
            gTasks[var].data[2] = 0x5A;
            gTasks[var].data[5] = 0;
            gTasks[var].data[3] = 0;
            gTasks[var].data[4] = 0;

            for (id = 0; id < MAX_LINK_PLAYERS; id++)
            {
                RecordedBattle_SetFrontierPassFlagFromHword(gBlockRecvBuffer[id][1]);
                switch (gLinkPlayers[id].id)
                {
                case 0:
                    gTasks[var].data[3] |= gBlockRecvBuffer[id][1] & 0x3F;
                    break;
                case 1:
                    gTasks[var].data[4] |= gBlockRecvBuffer[id][1] & 0x3F;
                    break;
                case 2:
                    gTasks[var].data[3] |= (gBlockRecvBuffer[id][1] & 0x3F) << 6;
                    break;
                case 3:
                    gTasks[var].data[4] |= (gBlockRecvBuffer[id][1] & 0x3F) << 6;
                    break;
                }
            }
            ZeroEnemyPartyMons();
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        else
            break;
        // fall through
    case 3:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty, sizeof(struct Pokemon) * 2);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 4:
        if ((GetBlockReceivedStatus() & 0xF) == 0xF)
        {
            ResetBlockReceivedFlags();
            for (id = 0; id < MAX_LINK_PLAYERS; id++)
            {
                if (id == playerMultiplayerId)
                {
                    switch (gLinkPlayers[id].id)
                    {
                    case 0:
                    case 3:
                        memcpy(gPlayerParty, gBlockRecvBuffer[id], sizeof(struct Pokemon) * 2);
                        break;
                    case 1:
                    case 2:
                        memcpy(gPlayerParty + MULTI_PARTY_SIZE, gBlockRecvBuffer[id], sizeof(struct Pokemon) * 2);
                        break;
                    }
                }
                else
                {
                    if ((!(gLinkPlayers[id].id & 1) && !(gLinkPlayers[playerMultiplayerId].id & 1))
                     || ((gLinkPlayers[id].id & 1) && (gLinkPlayers[playerMultiplayerId].id & 1)))
                    {
                        switch (gLinkPlayers[id].id)
                        {
                        case 0:
                        case 3:
                            memcpy(gPlayerParty, gBlockRecvBuffer[id], sizeof(struct Pokemon) * 2);
                            break;
                        case 1:
                        case 2:
                            memcpy(gPlayerParty + MULTI_PARTY_SIZE, gBlockRecvBuffer[id], sizeof(struct Pokemon) * 2);
                            break;
                        }
                    }
                    else
                    {
                        switch (gLinkPlayers[id].id)
                        {
                        case 0:
                        case 3:
                            memcpy(gEnemyParty, gBlockRecvBuffer[id], sizeof(struct Pokemon) * 2);
                            break;
                        case 1:
                        case 2:
                            memcpy(gEnemyParty + MULTI_PARTY_SIZE, gBlockRecvBuffer[id], sizeof(struct Pokemon) * 2);
                            break;
                        }
                    }
                }
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 5:
        if (IsLinkTaskFinished())
        {
            SendBlock(bitmask_all_link_players_but_self(), gPlayerParty + 2, sizeof(struct Pokemon));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 6:
        if ((GetBlockReceivedStatus() & 0xF) == 0xF)
        {
            ResetBlockReceivedFlags();
            for (id = 0; id < MAX_LINK_PLAYERS; id++)
            {
                if (id == playerMultiplayerId)
                {
                    switch (gLinkPlayers[id].id)
                    {
                    case 0:
                    case 3:
                        memcpy(gPlayerParty + 2, gBlockRecvBuffer[id], sizeof(struct Pokemon));
                        break;
                    case 1:
                    case 2:
                        memcpy(gPlayerParty + 5, gBlockRecvBuffer[id], sizeof(struct Pokemon));
                        break;
                    }
                }
                else
                {
                    if ((!(gLinkPlayers[id].id & 1) && !(gLinkPlayers[playerMultiplayerId].id & 1))
                     || ((gLinkPlayers[id].id & 1) && (gLinkPlayers[playerMultiplayerId].id & 1)))
                    {
                        switch (gLinkPlayers[id].id)
                        {
                        case 0:
                        case 3:
                            memcpy(gPlayerParty + 2, gBlockRecvBuffer[id], sizeof(struct Pokemon));
                            break;
                        case 1:
                        case 2:
                            memcpy(gPlayerParty + 5, gBlockRecvBuffer[id], sizeof(struct Pokemon));
                            break;
                        }
                    }
                    else
                    {
                        switch (gLinkPlayers[id].id)
                        {
                        case 0:
                        case 3:
                            memcpy(gEnemyParty + 2, gBlockRecvBuffer[id], sizeof(struct Pokemon));
                            break;
                        case 1:
                        case 2:
                            memcpy(gEnemyParty + 5, gBlockRecvBuffer[id], sizeof(struct Pokemon));
                            break;
                        }
                    }
                }
            }
            TryCorrectShedinjaLanguage(&gPlayerParty[0]);
            TryCorrectShedinjaLanguage(&gPlayerParty[1]);
            TryCorrectShedinjaLanguage(&gPlayerParty[2]);
            TryCorrectShedinjaLanguage(&gPlayerParty[3]);
            TryCorrectShedinjaLanguage(&gPlayerParty[4]);
            TryCorrectShedinjaLanguage(&gPlayerParty[5]);

            TryCorrectShedinjaLanguage(&gEnemyParty[0]);
            TryCorrectShedinjaLanguage(&gEnemyParty[1]);
            TryCorrectShedinjaLanguage(&gEnemyParty[2]);
            TryCorrectShedinjaLanguage(&gEnemyParty[3]);
            TryCorrectShedinjaLanguage(&gEnemyParty[4]);
            TryCorrectShedinjaLanguage(&gEnemyParty[5]);

            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 7:
        InitBattleControllers();
        sub_8184E58();
        gBattleCommunication[SPRITES_INIT_STATE1] = 0;
        gBattleCommunication[SPRITES_INIT_STATE2] = 0;
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            for (id = 0; id < MAX_LINK_PLAYERS && (gLinkPlayers[id].version & 0xFF) == VERSION_EMERALD; id++);

            if (id == MAX_LINK_PLAYERS)
                gBattleCommunication[MULTIUSE_STATE] = 8;
            else
                gBattleCommunication[MULTIUSE_STATE] = 10;
        }
        else
        {
            gBattleCommunication[MULTIUSE_STATE] = 10;
        }
        break;
    case 8:
        if (IsLinkTaskFinished())
        {
            u32* ptr = gBattleStruct->multiBuffer.battleVideo;
            ptr[0] = gBattleTypeFlags;
            ptr[1] = gRecordedBattleRngSeed; // UB: overwrites berry data
            SendBlock(bitmask_all_link_players_but_self(), ptr, sizeof(gBattleStruct->multiBuffer.battleVideo));
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 9:
        if ((GetBlockReceivedStatus() & 0xF) == 0xF)
        {
            ResetBlockReceivedFlags();
            for (var = 0; var < 4; var++)
            {
                u32 blockValue = gBlockRecvBuffer[var][0];
                if (blockValue & 4)
                {
                    memcpy(&gRecordedBattleRngSeed, &gBlockRecvBuffer[var][2], sizeof(gRecordedBattleRngSeed));
                    break;
                }
            }

            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 10:
        if (BattleInitAllSprites(&gBattleCommunication[SPRITES_INIT_STATE1], &gBattleCommunication[SPRITES_INIT_STATE2]))
        {
            gPreBattleCallback1 = gMain.callback1;
            gMain.callback1 = BattleMainCB1;
            SetMainCallback2(BattleMainCB2);
            if (gBattleTypeFlags & BATTLE_TYPE_LINK)
            {
                gTrainerBattleOpponent_A = TRAINER_LINK_OPPONENT;
                gBattleTypeFlags |= BATTLE_TYPE_LINK_IN_BATTLE;
            }
        }
        break;
    }
}

void BattleMainCB2(void)
{
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    RunTasks();

    if (JOY_HELD(B_BUTTON) && gBattleTypeFlags & BATTLE_TYPE_RECORDED && sub_8186450())
    {
        gSpecialVar_Result = gBattleOutcome = B_OUTCOME_PLAYER_TELEPORTED;
        ResetPaletteFadeControl();
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        SetMainCallback2(CB2_QuitRecordedBattle);
    }
}

static void FreeRestoreBattleData(void)
{
    gMain.callback1 = gPreBattleCallback1;
    gScanlineEffect.state = 3;
    gMain.inBattle = 0;
    ZeroEnemyPartyMons();
    m4aSongNumStop(SE_LOW_HEALTH);
    FreeMonSpritesGfx();
    FreeBattleSpritesData();
    FreeBattleResources();
}

void CB2_QuitRecordedBattle(void)
{
    UpdatePaletteFade();
    if (!gPaletteFade.active)
    {
        m4aMPlayStop(&gMPlayInfo_SE1);
        m4aMPlayStop(&gMPlayInfo_SE2);
        FreeRestoreBattleData();
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

void sub_8038528(struct Sprite* sprite)
{
    sprite->data[0] = 0;
    sprite->callback = sub_8038538;
}

static void sub_8038538(struct Sprite *sprite)
{
    u16 *arr = (u16*)(gDecompressionBuffer);

    switch (sprite->data[0])
    {
    case 0:
        sprite->data[0]++;
        sprite->data[1] = 0;
        sprite->data[2] = 0x281;
        sprite->data[3] = 0;
        sprite->data[4] = 1;
        // fall through
    case 1:
        sprite->data[4]--;
        if (sprite->data[4] == 0)
        {
            s32 i;
            s32 r2;
            s32 r0;

            sprite->data[4] = 2;
            r2 = sprite->data[1] + sprite->data[3] * 32;
            r0 = sprite->data[2] - sprite->data[3] * 32;
            for (i = 0; i < 29; i += 2)
            {
                arr[r2 + i] = 0x3D;
                arr[r0 + i] = 0x3D;
            }
            sprite->data[3]++;
            if (sprite->data[3] == 21)
            {
                sprite->data[0]++;
                sprite->data[1] = 32;
            }
        }
        break;
    case 2:
        sprite->data[1]--;
        if (sprite->data[1] == 20)
            SetMainCallback2(CB2_InitBattle);
        break;
    }
}

// Random mon list, exclude gen 1-3
#define EVO_TYPE_0 0
#define EVO_TYPE_1 1
#define EVO_TYPE_2 2
#define EVO_TYPE_SELF 3
#define EVO_TYPE_LEGENDARY 4

const u8 gEvoStages[5][20] = 
{
    [EVO_TYPE_0]            = _("EVO TYPE 0"),
    [EVO_TYPE_1]            = _("EVO TYPE 1"),
    [EVO_TYPE_2]            = _("EVO TYPE 2"),
    [EVO_TYPE_SELF]         = _("EVO TYPE SELF"),
    [EVO_TYPE_LEGENDARY]    = _("EVO TYPE LEGENDARY"),
};

static const u8 gSpeciesMapping[NUM_SPECIES+1] =
{
    [SPECIES_NONE]              = EVO_TYPE_SELF,
    [SPECIES_BULBASAUR]         = EVO_TYPE_0,
    [SPECIES_IVYSAUR]           = EVO_TYPE_1,
    [SPECIES_VENUSAUR]          = EVO_TYPE_2,
    [SPECIES_CHARMANDER]        = EVO_TYPE_0,
    [SPECIES_CHARMELEON]        = EVO_TYPE_1,
    [SPECIES_CHARIZARD]         = EVO_TYPE_2,
    [SPECIES_SQUIRTLE]          = EVO_TYPE_0,
    [SPECIES_WARTORTLE]         = EVO_TYPE_1,
    [SPECIES_BLASTOISE]         = EVO_TYPE_2,
    [SPECIES_CATERPIE]          = EVO_TYPE_0,
    [SPECIES_METAPOD]           = EVO_TYPE_1,
    [SPECIES_BUTTERFREE]        = EVO_TYPE_2,
    [SPECIES_WEEDLE]            = EVO_TYPE_0,
    [SPECIES_KAKUNA]            = EVO_TYPE_1,
    [SPECIES_BEEDRILL]          = EVO_TYPE_2,
    [SPECIES_PIDGEY]            = EVO_TYPE_0,
    [SPECIES_PIDGEOTTO]         = EVO_TYPE_1,
    [SPECIES_PIDGEOT]           = EVO_TYPE_2,
    [SPECIES_RATTATA]           = EVO_TYPE_0,
    [SPECIES_RATICATE]          = EVO_TYPE_1,
    [SPECIES_SPEAROW]           = EVO_TYPE_0,
    [SPECIES_FEAROW]            = EVO_TYPE_1,
    [SPECIES_EKANS]             = EVO_TYPE_0,
    [SPECIES_ARBOK]             = EVO_TYPE_1,
    [SPECIES_PIKACHU]           = EVO_TYPE_1,
    [SPECIES_RAICHU]            = EVO_TYPE_2,
    [SPECIES_SANDSHREW]         = EVO_TYPE_0,
    [SPECIES_SANDSLASH]         = EVO_TYPE_1,
    [SPECIES_NIDORAN_F]         = EVO_TYPE_0,
    [SPECIES_NIDORINA]          = EVO_TYPE_1,
    [SPECIES_NIDOQUEEN]         = EVO_TYPE_2,
    [SPECIES_NIDORAN_M]         = EVO_TYPE_0,
    [SPECIES_NIDORINO]          = EVO_TYPE_1,
    [SPECIES_NIDOKING]          = EVO_TYPE_2,
    [SPECIES_CLEFAIRY]          = EVO_TYPE_1,
    [SPECIES_CLEFABLE]          = EVO_TYPE_2,
    [SPECIES_VULPIX]            = EVO_TYPE_0,
    [SPECIES_NINETALES]         = EVO_TYPE_1,
    [SPECIES_JIGGLYPUFF]        = EVO_TYPE_1,
    [SPECIES_WIGGLYTUFF]        = EVO_TYPE_2,
    [SPECIES_ZUBAT]             = EVO_TYPE_0,
    [SPECIES_GOLBAT]            = EVO_TYPE_1,
    [SPECIES_ODDISH]            = EVO_TYPE_0,
    [SPECIES_GLOOM]             = EVO_TYPE_1,
    [SPECIES_VILEPLUME]         = EVO_TYPE_2,
    [SPECIES_PARAS]             = EVO_TYPE_0,
    [SPECIES_PARASECT]          = EVO_TYPE_1,
    [SPECIES_VENONAT]           = EVO_TYPE_0,
    [SPECIES_VENOMOTH]          = EVO_TYPE_1,
    [SPECIES_DIGLETT]           = EVO_TYPE_0,
    [SPECIES_DUGTRIO]           = EVO_TYPE_1,
    [SPECIES_MEOWTH]            = EVO_TYPE_0,
    [SPECIES_PERSIAN]           = EVO_TYPE_1,
    [SPECIES_PSYDUCK]           = EVO_TYPE_0,
    [SPECIES_GOLDUCK]           = EVO_TYPE_1,
    [SPECIES_MANKEY]            = EVO_TYPE_0,
    [SPECIES_PRIMEAPE]          = EVO_TYPE_1,
    [SPECIES_GROWLITHE]         = EVO_TYPE_0,
    [SPECIES_ARCANINE]          = EVO_TYPE_1,
    [SPECIES_POLIWAG]           = EVO_TYPE_0,
    [SPECIES_POLIWHIRL]         = EVO_TYPE_1,
    [SPECIES_POLIWRATH]         = EVO_TYPE_2,
    [SPECIES_ABRA]              = EVO_TYPE_0,
    [SPECIES_KADABRA]           = EVO_TYPE_1,
    [SPECIES_ALAKAZAM]          = EVO_TYPE_2,
    [SPECIES_MACHOP]            = EVO_TYPE_0,
    [SPECIES_MACHOKE]           = EVO_TYPE_1,
    [SPECIES_MACHAMP]           = EVO_TYPE_2,
    [SPECIES_BELLSPROUT]        = EVO_TYPE_0,
    [SPECIES_WEEPINBELL]        = EVO_TYPE_1,
    [SPECIES_VICTREEBEL]        = EVO_TYPE_2,
    [SPECIES_TENTACOOL]         = EVO_TYPE_0,
    [SPECIES_TENTACRUEL]        = EVO_TYPE_1,
    [SPECIES_GEODUDE]           = EVO_TYPE_0,
    [SPECIES_GRAVELER]          = EVO_TYPE_1,
    [SPECIES_GOLEM]             = EVO_TYPE_2,
    [SPECIES_PONYTA]            = EVO_TYPE_0,
    [SPECIES_RAPIDASH]          = EVO_TYPE_1,
    [SPECIES_SLOWPOKE]          = EVO_TYPE_0,
    [SPECIES_SLOWBRO]           = EVO_TYPE_2,
    [SPECIES_MAGNEMITE]         = EVO_TYPE_0,
    [SPECIES_MAGNETON]          = EVO_TYPE_1,
    [SPECIES_FARFETCHD]         = EVO_TYPE_0,
    [SPECIES_DODUO]             = EVO_TYPE_0,
    [SPECIES_DODRIO]            = EVO_TYPE_1,
    [SPECIES_SEEL]              = EVO_TYPE_0,
    [SPECIES_DEWGONG]           = EVO_TYPE_1,
    [SPECIES_GRIMER]            = EVO_TYPE_0,
    [SPECIES_MUK]               = EVO_TYPE_1,
    [SPECIES_SHELLDER]          = EVO_TYPE_0,
    [SPECIES_CLOYSTER]          = EVO_TYPE_1,
    [SPECIES_GASTLY]            = EVO_TYPE_0,
    [SPECIES_HAUNTER]           = EVO_TYPE_1,
    [SPECIES_GENGAR]            = EVO_TYPE_2,
    [SPECIES_ONIX]              = EVO_TYPE_0,
    [SPECIES_DROWZEE]           = EVO_TYPE_0,
    [SPECIES_HYPNO]             = EVO_TYPE_1,
    [SPECIES_KRABBY]            = EVO_TYPE_0,
    [SPECIES_KINGLER]           = EVO_TYPE_1,
    [SPECIES_VOLTORB]           = EVO_TYPE_0,
    [SPECIES_ELECTRODE]         = EVO_TYPE_1,
    [SPECIES_EXEGGCUTE]         = EVO_TYPE_0,
    [SPECIES_EXEGGUTOR]         = EVO_TYPE_1,
    [SPECIES_CUBONE]            = EVO_TYPE_0,
    [SPECIES_MAROWAK]           = EVO_TYPE_1,
    [SPECIES_HITMONLEE]         = EVO_TYPE_1,
    [SPECIES_HITMONCHAN]        = EVO_TYPE_1,
    [SPECIES_LICKITUNG]         = EVO_TYPE_0,
    [SPECIES_KOFFING]           = EVO_TYPE_0,
    [SPECIES_WEEZING]           = EVO_TYPE_1,
    [SPECIES_RHYHORN]           = EVO_TYPE_0,
    [SPECIES_RHYDON]            = EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    [SPECIES_CHANSEY]           = EVO_TYPE_1,
    #else
    [SPECIES_CHANSEY]           = EVO_TYPE_0,
    #endif
    [SPECIES_TANGELA]           = EVO_TYPE_0,
    [SPECIES_KANGASKHAN]        = EVO_TYPE_0,
    [SPECIES_HORSEA]            = EVO_TYPE_0,
    [SPECIES_SEADRA]            = EVO_TYPE_1,
    [SPECIES_GOLDEEN]           = EVO_TYPE_0,
    [SPECIES_SEAKING]           = EVO_TYPE_1,
    [SPECIES_STARYU]            = EVO_TYPE_0,
    [SPECIES_STARMIE]           = EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    [SPECIES_MR_MIME]           = EVO_TYPE_1,
    #else
    [SPECIES_MR_MIME]           = EVO_TYPE_0,
    #endif
    [SPECIES_SCYTHER]           = EVO_TYPE_0,
    [SPECIES_JYNX]              = EVO_TYPE_1,
    [SPECIES_ELECTABUZZ]        = EVO_TYPE_1,
    [SPECIES_MAGMAR]            = EVO_TYPE_1,
    [SPECIES_PINSIR]            = EVO_TYPE_0,
    [SPECIES_TAUROS]            = EVO_TYPE_0,
    [SPECIES_MAGIKARP]          = EVO_TYPE_0,
    [SPECIES_GYARADOS]          = EVO_TYPE_2,
    [SPECIES_LAPRAS]            = EVO_TYPE_0,
    [SPECIES_DITTO]             = EVO_TYPE_0,
    [SPECIES_EEVEE]             = EVO_TYPE_0,
    [SPECIES_VAPOREON]          = EVO_TYPE_1,
    [SPECIES_JOLTEON]           = EVO_TYPE_1,
    [SPECIES_FLAREON]           = EVO_TYPE_1,
    [SPECIES_PORYGON]           = EVO_TYPE_0,
    [SPECIES_OMANYTE]           = EVO_TYPE_0,
    [SPECIES_OMASTAR]           = EVO_TYPE_1,
    [SPECIES_KABUTO]            = EVO_TYPE_0,
    [SPECIES_KABUTOPS]          = EVO_TYPE_1,
    [SPECIES_AERODACTYL]        = EVO_TYPE_0,
    #ifdef POKEMON_EXPANSION
    [SPECIES_SNORLAX]           = EVO_TYPE_1,
    #else
    [SPECIES_SNORLAX]           = EVO_TYPE_0,
    #endif
    [SPECIES_ARTICUNO]          = EVO_TYPE_LEGENDARY,
    [SPECIES_ZAPDOS]            = EVO_TYPE_LEGENDARY,
    [SPECIES_MOLTRES]           = EVO_TYPE_LEGENDARY,
    [SPECIES_DRATINI]           = EVO_TYPE_0,
    [SPECIES_DRAGONAIR]         = EVO_TYPE_1,
    [SPECIES_DRAGONITE]         = EVO_TYPE_2,
    [SPECIES_MEWTWO]            = EVO_TYPE_LEGENDARY,
    [SPECIES_MEW]               = EVO_TYPE_LEGENDARY,
    [SPECIES_CHIKORITA]         = EVO_TYPE_0,
    [SPECIES_BAYLEEF]           = EVO_TYPE_1,
    [SPECIES_MEGANIUM]          = EVO_TYPE_2,
    [SPECIES_CYNDAQUIL]         = EVO_TYPE_0,
    [SPECIES_QUILAVA]           = EVO_TYPE_1,
    [SPECIES_TYPHLOSION]        = EVO_TYPE_2,
    [SPECIES_TOTODILE]          = EVO_TYPE_0,
    [SPECIES_CROCONAW]          = EVO_TYPE_1,
    [SPECIES_FERALIGATR]        = EVO_TYPE_2,
    [SPECIES_SENTRET]           = EVO_TYPE_0,
    [SPECIES_FURRET]            = EVO_TYPE_1,
    [SPECIES_HOOTHOOT]          = EVO_TYPE_0,
    [SPECIES_NOCTOWL]           = EVO_TYPE_1,
    [SPECIES_LEDYBA]            = EVO_TYPE_0,
    [SPECIES_LEDIAN]            = EVO_TYPE_1,
    [SPECIES_SPINARAK]          = EVO_TYPE_0,
    [SPECIES_ARIADOS]           = EVO_TYPE_1,
    [SPECIES_CROBAT]            = EVO_TYPE_2,
    [SPECIES_CHINCHOU]          = EVO_TYPE_0,
    [SPECIES_LANTURN]           = EVO_TYPE_1,
    [SPECIES_PICHU]             = EVO_TYPE_0,
    [SPECIES_CLEFFA]            = EVO_TYPE_0,
    [SPECIES_IGGLYBUFF]         = EVO_TYPE_0,
    [SPECIES_TOGEPI]            = EVO_TYPE_0,
    [SPECIES_TOGETIC]           = EVO_TYPE_1,
    [SPECIES_NATU]              = EVO_TYPE_0,
    [SPECIES_XATU]              = EVO_TYPE_1,
    [SPECIES_MAREEP]            = EVO_TYPE_0,
    [SPECIES_FLAAFFY]           = EVO_TYPE_1,
    [SPECIES_AMPHAROS]          = EVO_TYPE_2,
    [SPECIES_BELLOSSOM]         = EVO_TYPE_2,
    [SPECIES_MARILL]            = EVO_TYPE_1,
    [SPECIES_AZUMARILL]         = EVO_TYPE_2,
    #ifdef POKEMON_EXPANSION
    [SPECIES_SUDOWOODO]         = EVO_TYPE_1,
    #else
    [SPECIES_SUDOWOODO]         = EVO_TYPE_0,
    #endif
    [SPECIES_POLITOED]          = EVO_TYPE_2,
    [SPECIES_HOPPIP]            = EVO_TYPE_0,
    [SPECIES_SKIPLOOM]          = EVO_TYPE_1,
    [SPECIES_JUMPLUFF]          = EVO_TYPE_2,
    [SPECIES_AIPOM]             = EVO_TYPE_0,
    [SPECIES_SUNKERN]           = EVO_TYPE_0,
    [SPECIES_SUNFLORA]          = EVO_TYPE_1,
    [SPECIES_YANMA]             = EVO_TYPE_0,
    [SPECIES_WOOPER]            = EVO_TYPE_0,
    [SPECIES_QUAGSIRE]          = EVO_TYPE_1,
    [SPECIES_ESPEON]            = EVO_TYPE_1,
    [SPECIES_UMBREON]           = EVO_TYPE_1,
    [SPECIES_MURKROW]           = EVO_TYPE_0,
    [SPECIES_SLOWKING]          = EVO_TYPE_2,
    [SPECIES_MISDREAVUS]        = EVO_TYPE_0,
    [SPECIES_UNOWN]             = EVO_TYPE_0,
    [SPECIES_WOBBUFFET]         = EVO_TYPE_1,
    [SPECIES_GIRAFARIG]         = EVO_TYPE_0,
    [SPECIES_PINECO]            = EVO_TYPE_0,
    [SPECIES_FORRETRESS]        = EVO_TYPE_1,
    [SPECIES_DUNSPARCE]         = EVO_TYPE_0,
    [SPECIES_GLIGAR]            = EVO_TYPE_0,
    [SPECIES_STEELIX]           = EVO_TYPE_1,
    [SPECIES_SNUBBULL]          = EVO_TYPE_0,
    [SPECIES_GRANBULL]          = EVO_TYPE_1,
    [SPECIES_QWILFISH]          = EVO_TYPE_0,
    [SPECIES_SCIZOR]            = EVO_TYPE_1,
    [SPECIES_SHUCKLE]           = EVO_TYPE_0,
    [SPECIES_HERACROSS]         = EVO_TYPE_0,
    [SPECIES_SNEASEL]           = EVO_TYPE_0,
    [SPECIES_TEDDIURSA]         = EVO_TYPE_0,
    [SPECIES_URSARING]          = EVO_TYPE_1,
    [SPECIES_SLUGMA]            = EVO_TYPE_0,
    [SPECIES_MAGCARGO]          = EVO_TYPE_1,
    [SPECIES_SWINUB]            = EVO_TYPE_0,
    [SPECIES_PILOSWINE]         = EVO_TYPE_1,
    [SPECIES_CORSOLA]           = EVO_TYPE_0,
    [SPECIES_REMORAID]          = EVO_TYPE_0,
    [SPECIES_OCTILLERY]         = EVO_TYPE_1,
    [SPECIES_DELIBIRD]          = EVO_TYPE_0,
    #ifdef POKEMON_EXPANSION
    [SPECIES_MANTINE]           = EVO_TYPE_1,
    #else
    [SPECIES_MANTINE]           = EVO_TYPE_0,
    #endif
    [SPECIES_SKARMORY]          = EVO_TYPE_0,
    [SPECIES_HOUNDOUR]          = EVO_TYPE_0,
    [SPECIES_HOUNDOOM]          = EVO_TYPE_1,
    [SPECIES_KINGDRA]           = EVO_TYPE_2,
    [SPECIES_PHANPY]            = EVO_TYPE_0,
    [SPECIES_DONPHAN]           = EVO_TYPE_1,
    [SPECIES_PORYGON2]          = EVO_TYPE_1,
    [SPECIES_STANTLER]          = EVO_TYPE_0,
    [SPECIES_SMEARGLE]          = EVO_TYPE_0,
    [SPECIES_TYROGUE]           = EVO_TYPE_0,
    [SPECIES_HITMONTOP]         = EVO_TYPE_1,
    [SPECIES_SMOOCHUM]          = EVO_TYPE_0,
    [SPECIES_ELEKID]            = EVO_TYPE_0,
    [SPECIES_MAGBY]             = EVO_TYPE_0,
    [SPECIES_MILTANK]           = EVO_TYPE_0,
    #ifdef POKEMON_EXPANSION
    [SPECIES_BLISSEY]           = EVO_TYPE_2,
    #else
    [SPECIES_BLISSEY]           = EVO_TYPE_1,
    #endif
    [SPECIES_RAIKOU]            = EVO_TYPE_LEGENDARY,
    [SPECIES_ENTEI]             = EVO_TYPE_LEGENDARY,
    [SPECIES_SUICUNE]           = EVO_TYPE_LEGENDARY,
    [SPECIES_LARVITAR]          = EVO_TYPE_0,
    [SPECIES_PUPITAR]           = EVO_TYPE_1,
    [SPECIES_TYRANITAR]         = EVO_TYPE_2,
    [SPECIES_LUGIA]             = EVO_TYPE_LEGENDARY,
    [SPECIES_HO_OH]             = EVO_TYPE_LEGENDARY,
    [SPECIES_CELEBI]            = EVO_TYPE_LEGENDARY,
    #ifndef POKEMON_EXPANSION
    [SPECIES_OLD_UNOWN_B]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_C]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_D]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_E]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_F]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_G]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_H]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_I]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_J]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_K]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_L]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_M]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_N]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_O]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_P]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_Q]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_R]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_S]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_T]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_U]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_V]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_W]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_X]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_Y]       = EVO_TYPE_SELF,
    [SPECIES_OLD_UNOWN_Z]       = EVO_TYPE_SELF,
    #endif
    [SPECIES_TREECKO]           = EVO_TYPE_0,
    [SPECIES_GROVYLE]           = EVO_TYPE_1,
    [SPECIES_SCEPTILE]          = EVO_TYPE_2,
    [SPECIES_TORCHIC]           = EVO_TYPE_0,
    [SPECIES_COMBUSKEN]         = EVO_TYPE_1,
    [SPECIES_BLAZIKEN]          = EVO_TYPE_2,
    [SPECIES_MUDKIP]            = EVO_TYPE_0,
    [SPECIES_MARSHTOMP]         = EVO_TYPE_1,
    [SPECIES_SWAMPERT]          = EVO_TYPE_2,
    [SPECIES_POOCHYENA]         = EVO_TYPE_0,
    [SPECIES_MIGHTYENA]         = EVO_TYPE_1,
    [SPECIES_ZIGZAGOON]         = EVO_TYPE_0,
    [SPECIES_LINOONE]           = EVO_TYPE_1,
    [SPECIES_WURMPLE]           = EVO_TYPE_0,
    [SPECIES_SILCOON]           = EVO_TYPE_1,
    [SPECIES_BEAUTIFLY]         = EVO_TYPE_2,
    [SPECIES_CASCOON]           = EVO_TYPE_1,
    [SPECIES_DUSTOX]            = EVO_TYPE_2,
    [SPECIES_LOTAD]             = EVO_TYPE_0,
    [SPECIES_LOMBRE]            = EVO_TYPE_1,
    [SPECIES_LUDICOLO]          = EVO_TYPE_2,
    [SPECIES_SEEDOT]            = EVO_TYPE_0,
    [SPECIES_NUZLEAF]           = EVO_TYPE_1,
    [SPECIES_SHIFTRY]           = EVO_TYPE_2,
    [SPECIES_NINCADA]           = EVO_TYPE_0,
    [SPECIES_NINJASK]           = EVO_TYPE_1,
    [SPECIES_SHEDINJA]          = EVO_TYPE_1,
    [SPECIES_TAILLOW]           = EVO_TYPE_0,
    [SPECIES_SWELLOW]           = EVO_TYPE_1,
    [SPECIES_SHROOMISH]         = EVO_TYPE_0,
    [SPECIES_BRELOOM]           = EVO_TYPE_1,
    [SPECIES_SPINDA]            = EVO_TYPE_0,
    [SPECIES_WINGULL]           = EVO_TYPE_0,
    [SPECIES_PELIPPER]          = EVO_TYPE_1,
    [SPECIES_SURSKIT]           = EVO_TYPE_0,
    [SPECIES_MASQUERAIN]        = EVO_TYPE_1,
    [SPECIES_WAILMER]           = EVO_TYPE_0,
    [SPECIES_WAILORD]           = EVO_TYPE_1,
    [SPECIES_SKITTY]            = EVO_TYPE_0,
    [SPECIES_DELCATTY]          = EVO_TYPE_1,
    [SPECIES_KECLEON]           = EVO_TYPE_0,
    [SPECIES_BALTOY]            = EVO_TYPE_0,
    [SPECIES_CLAYDOL]           = EVO_TYPE_1,
    [SPECIES_NOSEPASS]          = EVO_TYPE_0,
    [SPECIES_TORKOAL]           = EVO_TYPE_0,
    [SPECIES_SABLEYE]           = EVO_TYPE_0,
    [SPECIES_BARBOACH]          = EVO_TYPE_0,
    [SPECIES_WHISCASH]          = EVO_TYPE_0,
    [SPECIES_LUVDISC]           = EVO_TYPE_0,
    [SPECIES_CORPHISH]          = EVO_TYPE_0,
    [SPECIES_CRAWDAUNT]         = EVO_TYPE_1,
    [SPECIES_FEEBAS]            = EVO_TYPE_0,
    [SPECIES_MILOTIC]           = EVO_TYPE_1,
    [SPECIES_CARVANHA]          = EVO_TYPE_0,
    [SPECIES_SHARPEDO]          = EVO_TYPE_1,
    [SPECIES_TRAPINCH]          = EVO_TYPE_0,
    [SPECIES_VIBRAVA]           = EVO_TYPE_1,
    [SPECIES_FLYGON]            = EVO_TYPE_2,
    [SPECIES_MAKUHITA]          = EVO_TYPE_0,
    [SPECIES_HARIYAMA]          = EVO_TYPE_1,
    [SPECIES_ELECTRIKE]         = EVO_TYPE_0,
    [SPECIES_MANECTRIC]         = EVO_TYPE_1,
    [SPECIES_NUMEL]             = EVO_TYPE_0,
    [SPECIES_CAMERUPT]          = EVO_TYPE_1,
    [SPECIES_SPHEAL]            = EVO_TYPE_0,
    [SPECIES_SEALEO]            = EVO_TYPE_1,
    [SPECIES_WALREIN]           = EVO_TYPE_2,
    [SPECIES_CACNEA]            = EVO_TYPE_0,
    [SPECIES_CACTURNE]          = EVO_TYPE_1,
    [SPECIES_SNORUNT]           = EVO_TYPE_0,
    [SPECIES_GLALIE]            = EVO_TYPE_1,
    [SPECIES_LUNATONE]          = EVO_TYPE_0,
    [SPECIES_SOLROCK]           = EVO_TYPE_0,
    [SPECIES_AZURILL]           = EVO_TYPE_0,
    [SPECIES_SPOINK]            = EVO_TYPE_0,
    [SPECIES_GRUMPIG]           = EVO_TYPE_1,
    [SPECIES_PLUSLE]            = EVO_TYPE_0,
    [SPECIES_MINUN]             = EVO_TYPE_0,
    [SPECIES_MAWILE]            = EVO_TYPE_0,
    [SPECIES_MEDITITE]          = EVO_TYPE_0,
    [SPECIES_MEDICHAM]          = EVO_TYPE_1,
    [SPECIES_SWABLU]            = EVO_TYPE_0,
    [SPECIES_ALTARIA]           = EVO_TYPE_1,
    [SPECIES_WYNAUT]            = EVO_TYPE_0,
    [SPECIES_DUSKULL]           = EVO_TYPE_0,
    [SPECIES_DUSCLOPS]          = EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    [SPECIES_ROSELIA]           = EVO_TYPE_1,
    #else
    [SPECIES_ROSELIA]           = EVO_TYPE_0,
    #endif
    [SPECIES_SLAKOTH]           = EVO_TYPE_0,
    [SPECIES_VIGOROTH]          = EVO_TYPE_1,
    [SPECIES_SLAKING]           = EVO_TYPE_2,
    [SPECIES_GULPIN]            = EVO_TYPE_0,
    [SPECIES_SWALOT]            = EVO_TYPE_1,
    [SPECIES_TROPIUS]           = EVO_TYPE_0,
    [SPECIES_WHISMUR]           = EVO_TYPE_0,
    [SPECIES_LOUDRED]           = EVO_TYPE_1,
    [SPECIES_EXPLOUD]           = EVO_TYPE_2,
    [SPECIES_CLAMPERL]          = EVO_TYPE_0,
    [SPECIES_HUNTAIL]           = EVO_TYPE_1,
    [SPECIES_GOREBYSS]          = EVO_TYPE_1,
    [SPECIES_ABSOL]             = EVO_TYPE_0,
    [SPECIES_SHUPPET]           = EVO_TYPE_0,
    [SPECIES_BANETTE]           = EVO_TYPE_1,
    [SPECIES_SEVIPER]           = EVO_TYPE_0,
    [SPECIES_ZANGOOSE]          = EVO_TYPE_0,
    [SPECIES_RELICANTH]         = EVO_TYPE_0,
    [SPECIES_ARON]              = EVO_TYPE_0,
    [SPECIES_LAIRON]            = EVO_TYPE_1,
    [SPECIES_AGGRON]            = EVO_TYPE_2,
    [SPECIES_CASTFORM]          = EVO_TYPE_SELF,
    [SPECIES_VOLBEAT]           = EVO_TYPE_1,
    [SPECIES_ILLUMISE]          = EVO_TYPE_1,
    [SPECIES_LILEEP]            = EVO_TYPE_0,
    [SPECIES_CRADILY]           = EVO_TYPE_1,
    [SPECIES_ANORITH]           = EVO_TYPE_0,
    [SPECIES_ARMALDO]           = EVO_TYPE_1,
    [SPECIES_RALTS]             = EVO_TYPE_0,
    [SPECIES_KIRLIA]            = EVO_TYPE_1,
    [SPECIES_GARDEVOIR]         = EVO_TYPE_2,
    [SPECIES_BAGON]             = EVO_TYPE_0,
    [SPECIES_SHELGON]           = EVO_TYPE_1,
    [SPECIES_SALAMENCE]         = EVO_TYPE_2,
    [SPECIES_BELDUM]            = EVO_TYPE_0,
    [SPECIES_METANG]            = EVO_TYPE_1,
    [SPECIES_METAGROSS]         = EVO_TYPE_2,
    [SPECIES_REGIROCK]          = EVO_TYPE_LEGENDARY,
    [SPECIES_REGICE]            = EVO_TYPE_LEGENDARY,
    [SPECIES_REGISTEEL]         = EVO_TYPE_LEGENDARY,
    [SPECIES_KYOGRE]            = EVO_TYPE_LEGENDARY,
    [SPECIES_GROUDON]           = EVO_TYPE_LEGENDARY,
    [SPECIES_RAYQUAZA]          = EVO_TYPE_LEGENDARY,
    [SPECIES_LATIAS]            = EVO_TYPE_LEGENDARY,
    [SPECIES_LATIOS]            = EVO_TYPE_LEGENDARY,
    [SPECIES_JIRACHI]           = EVO_TYPE_LEGENDARY,
    [SPECIES_DEOXYS]            = EVO_TYPE_LEGENDARY,
    #ifndef POKEMON_EXPANSION
    [SPECIES_CHIMECHO]          = EVO_TYPE_0,
    #else
    [SPECIES_CHIMECHO]          = EVO_TYPE_1,
    [SPECIES_TURTWIG]           = EVO_TYPE_0,
    [SPECIES_GROTLE]            = EVO_TYPE_1,
    [SPECIES_TORTERRA]          = EVO_TYPE_2,
    [SPECIES_CHIMCHAR]          = EVO_TYPE_0,
    [SPECIES_MONFERNO]          = EVO_TYPE_1,
    [SPECIES_INFERNAPE]         = EVO_TYPE_2,
    [SPECIES_PIPLUP]            = EVO_TYPE_0,
    [SPECIES_PRINPLUP]          = EVO_TYPE_1,
    [SPECIES_EMPOLEON]          = EVO_TYPE_2,
    [SPECIES_STARLY]            = EVO_TYPE_0,
    [SPECIES_STARAVIA]          = EVO_TYPE_1,
    [SPECIES_STARAPTOR]         = EVO_TYPE_2,
    [SPECIES_BIDOOF]            = EVO_TYPE_0,
    [SPECIES_BIBAREL]           = EVO_TYPE_1,
    [SPECIES_KRICKETOT]         = EVO_TYPE_0,
    [SPECIES_KRICKETUNE]        = EVO_TYPE_1,
    [SPECIES_SHINX]             = EVO_TYPE_0,
    [SPECIES_LUXIO]             = EVO_TYPE_1,
    [SPECIES_LUXRAY]            = EVO_TYPE_2,
    [SPECIES_BUDEW]             = EVO_TYPE_0,
    [SPECIES_ROSERADE]          = EVO_TYPE_2,
    [SPECIES_CRANIDOS]          = EVO_TYPE_0,
    [SPECIES_RAMPARDOS]         = EVO_TYPE_1,
    [SPECIES_SHIELDON]          = EVO_TYPE_0,
    [SPECIES_BASTIODON]         = EVO_TYPE_1,
    [SPECIES_BURMY]             = EVO_TYPE_0,
    [SPECIES_WORMADAM]          = EVO_TYPE_1,
    [SPECIES_MOTHIM]            = EVO_TYPE_1,
    [SPECIES_COMBEE]            = EVO_TYPE_0,
    [SPECIES_VESPIQUEN]         = EVO_TYPE_1,
    [SPECIES_PACHIRISU]         = EVO_TYPE_0,
    [SPECIES_BUIZEL]            = EVO_TYPE_0,
    [SPECIES_FLOATZEL]          = EVO_TYPE_1,
    [SPECIES_CHERUBI]           = EVO_TYPE_0,
    [SPECIES_CHERRIM]           = EVO_TYPE_1,
    [SPECIES_SHELLOS]           = EVO_TYPE_0,
    [SPECIES_GASTRODON]         = EVO_TYPE_1,
    [SPECIES_AMBIPOM]           = EVO_TYPE_1,
    [SPECIES_DRIFLOON]          = EVO_TYPE_0,
    [SPECIES_DRIFBLIM]          = EVO_TYPE_1,
    [SPECIES_BUNEARY]           = EVO_TYPE_0,
    [SPECIES_LOPUNNY]           = EVO_TYPE_1,
    [SPECIES_MISMAGIUS]         = EVO_TYPE_1,
    [SPECIES_HONCHKROW]         = EVO_TYPE_1,
    [SPECIES_GLAMEOW]           = EVO_TYPE_0,
    [SPECIES_PURUGLY]           = EVO_TYPE_1,
    [SPECIES_CHINGLING]         = EVO_TYPE_0,
    [SPECIES_STUNKY]            = EVO_TYPE_0,
    [SPECIES_SKUNTANK]          = EVO_TYPE_1,
    [SPECIES_BRONZOR]           = EVO_TYPE_0,
    [SPECIES_BRONZONG]          = EVO_TYPE_1,
    [SPECIES_BONSLY]            = EVO_TYPE_0,
    [SPECIES_MIME_JR]           = EVO_TYPE_0,
    [SPECIES_HAPPINY]           = EVO_TYPE_0,
    [SPECIES_CHATOT]            = EVO_TYPE_0,
    [SPECIES_SPIRITOMB]         = EVO_TYPE_0,
    [SPECIES_GIBLE]             = EVO_TYPE_0,
    [SPECIES_GABITE]            = EVO_TYPE_1,
    [SPECIES_GARCHOMP]          = EVO_TYPE_2,
    [SPECIES_MUNCHLAX]          = EVO_TYPE_0,
    [SPECIES_RIOLU]             = EVO_TYPE_0,
    [SPECIES_LUCARIO]           = EVO_TYPE_1,
    [SPECIES_HIPPOPOTAS]        = EVO_TYPE_0,
    [SPECIES_HIPPOWDON]         = EVO_TYPE_1,
    [SPECIES_SKORUPI]           = EVO_TYPE_0,
    [SPECIES_DRAPION]           = EVO_TYPE_1,
    [SPECIES_CROAGUNK]          = EVO_TYPE_0,
    [SPECIES_TOXICROAK]         = EVO_TYPE_1,
    [SPECIES_CARNIVINE]         = EVO_TYPE_0,
    [SPECIES_FINNEON]           = EVO_TYPE_0,
    [SPECIES_LUMINEON]          = EVO_TYPE_1,
    [SPECIES_MANTYKE]           = EVO_TYPE_0,
    [SPECIES_SNOVER]            = EVO_TYPE_0,
    [SPECIES_ABOMASNOW]         = EVO_TYPE_1,
    [SPECIES_WEAVILE]           = EVO_TYPE_1,
    [SPECIES_MAGNEZONE]         = EVO_TYPE_2,
    [SPECIES_LICKILICKY]        = EVO_TYPE_1,
    [SPECIES_RHYPERIOR]         = EVO_TYPE_2,
    [SPECIES_TANGROWTH]         = EVO_TYPE_1,
    [SPECIES_ELECTIVIRE]        = EVO_TYPE_2,
    [SPECIES_MAGMORTAR]         = EVO_TYPE_2,
    [SPECIES_TOGEKISS]          = EVO_TYPE_2,
    [SPECIES_YANMEGA]           = EVO_TYPE_1,
    [SPECIES_LEAFEON]           = EVO_TYPE_1,
    [SPECIES_GLACEON]           = EVO_TYPE_1,
    [SPECIES_GLISCOR]           = EVO_TYPE_1,
    [SPECIES_MAMOSWINE]         = EVO_TYPE_2,
    [SPECIES_PORYGON_Z]         = EVO_TYPE_2,
    [SPECIES_GALLADE]           = EVO_TYPE_2,
    [SPECIES_PROBOPASS]         = EVO_TYPE_1,
    [SPECIES_DUSKNOIR]          = EVO_TYPE_2,
    [SPECIES_FROSLASS]          = EVO_TYPE_1,
    [SPECIES_ROTOM]             = EVO_TYPE_0,
    [SPECIES_UXIE]              = EVO_TYPE_LEGENDARY,
    [SPECIES_MESPRIT]           = EVO_TYPE_LEGENDARY,
    [SPECIES_AZELF]             = EVO_TYPE_LEGENDARY,
    [SPECIES_DIALGA]            = EVO_TYPE_LEGENDARY,
    [SPECIES_PALKIA]            = EVO_TYPE_LEGENDARY,
    [SPECIES_HEATRAN]           = EVO_TYPE_LEGENDARY,
    [SPECIES_REGIGIGAS]         = EVO_TYPE_LEGENDARY,
    [SPECIES_GIRATINA]          = EVO_TYPE_LEGENDARY,
    [SPECIES_CRESSELIA]         = EVO_TYPE_LEGENDARY,
    [SPECIES_PHIONE]            = EVO_TYPE_LEGENDARY,
    [SPECIES_MANAPHY]           = EVO_TYPE_LEGENDARY,
    [SPECIES_DARKRAI]           = EVO_TYPE_LEGENDARY,
    [SPECIES_SHAYMIN]           = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS]            = EVO_TYPE_LEGENDARY,
    [SPECIES_VICTINI]           = EVO_TYPE_LEGENDARY,
    [SPECIES_SNIVY]             = EVO_TYPE_0,
    [SPECIES_SERVINE]           = EVO_TYPE_1,
    [SPECIES_SERPERIOR]         = EVO_TYPE_2,
    [SPECIES_TEPIG]             = EVO_TYPE_0,
    [SPECIES_PIGNITE]           = EVO_TYPE_1,
    [SPECIES_EMBOAR]            = EVO_TYPE_2,
    [SPECIES_OSHAWOTT]          = EVO_TYPE_0,
    [SPECIES_DEWOTT]            = EVO_TYPE_1,
    [SPECIES_SAMUROTT]          = EVO_TYPE_2,
    [SPECIES_PATRAT]            = EVO_TYPE_0,
    [SPECIES_WATCHOG]           = EVO_TYPE_1,
    [SPECIES_LILLIPUP]          = EVO_TYPE_0,
    [SPECIES_HERDIER]           = EVO_TYPE_1,
    [SPECIES_STOUTLAND]         = EVO_TYPE_2,
    [SPECIES_PURRLOIN]          = EVO_TYPE_0,
    [SPECIES_LIEPARD]           = EVO_TYPE_1,
    [SPECIES_PANSAGE]           = EVO_TYPE_0,
    [SPECIES_SIMISAGE]          = EVO_TYPE_1,
    [SPECIES_PANSEAR]           = EVO_TYPE_0,
    [SPECIES_SIMISEAR]          = EVO_TYPE_1,
    [SPECIES_PANPOUR]           = EVO_TYPE_0,
    [SPECIES_SIMIPOUR]          = EVO_TYPE_1,
    [SPECIES_MUNNA]             = EVO_TYPE_0,
    [SPECIES_MUSHARNA]          = EVO_TYPE_1,
    [SPECIES_PIDOVE]            = EVO_TYPE_0,
    [SPECIES_TRANQUILL]         = EVO_TYPE_1,
    [SPECIES_UNFEZANT]          = EVO_TYPE_2,
    [SPECIES_BLITZLE]           = EVO_TYPE_0,
    [SPECIES_ZEBSTRIKA]         = EVO_TYPE_1,
    [SPECIES_ROGGENROLA]        = EVO_TYPE_0,
    [SPECIES_BOLDORE]           = EVO_TYPE_1,
    [SPECIES_GIGALITH]          = EVO_TYPE_2,
    [SPECIES_WOOBAT]            = EVO_TYPE_0,
    [SPECIES_SWOOBAT]           = EVO_TYPE_1,
    [SPECIES_DRILBUR]           = EVO_TYPE_0,
    [SPECIES_EXCADRILL]         = EVO_TYPE_1,
    [SPECIES_AUDINO]            = EVO_TYPE_0,
    [SPECIES_TIMBURR]           = EVO_TYPE_0,
    [SPECIES_GURDURR]           = EVO_TYPE_1,
    [SPECIES_CONKELDURR]        = EVO_TYPE_2,
    [SPECIES_TYMPOLE]           = EVO_TYPE_0,
    [SPECIES_PALPITOAD]         = EVO_TYPE_1,
    [SPECIES_SEISMITOAD]        = EVO_TYPE_2,
    [SPECIES_THROH]             = EVO_TYPE_0,
    [SPECIES_SAWK]              = EVO_TYPE_0,
    [SPECIES_SEWADDLE]          = EVO_TYPE_0,
    [SPECIES_SWADLOON]          = EVO_TYPE_1,
    [SPECIES_LEAVANNY]          = EVO_TYPE_2,
    [SPECIES_VENIPEDE]          = EVO_TYPE_0,
    [SPECIES_WHIRLIPEDE]        = EVO_TYPE_1,
    [SPECIES_SCOLIPEDE]         = EVO_TYPE_2,
    [SPECIES_COTTONEE]          = EVO_TYPE_0,
    [SPECIES_WHIMSICOTT]        = EVO_TYPE_1,
    [SPECIES_PETILIL]           = EVO_TYPE_0,
    [SPECIES_LILLIGANT]         = EVO_TYPE_1,
    [SPECIES_BASCULIN]          = EVO_TYPE_0,
    [SPECIES_SANDILE]           = EVO_TYPE_0,
    [SPECIES_KROKOROK]          = EVO_TYPE_1,
    [SPECIES_KROOKODILE]        = EVO_TYPE_2,
    [SPECIES_DARUMAKA]          = EVO_TYPE_0,
    [SPECIES_DARMANITAN]        = EVO_TYPE_1,
    [SPECIES_MARACTUS]          = EVO_TYPE_0,
    [SPECIES_DWEBBLE]           = EVO_TYPE_0,
    [SPECIES_CRUSTLE]           = EVO_TYPE_1,
    [SPECIES_SCRAGGY]           = EVO_TYPE_0,
    [SPECIES_SCRAFTY]           = EVO_TYPE_1,
    [SPECIES_SIGILYPH]          = EVO_TYPE_0,
    [SPECIES_YAMASK]            = EVO_TYPE_0,
    [SPECIES_COFAGRIGUS]        = EVO_TYPE_1,
    [SPECIES_TIRTOUGA]          = EVO_TYPE_0,
    [SPECIES_CARRACOSTA]        = EVO_TYPE_1,
    [SPECIES_ARCHEN]            = EVO_TYPE_0,
    [SPECIES_ARCHEOPS]          = EVO_TYPE_1,
    [SPECIES_TRUBBISH]          = EVO_TYPE_0,
    [SPECIES_GARBODOR]          = EVO_TYPE_1,
    [SPECIES_ZORUA]             = EVO_TYPE_0,
    [SPECIES_ZOROARK]           = EVO_TYPE_1,
    [SPECIES_MINCCINO]          = EVO_TYPE_0,
    [SPECIES_CINCCINO]          = EVO_TYPE_1,
    [SPECIES_GOTHITA]           = EVO_TYPE_0,
    [SPECIES_GOTHORITA]         = EVO_TYPE_1,
    [SPECIES_GOTHITELLE]        = EVO_TYPE_2,
    [SPECIES_SOLOSIS]           = EVO_TYPE_0,
    [SPECIES_DUOSION]           = EVO_TYPE_1,
    [SPECIES_REUNICLUS]         = EVO_TYPE_2,
    [SPECIES_DUCKLETT]          = EVO_TYPE_0,
    [SPECIES_SWANNA]            = EVO_TYPE_1,
    [SPECIES_VANILLITE]         = EVO_TYPE_0,
    [SPECIES_VANILLISH]         = EVO_TYPE_1,
    [SPECIES_VANILLUXE]         = EVO_TYPE_2,
    [SPECIES_DEERLING]          = EVO_TYPE_0,
    [SPECIES_SAWSBUCK]          = EVO_TYPE_1,
    [SPECIES_EMOLGA]            = EVO_TYPE_0,
    [SPECIES_KARRABLAST]        = EVO_TYPE_0,
    [SPECIES_ESCAVALIER]        = EVO_TYPE_1,
    [SPECIES_FOONGUS]           = EVO_TYPE_0,
    [SPECIES_AMOONGUSS]         = EVO_TYPE_1,
    [SPECIES_FRILLISH]          = EVO_TYPE_0,
    [SPECIES_JELLICENT]         = EVO_TYPE_1,
    [SPECIES_ALOMOMOLA]         = EVO_TYPE_0,
    [SPECIES_JOLTIK]            = EVO_TYPE_0,
    [SPECIES_GALVANTULA]        = EVO_TYPE_1,
    [SPECIES_FERROSEED]         = EVO_TYPE_0,
    [SPECIES_FERROTHORN]        = EVO_TYPE_1,
    [SPECIES_KLINK]             = EVO_TYPE_0,
    [SPECIES_KLANG]             = EVO_TYPE_1,
    [SPECIES_KLINKLANG]         = EVO_TYPE_2,
    [SPECIES_TYNAMO]            = EVO_TYPE_0,
    [SPECIES_EELEKTRIK]         = EVO_TYPE_1,
    [SPECIES_EELEKTROSS]        = EVO_TYPE_2,
    [SPECIES_ELGYEM]            = EVO_TYPE_0,
    [SPECIES_BEHEEYEM]          = EVO_TYPE_1,
    [SPECIES_LITWICK]           = EVO_TYPE_0,
    [SPECIES_LAMPENT]           = EVO_TYPE_1,
    [SPECIES_CHANDELURE]        = EVO_TYPE_2,
    [SPECIES_AXEW]              = EVO_TYPE_0,
    [SPECIES_FRAXURE]           = EVO_TYPE_1,
    [SPECIES_HAXORUS]           = EVO_TYPE_2,
    [SPECIES_CUBCHOO]           = EVO_TYPE_0,
    [SPECIES_BEARTIC]           = EVO_TYPE_1,
    [SPECIES_CRYOGONAL]         = EVO_TYPE_0,
    [SPECIES_SHELMET]           = EVO_TYPE_0,
    [SPECIES_ACCELGOR]          = EVO_TYPE_1,
    [SPECIES_STUNFISK]          = EVO_TYPE_0,
    [SPECIES_MIENFOO]           = EVO_TYPE_0,
    [SPECIES_MIENSHAO]          = EVO_TYPE_1,
    [SPECIES_DRUDDIGON]         = EVO_TYPE_0,
    [SPECIES_GOLETT]            = EVO_TYPE_0,
    [SPECIES_GOLURK]            = EVO_TYPE_1,
    [SPECIES_PAWNIARD]          = EVO_TYPE_0,
    [SPECIES_BISHARP]           = EVO_TYPE_1,
    [SPECIES_BOUFFALANT]        = EVO_TYPE_0,
    [SPECIES_RUFFLET]           = EVO_TYPE_0,
    [SPECIES_BRAVIARY]          = EVO_TYPE_1,
    [SPECIES_VULLABY]           = EVO_TYPE_0,
    [SPECIES_MANDIBUZZ]         = EVO_TYPE_1,
    [SPECIES_HEATMOR]           = EVO_TYPE_0,
    [SPECIES_DURANT]            = EVO_TYPE_0,
    [SPECIES_DEINO]             = EVO_TYPE_0,
    [SPECIES_ZWEILOUS]          = EVO_TYPE_1,
    [SPECIES_HYDREIGON]         = EVO_TYPE_2,
    [SPECIES_LARVESTA]          = EVO_TYPE_0,
    [SPECIES_VOLCARONA]         = EVO_TYPE_1,
    [SPECIES_COBALION]          = EVO_TYPE_LEGENDARY,
    [SPECIES_TERRAKION]         = EVO_TYPE_LEGENDARY,
    [SPECIES_VIRIZION]          = EVO_TYPE_LEGENDARY,
    [SPECIES_TORNADUS]          = EVO_TYPE_LEGENDARY,
    [SPECIES_THUNDURUS]         = EVO_TYPE_LEGENDARY,
    [SPECIES_RESHIRAM]          = EVO_TYPE_LEGENDARY,
    [SPECIES_ZEKROM]            = EVO_TYPE_LEGENDARY,
    [SPECIES_LANDORUS]          = EVO_TYPE_LEGENDARY,
    [SPECIES_KYUREM]            = EVO_TYPE_LEGENDARY,
    [SPECIES_KELDEO]            = EVO_TYPE_LEGENDARY,
    [SPECIES_MELOETTA]          = EVO_TYPE_LEGENDARY,
    [SPECIES_GENESECT]          = EVO_TYPE_LEGENDARY,
    [SPECIES_CHESPIN]           = EVO_TYPE_0,
    [SPECIES_QUILLADIN]         = EVO_TYPE_1,
    [SPECIES_CHESNAUGHT]        = EVO_TYPE_2,
    [SPECIES_FENNEKIN]          = EVO_TYPE_0,
    [SPECIES_BRAIXEN]           = EVO_TYPE_1,
    [SPECIES_DELPHOX]           = EVO_TYPE_2,
    [SPECIES_FROAKIE]           = EVO_TYPE_0,
    [SPECIES_FROGADIER]         = EVO_TYPE_1,
    [SPECIES_GRENINJA]          = EVO_TYPE_2,
    [SPECIES_BUNNELBY]          = EVO_TYPE_0,
    [SPECIES_DIGGERSBY]         = EVO_TYPE_1,
    [SPECIES_FLETCHLING]        = EVO_TYPE_0,
    [SPECIES_FLETCHINDER]       = EVO_TYPE_1,
    [SPECIES_TALONFLAME]        = EVO_TYPE_2,
    [SPECIES_SCATTERBUG]        = EVO_TYPE_0,
    [SPECIES_SPEWPA]            = EVO_TYPE_1,
    [SPECIES_VIVILLON]          = EVO_TYPE_2,
    [SPECIES_LITLEO]            = EVO_TYPE_0,
    [SPECIES_PYROAR]            = EVO_TYPE_1,
    [SPECIES_FLABEBE]           = EVO_TYPE_0,
    [SPECIES_FLOETTE]           = EVO_TYPE_1,
    [SPECIES_FLORGES]           = EVO_TYPE_2,
    [SPECIES_SKIDDO]            = EVO_TYPE_0,
    [SPECIES_GOGOAT]            = EVO_TYPE_1,
    [SPECIES_PANCHAM]           = EVO_TYPE_0,
    [SPECIES_PANGORO]           = EVO_TYPE_1,
    [SPECIES_FURFROU]           = EVO_TYPE_0,
    [SPECIES_ESPURR]            = EVO_TYPE_0,
    [SPECIES_MEOWSTIC]          = EVO_TYPE_1,
    [SPECIES_HONEDGE]           = EVO_TYPE_0,
    [SPECIES_DOUBLADE]          = EVO_TYPE_1,
    [SPECIES_AEGISLASH]         = EVO_TYPE_2,
    [SPECIES_SPRITZEE]          = EVO_TYPE_0,
    [SPECIES_AROMATISSE]        = EVO_TYPE_1,
    [SPECIES_SWIRLIX]           = EVO_TYPE_0,
    [SPECIES_SLURPUFF]          = EVO_TYPE_1,
    [SPECIES_INKAY]             = EVO_TYPE_0,
    [SPECIES_MALAMAR]           = EVO_TYPE_1,
    [SPECIES_BINACLE]           = EVO_TYPE_0,
    [SPECIES_BARBARACLE]        = EVO_TYPE_1,
    [SPECIES_SKRELP]            = EVO_TYPE_0,
    [SPECIES_DRAGALGE]          = EVO_TYPE_1,
    [SPECIES_CLAUNCHER]         = EVO_TYPE_0,
    [SPECIES_CLAWITZER]         = EVO_TYPE_1,
    [SPECIES_HELIOPTILE]        = EVO_TYPE_0,
    [SPECIES_HELIOLISK]         = EVO_TYPE_1,
    [SPECIES_TYRUNT]            = EVO_TYPE_0,
    [SPECIES_TYRANTRUM]         = EVO_TYPE_1,
    [SPECIES_AMAURA]            = EVO_TYPE_0,
    [SPECIES_AURORUS]           = EVO_TYPE_1,
    [SPECIES_SYLVEON]           = EVO_TYPE_1,
    [SPECIES_HAWLUCHA]          = EVO_TYPE_0,
    [SPECIES_DEDENNE]           = EVO_TYPE_0,
    [SPECIES_CARBINK]           = EVO_TYPE_0,
    [SPECIES_GOOMY]             = EVO_TYPE_0,
    [SPECIES_SLIGGOO]           = EVO_TYPE_1,
    [SPECIES_GOODRA]            = EVO_TYPE_2,
    [SPECIES_KLEFKI]            = EVO_TYPE_0,
    [SPECIES_PHANTUMP]          = EVO_TYPE_0,
    [SPECIES_TREVENANT]         = EVO_TYPE_1,
    [SPECIES_PUMPKABOO]         = EVO_TYPE_0,
    [SPECIES_GOURGEIST]         = EVO_TYPE_1,
    [SPECIES_BERGMITE]          = EVO_TYPE_0,
    [SPECIES_AVALUGG]           = EVO_TYPE_1,
    [SPECIES_NOIBAT]            = EVO_TYPE_0,
    [SPECIES_NOIVERN]           = EVO_TYPE_1,
    [SPECIES_XERNEAS]           = EVO_TYPE_LEGENDARY,
    [SPECIES_YVELTAL]           = EVO_TYPE_LEGENDARY,
    [SPECIES_ZYGARDE]           = EVO_TYPE_LEGENDARY,
    [SPECIES_DIANCIE]           = EVO_TYPE_LEGENDARY,
    [SPECIES_HOOPA]             = EVO_TYPE_LEGENDARY,
    [SPECIES_VOLCANION]         = EVO_TYPE_LEGENDARY,
    [SPECIES_ROWLET]            = EVO_TYPE_0,
    [SPECIES_DARTRIX]           = EVO_TYPE_1,
    [SPECIES_DECIDUEYE]         = EVO_TYPE_2,
    [SPECIES_LITTEN]            = EVO_TYPE_0,
    [SPECIES_TORRACAT]          = EVO_TYPE_1,
    [SPECIES_INCINEROAR]        = EVO_TYPE_2,
    [SPECIES_POPPLIO]           = EVO_TYPE_0,
    [SPECIES_BRIONNE]           = EVO_TYPE_1,
    [SPECIES_PRIMARINA]         = EVO_TYPE_2,
    [SPECIES_PIKIPEK]           = EVO_TYPE_0,
    [SPECIES_TRUMBEAK]          = EVO_TYPE_1,
    [SPECIES_TOUCANNON]         = EVO_TYPE_2,
    [SPECIES_YUNGOOS]           = EVO_TYPE_0,
    [SPECIES_GUMSHOOS]          = EVO_TYPE_1,
    [SPECIES_GRUBBIN]           = EVO_TYPE_0,
    [SPECIES_CHARJABUG]         = EVO_TYPE_1,
    [SPECIES_VIKAVOLT]          = EVO_TYPE_2,
    [SPECIES_CRABRAWLER]        = EVO_TYPE_0,
    [SPECIES_CRABOMINABLE]      = EVO_TYPE_1,
    [SPECIES_ORICORIO]          = EVO_TYPE_0,
    [SPECIES_CUTIEFLY]          = EVO_TYPE_0,
    [SPECIES_RIBOMBEE]          = EVO_TYPE_1,
    [SPECIES_ROCKRUFF]          = EVO_TYPE_0,
    [SPECIES_LYCANROC]          = EVO_TYPE_1,
    [SPECIES_WISHIWASHI]        = EVO_TYPE_0,
    [SPECIES_MAREANIE]          = EVO_TYPE_0,
    [SPECIES_TOXAPEX]           = EVO_TYPE_1,
    [SPECIES_MUDBRAY]           = EVO_TYPE_0,
    [SPECIES_MUDSDALE]          = EVO_TYPE_1,
    [SPECIES_DEWPIDER]          = EVO_TYPE_0,
    [SPECIES_ARAQUANID]         = EVO_TYPE_1,
    [SPECIES_FOMANTIS]          = EVO_TYPE_0,
    [SPECIES_LURANTIS]          = EVO_TYPE_1,
    [SPECIES_MORELULL]          = EVO_TYPE_0,
    [SPECIES_SHIINOTIC]         = EVO_TYPE_1,
    [SPECIES_SALANDIT]          = EVO_TYPE_0,
    [SPECIES_SALAZZLE]          = EVO_TYPE_1,
    [SPECIES_STUFFUL]           = EVO_TYPE_0,
    [SPECIES_BEWEAR]            = EVO_TYPE_1,
    [SPECIES_BOUNSWEET]         = EVO_TYPE_0,
    [SPECIES_STEENEE]           = EVO_TYPE_1,
    [SPECIES_TSAREENA]          = EVO_TYPE_2,
    [SPECIES_COMFEY]            = EVO_TYPE_0,
    [SPECIES_ORANGURU]          = EVO_TYPE_0,
    [SPECIES_PASSIMIAN]         = EVO_TYPE_0,
    [SPECIES_WIMPOD]            = EVO_TYPE_0,
    [SPECIES_GOLISOPOD]         = EVO_TYPE_1,
    [SPECIES_SANDYGAST]         = EVO_TYPE_0,
    [SPECIES_PALOSSAND]         = EVO_TYPE_1,
    [SPECIES_PYUKUMUKU]         = EVO_TYPE_0,
    [SPECIES_TYPE_NULL]         = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY]          = EVO_TYPE_LEGENDARY,
    [SPECIES_MINIOR]            = EVO_TYPE_0,
    [SPECIES_KOMALA]            = EVO_TYPE_0,
    [SPECIES_TURTONATOR]        = EVO_TYPE_0,
    [SPECIES_TOGEDEMARU]        = EVO_TYPE_0,
    [SPECIES_MIMIKYU]           = EVO_TYPE_0,
    [SPECIES_BRUXISH]           = EVO_TYPE_0,
    [SPECIES_DRAMPA]            = EVO_TYPE_0,
    [SPECIES_DHELMISE]          = EVO_TYPE_0,
    [SPECIES_JANGMO_O]          = EVO_TYPE_0,
    [SPECIES_HAKAMO_O]          = EVO_TYPE_1,
    [SPECIES_KOMMO_O]           = EVO_TYPE_2,
    [SPECIES_TAPU_KOKO]         = EVO_TYPE_LEGENDARY,
    [SPECIES_TAPU_LELE]         = EVO_TYPE_LEGENDARY,
    [SPECIES_TAPU_BULU]         = EVO_TYPE_LEGENDARY,
    [SPECIES_TAPU_FINI]         = EVO_TYPE_LEGENDARY,
    [SPECIES_COSMOG]            = EVO_TYPE_LEGENDARY,
    [SPECIES_COSMOEM]           = EVO_TYPE_LEGENDARY,
    [SPECIES_SOLGALEO]          = EVO_TYPE_LEGENDARY,
    [SPECIES_LUNALA]            = EVO_TYPE_LEGENDARY,
    [SPECIES_NIHILEGO]          = EVO_TYPE_LEGENDARY,
    [SPECIES_BUZZWOLE]          = EVO_TYPE_LEGENDARY,
    [SPECIES_PHEROMOSA]         = EVO_TYPE_LEGENDARY,
    [SPECIES_XURKITREE]         = EVO_TYPE_LEGENDARY,
    [SPECIES_CELESTEELA]        = EVO_TYPE_LEGENDARY,
    [SPECIES_KARTANA]           = EVO_TYPE_LEGENDARY,
    [SPECIES_GUZZLORD]          = EVO_TYPE_LEGENDARY,
    [SPECIES_NECROZMA]          = EVO_TYPE_LEGENDARY,
    [SPECIES_MAGEARNA]          = EVO_TYPE_LEGENDARY,
    [SPECIES_MARSHADOW]         = EVO_TYPE_LEGENDARY,
    [SPECIES_POIPOLE]           = EVO_TYPE_LEGENDARY,
    [SPECIES_NAGANADEL]         = EVO_TYPE_LEGENDARY,
    [SPECIES_STAKATAKA]         = EVO_TYPE_LEGENDARY,
    [SPECIES_BLACEPHALON]       = EVO_TYPE_LEGENDARY,
    [SPECIES_ZERAORA]           = EVO_TYPE_LEGENDARY,
    [SPECIES_MELTAN]            = EVO_TYPE_LEGENDARY,
    [SPECIES_MELMETAL]          = EVO_TYPE_LEGENDARY,
    [SPECIES_GROOKEY]           = EVO_TYPE_0,
    [SPECIES_THWACKEY]          = EVO_TYPE_1,
    [SPECIES_RILLABOOM]         = EVO_TYPE_2,
    [SPECIES_SCORBUNNY]         = EVO_TYPE_0,
    [SPECIES_RABOOT]            = EVO_TYPE_1,
    [SPECIES_CINDERACE]         = EVO_TYPE_2,
    [SPECIES_SOBBLE]            = EVO_TYPE_0,
    [SPECIES_DRIZZILE]          = EVO_TYPE_1,
    [SPECIES_INTELEON]          = EVO_TYPE_2,
    [SPECIES_SKWOVET]           = EVO_TYPE_0,
    [SPECIES_GREEDENT]          = EVO_TYPE_1,
    [SPECIES_ROOKIDEE]          = EVO_TYPE_0,
    [SPECIES_CORVISQUIRE]       = EVO_TYPE_1,
    [SPECIES_CORVIKNIGHT]       = EVO_TYPE_2,
    [SPECIES_BLIPBUG]           = EVO_TYPE_0,
    [SPECIES_DOTTLER]           = EVO_TYPE_1,
    [SPECIES_ORBEETLE]          = EVO_TYPE_2,
    [SPECIES_NICKIT]            = EVO_TYPE_0,
    [SPECIES_THIEVUL]           = EVO_TYPE_1,
    [SPECIES_GOSSIFLEUR]        = EVO_TYPE_0,
    [SPECIES_ELDEGOSS]          = EVO_TYPE_1,
    [SPECIES_WOOLOO]            = EVO_TYPE_0,
    [SPECIES_DUBWOOL]           = EVO_TYPE_1,
    [SPECIES_CHEWTLE]           = EVO_TYPE_0,
    [SPECIES_DREDNAW]           = EVO_TYPE_1,
    [SPECIES_YAMPER]            = EVO_TYPE_0,
    [SPECIES_BOLTUND]           = EVO_TYPE_1,
    [SPECIES_ROLYCOLY]          = EVO_TYPE_0,
    [SPECIES_CARKOL]            = EVO_TYPE_1,
    [SPECIES_COALOSSAL]         = EVO_TYPE_2,
    [SPECIES_APPLIN]            = EVO_TYPE_0,
    [SPECIES_FLAPPLE]           = EVO_TYPE_1,
    [SPECIES_APPLETUN]          = EVO_TYPE_1,
    [SPECIES_SILICOBRA]         = EVO_TYPE_0,
    [SPECIES_SANDACONDA]        = EVO_TYPE_1,
    [SPECIES_CRAMORANT]         = EVO_TYPE_0,
    [SPECIES_ARROKUDA]          = EVO_TYPE_0,
    [SPECIES_BARRASKEWDA]       = EVO_TYPE_1,
    [SPECIES_TOXEL]             = EVO_TYPE_0,
    [SPECIES_TOXTRICITY]        = EVO_TYPE_1,
    [SPECIES_SIZZLIPEDE]        = EVO_TYPE_0,
    [SPECIES_CENTISKORCH]       = EVO_TYPE_1,
    [SPECIES_CLOBBOPUS]         = EVO_TYPE_0,
    [SPECIES_GRAPPLOCT]         = EVO_TYPE_1,
    [SPECIES_SINISTEA]          = EVO_TYPE_0,
    [SPECIES_POLTEAGEIST]       = EVO_TYPE_1,
    [SPECIES_HATENNA]           = EVO_TYPE_0,
    [SPECIES_HATTREM]           = EVO_TYPE_1,
    [SPECIES_HATTERENE]         = EVO_TYPE_2,
    [SPECIES_IMPIDIMP]          = EVO_TYPE_0,
    [SPECIES_MORGREM]           = EVO_TYPE_1,
    [SPECIES_GRIMMSNARL]        = EVO_TYPE_2,
    [SPECIES_OBSTAGOON]         = EVO_TYPE_2,
    [SPECIES_PERRSERKER]        = EVO_TYPE_1,
    [SPECIES_CURSOLA]           = EVO_TYPE_1,
    [SPECIES_SIRFETCHD]         = EVO_TYPE_1,
    [SPECIES_MR_RIME]           = EVO_TYPE_2,
    [SPECIES_RUNERIGUS]         = EVO_TYPE_1,
    [SPECIES_MILCERY]           = EVO_TYPE_0,
    [SPECIES_ALCREMIE]          = EVO_TYPE_1,
    [SPECIES_FALINKS]           = EVO_TYPE_0,
    [SPECIES_PINCURCHIN]        = EVO_TYPE_0,
    [SPECIES_SNOM]              = EVO_TYPE_0,
    [SPECIES_FROSMOTH]          = EVO_TYPE_1,
    [SPECIES_STONJOURNER]       = EVO_TYPE_0,
    [SPECIES_EISCUE]            = EVO_TYPE_0,
    [SPECIES_INDEEDEE]          = EVO_TYPE_0,
    [SPECIES_MORPEKO]           = EVO_TYPE_0,
    [SPECIES_CUFANT]            = EVO_TYPE_0,
    [SPECIES_COPPERAJAH]        = EVO_TYPE_1,
    [SPECIES_DRACOZOLT]         = EVO_TYPE_0,
    [SPECIES_ARCTOZOLT]         = EVO_TYPE_0,
    [SPECIES_DRACOVISH]         = EVO_TYPE_0,
    [SPECIES_ARCTOVISH]         = EVO_TYPE_0,
    [SPECIES_DURALUDON]         = EVO_TYPE_0,
    [SPECIES_DREEPY]            = EVO_TYPE_0,
    [SPECIES_DRAKLOAK]          = EVO_TYPE_1,
    [SPECIES_DRAGAPULT]         = EVO_TYPE_2,
    [SPECIES_ZACIAN]            = EVO_TYPE_LEGENDARY,
    [SPECIES_ZAMAZENTA]         = EVO_TYPE_LEGENDARY,
    [SPECIES_ETERNATUS]         = EVO_TYPE_LEGENDARY,
    [SPECIES_KUBFU]             = EVO_TYPE_LEGENDARY,
    [SPECIES_URSHIFU]           = EVO_TYPE_LEGENDARY,
    [SPECIES_ZARUDE]            = EVO_TYPE_LEGENDARY,
    [SPECIES_REGIELEKI]         = EVO_TYPE_LEGENDARY,
    [SPECIES_REGIDRAGO]         = EVO_TYPE_LEGENDARY,
    [SPECIES_GLASTRIER]         = EVO_TYPE_LEGENDARY,
    [SPECIES_SPECTRIER]         = EVO_TYPE_LEGENDARY,
    [SPECIES_CALYREX]           = EVO_TYPE_LEGENDARY,
    [SPECIES_VENUSAUR_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_CHARIZARD_MEGA_X]  = EVO_TYPE_SELF,
    [SPECIES_CHARIZARD_MEGA_Y]  = EVO_TYPE_SELF,
    [SPECIES_BLASTOISE_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_BEEDRILL_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_PIDGEOT_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_ALAKAZAM_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_SLOWBRO_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_GENGAR_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_KANGASKHAN_MEGA]   = EVO_TYPE_SELF,
    [SPECIES_PINSIR_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_GYARADOS_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_AERODACTYL_MEGA]   = EVO_TYPE_SELF,
    [SPECIES_MEWTWO_MEGA_X]     = EVO_TYPE_SELF,
    [SPECIES_MEWTWO_MEGA_Y]     = EVO_TYPE_SELF,
    [SPECIES_AMPHAROS_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_STEELIX_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_SCIZOR_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_HERACROSS_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_HOUNDOOM_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_TYRANITAR_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_SCEPTILE_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_BLAZIKEN_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_SWAMPERT_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_GARDEVOIR_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_SABLEYE_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_MAWILE_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_AGGRON_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_MEDICHAM_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_MANECTRIC_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_SHARPEDO_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_CAMERUPT_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_ALTARIA_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_BANETTE_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_ABSOL_MEGA]        = EVO_TYPE_SELF,
    [SPECIES_GLALIE_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_SALAMENCE_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_METAGROSS_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_LATIAS_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_LATIOS_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_LOPUNNY_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_GARCHOMP_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_LUCARIO_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_ABOMASNOW_MEGA]    = EVO_TYPE_SELF,
    [SPECIES_GALLADE_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_AUDINO_MEGA]       = EVO_TYPE_SELF,
    [SPECIES_DIANCIE_MEGA]      = EVO_TYPE_SELF,
    [SPECIES_RAYQUAZA_MEGA]     = EVO_TYPE_SELF,
    [SPECIES_KYOGRE_PRIMAL]     = EVO_TYPE_SELF,
    [SPECIES_GROUDON_PRIMAL]    = EVO_TYPE_SELF,
    [SPECIES_RATTATA_ALOLAN]    = EVO_TYPE_0,
    [SPECIES_RATICATE_ALOLAN]   = EVO_TYPE_1,
    [SPECIES_RAICHU_ALOLAN]     = EVO_TYPE_2,
    [SPECIES_SANDSHREW_ALOLAN]  = EVO_TYPE_0,
    [SPECIES_SANDSLASH_ALOLAN]  = EVO_TYPE_1,
    [SPECIES_VULPIX_ALOLAN]     = EVO_TYPE_0,
    [SPECIES_NINETALES_ALOLAN]  = EVO_TYPE_1,
    [SPECIES_DIGLETT_ALOLAN]    = EVO_TYPE_0,
    [SPECIES_DUGTRIO_ALOLAN]    = EVO_TYPE_1,
    [SPECIES_MEOWTH_ALOLAN]     = EVO_TYPE_0,
    [SPECIES_PERSIAN_ALOLAN]    = EVO_TYPE_1,
    [SPECIES_GEODUDE_ALOLAN]    = EVO_TYPE_0,
    [SPECIES_GRAVELER_ALOLAN]   = EVO_TYPE_1,
    [SPECIES_GOLEM_ALOLAN]      = EVO_TYPE_2,
    [SPECIES_GRIMER_ALOLAN]     = EVO_TYPE_0,
    [SPECIES_MUK_ALOLAN]        = EVO_TYPE_1,
    [SPECIES_EXEGGUTOR_ALOLAN]  = EVO_TYPE_1,
    [SPECIES_MAROWAK_ALOLAN]    = EVO_TYPE_1,
    [SPECIES_MEOWTH_GALARIAN]   = EVO_TYPE_0,
    [SPECIES_PONYTA_GALARIAN]   = EVO_TYPE_0,
    [SPECIES_RAPIDASH_GALARIAN] = EVO_TYPE_1,
    [SPECIES_SLOWPOKE_GALARIAN] = EVO_TYPE_0,
    [SPECIES_SLOWBRO_GALARIAN]  = EVO_TYPE_1,
    [SPECIES_FARFETCHD_GALARIAN] = EVO_TYPE_0,
    [SPECIES_WEEZING_GALARIAN]  = EVO_TYPE_1,
    [SPECIES_MR_MIME_GALARIAN]  = EVO_TYPE_1,
    [SPECIES_ARTICUNO_GALARIAN] = EVO_TYPE_LEGENDARY,
    [SPECIES_ZAPDOS_GALARIAN]   = EVO_TYPE_LEGENDARY,
    [SPECIES_MOLTRES_GALARIAN]  = EVO_TYPE_LEGENDARY,
    [SPECIES_SLOWKING_GALARIAN] = EVO_TYPE_1,
    [SPECIES_CORSOLA_GALARIAN]  = EVO_TYPE_1,
    [SPECIES_ZIGZAGOON_GALARIAN] = EVO_TYPE_0,
    [SPECIES_LINOONE_GALARIAN]  = EVO_TYPE_1,
    [SPECIES_DARUMAKA_GALARIAN] = EVO_TYPE_0,
    [SPECIES_DARMANITAN_GALARIAN] = EVO_TYPE_1,
    [SPECIES_YAMASK_GALARIAN]   = EVO_TYPE_0,
    [SPECIES_STUNFISK_GALARIAN] = EVO_TYPE_0,
    [SPECIES_PIKACHU_COSPLAY]   = EVO_TYPE_0,
    [SPECIES_PIKACHU_ROCK_STAR] = EVO_TYPE_0,
    [SPECIES_PIKACHU_BELLE]     = EVO_TYPE_0,
    [SPECIES_PIKACHU_POP_STAR]  = EVO_TYPE_0,
    [SPECIES_PIKACHU_PH_D]      = EVO_TYPE_0,
    [SPECIES_PIKACHU_LIBRE]     = EVO_TYPE_0,
    [SPECIES_PIKACHU_ORIGINAL_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_HOENN_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_SINNOH_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_UNOVA_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_KALOS_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_ALOLA_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_PARTNER_CAP] = EVO_TYPE_0,
    [SPECIES_PIKACHU_WORLD_CAP] = EVO_TYPE_0,
    [SPECIES_PICHU_SPIKY_EARED] = EVO_TYPE_0,
    [SPECIES_UNOWN_B]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_C]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_D]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_E]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_F]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_G]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_H]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_I]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_J]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_K]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_L]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_M]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_N]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_O]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_P]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_Q]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_R]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_S]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_T]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_U]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_V]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_W]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_X]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_Y]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_Z]           = EVO_TYPE_SELF,
    [SPECIES_UNOWN_EMARK]       = EVO_TYPE_SELF,
    [SPECIES_UNOWN_QMARK]       = EVO_TYPE_SELF,
    [SPECIES_CASTFORM_SUNNY]    = EVO_TYPE_SELF,
    [SPECIES_CASTFORM_RAINY]    = EVO_TYPE_SELF,
    [SPECIES_CASTFORM_SNOWY]    = EVO_TYPE_SELF,
    [SPECIES_DEOXYS_ATTACK]     = EVO_TYPE_SELF,
    [SPECIES_DEOXYS_DEFENSE]    = EVO_TYPE_SELF,
    [SPECIES_DEOXYS_SPEED]      = EVO_TYPE_SELF,
    [SPECIES_BURMY_SANDY_CLOAK] = EVO_TYPE_0,
    [SPECIES_BURMY_TRASH_CLOAK] = EVO_TYPE_0,
    [SPECIES_WORMADAM_SANDY_CLOAK] = EVO_TYPE_1,
    [SPECIES_WORMADAM_TRASH_CLOAK] = EVO_TYPE_1,
    [SPECIES_CHERRIM_SUNSHINE]  = EVO_TYPE_1,
    [SPECIES_SHELLOS_EAST_SEA]  = EVO_TYPE_0,
    [SPECIES_GASTRODON_EAST_SEA] = EVO_TYPE_1,
    [SPECIES_ROTOM_HEAT]        = EVO_TYPE_0,
    [SPECIES_ROTOM_WASH]        = EVO_TYPE_0,
    [SPECIES_ROTOM_FROST]       = EVO_TYPE_0,
    [SPECIES_ROTOM_FAN]         = EVO_TYPE_0,
    [SPECIES_ROTOM_MOW]         = EVO_TYPE_0,
    [SPECIES_GIRATINA_ORIGIN]   = EVO_TYPE_LEGENDARY,
    [SPECIES_SHAYMIN_SKY]       = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_FIGHTING]   = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_FLYING]     = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_POISON]     = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_GROUND]     = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_ROCK]       = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_BUG]        = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_GHOST]      = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_STEEL]      = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_FIRE]       = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_WATER]      = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_GRASS]      = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_ELECTRIC]   = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_PSYCHIC]    = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_ICE]        = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_DRAGON]     = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_DARK]       = EVO_TYPE_LEGENDARY,
    [SPECIES_ARCEUS_FAIRY]      = EVO_TYPE_LEGENDARY,
    [SPECIES_BASCULIN_BLUE_STRIPED] = EVO_TYPE_0,
    [SPECIES_DARMANITAN_ZEN_MODE] = EVO_TYPE_1,
    [SPECIES_DARMANITAN_ZEN_MODE_GALARIAN] = EVO_TYPE_1,
    [SPECIES_DEERLING_SUMMER]   = EVO_TYPE_0,
    [SPECIES_DEERLING_AUTUMN]   = EVO_TYPE_0,
    [SPECIES_DEERLING_WINTER]   = EVO_TYPE_0,
    [SPECIES_SAWSBUCK_SUMMER]   = EVO_TYPE_1,
    [SPECIES_SAWSBUCK_AUTUMN]   = EVO_TYPE_1,
    [SPECIES_SAWSBUCK_WINTER]   = EVO_TYPE_1,
    [SPECIES_TORNADUS_THERIAN]  = EVO_TYPE_LEGENDARY,
    [SPECIES_THUNDURUS_THERIAN] = EVO_TYPE_LEGENDARY,
    [SPECIES_LANDORUS_THERIAN]  = EVO_TYPE_LEGENDARY,
    [SPECIES_KYUREM_WHITE]      = EVO_TYPE_LEGENDARY,
    [SPECIES_KYUREM_BLACK]      = EVO_TYPE_LEGENDARY,
    [SPECIES_KELDEO_RESOLUTE]   = EVO_TYPE_LEGENDARY,
    [SPECIES_MELOETTA_PIROUETTE] = EVO_TYPE_LEGENDARY,
    [SPECIES_GENESECT_DOUSE_DRIVE] = EVO_TYPE_LEGENDARY,
    [SPECIES_GENESECT_SHOCK_DRIVE] = EVO_TYPE_LEGENDARY,
    [SPECIES_GENESECT_BURN_DRIVE] = EVO_TYPE_LEGENDARY,
    [SPECIES_GENESECT_CHILL_DRIVE] = EVO_TYPE_LEGENDARY,
    [SPECIES_GRENINJA_BATTLE_BOND] = EVO_TYPE_2,
    [SPECIES_GRENINJA_ASH]      = EVO_TYPE_2,
    [SPECIES_VIVILLON_POLAR]    = EVO_TYPE_2,
    [SPECIES_VIVILLON_TUNDRA]   = EVO_TYPE_2,
    [SPECIES_VIVILLON_CONTINENTAL] = EVO_TYPE_2,
    [SPECIES_VIVILLON_GARDEN]   = EVO_TYPE_2,
    [SPECIES_VIVILLON_ELEGANT]  = EVO_TYPE_2,
    [SPECIES_VIVILLON_MEADOW]   = EVO_TYPE_2,
    [SPECIES_VIVILLON_MODERN]   = EVO_TYPE_2,
    [SPECIES_VIVILLON_MARINE]   = EVO_TYPE_2,
    [SPECIES_VIVILLON_ARCHIPELAGO] = EVO_TYPE_2,
    [SPECIES_VIVILLON_HIGH_PLAINS] = EVO_TYPE_2,
    [SPECIES_VIVILLON_SANDSTORM] = EVO_TYPE_2,
    [SPECIES_VIVILLON_RIVER]    = EVO_TYPE_2,
    [SPECIES_VIVILLON_MONSOON]  = EVO_TYPE_2,
    [SPECIES_VIVILLON_SAVANNA]  = EVO_TYPE_2,
    [SPECIES_VIVILLON_SUN]      = EVO_TYPE_2,
    [SPECIES_VIVILLON_OCEAN]    = EVO_TYPE_2,
    [SPECIES_VIVILLON_JUNGLE]   = EVO_TYPE_2,
    [SPECIES_VIVILLON_FANCY]    = EVO_TYPE_2,
    [SPECIES_VIVILLON_POKE_BALL] = EVO_TYPE_2,
    [SPECIES_FLABEBE_YELLOW_FLOWER] = EVO_TYPE_0,
    [SPECIES_FLABEBE_ORANGE_FLOWER] = EVO_TYPE_0,
    [SPECIES_FLABEBE_BLUE_FLOWER] = EVO_TYPE_0,
    [SPECIES_FLABEBE_WHITE_FLOWER] = EVO_TYPE_0,
    [SPECIES_FLOETTE_YELLOW_FLOWER] = EVO_TYPE_1,
    [SPECIES_FLOETTE_ORANGE_FLOWER] = EVO_TYPE_1,
    [SPECIES_FLOETTE_BLUE_FLOWER] = EVO_TYPE_1,
    [SPECIES_FLOETTE_WHITE_FLOWER] = EVO_TYPE_1,
    [SPECIES_FLOETTE_ETERNAL_FLOWER] = EVO_TYPE_0,
    [SPECIES_FLORGES_YELLOW_FLOWER] = EVO_TYPE_2,
    [SPECIES_FLORGES_ORANGE_FLOWER] = EVO_TYPE_2,
    [SPECIES_FLORGES_BLUE_FLOWER] = EVO_TYPE_2,
    [SPECIES_FLORGES_WHITE_FLOWER] = EVO_TYPE_2,
    [SPECIES_FURFROU_HEART_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_STAR_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_DIAMOND_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_DEBUTANTE_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_MATRON_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_DANDY_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_LA_REINE_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_KABUKI_TRIM] = EVO_TYPE_0,
    [SPECIES_FURFROU_PHARAOH_TRIM] = EVO_TYPE_0,
    [SPECIES_MEOWSTIC_FEMALE]   = EVO_TYPE_1,
    [SPECIES_AEGISLASH_BLADE]   = EVO_TYPE_2,
    [SPECIES_PUMPKABOO_SMALL]   = EVO_TYPE_0,
    [SPECIES_PUMPKABOO_LARGE]   = EVO_TYPE_0,
    [SPECIES_PUMPKABOO_SUPER]   = EVO_TYPE_0,
    [SPECIES_GOURGEIST_SMALL]   = EVO_TYPE_1,
    [SPECIES_GOURGEIST_LARGE]   = EVO_TYPE_1,
    [SPECIES_GOURGEIST_SUPER]   = EVO_TYPE_1,
    [SPECIES_XERNEAS_ACTIVE]    = EVO_TYPE_LEGENDARY,
    [SPECIES_ZYGARDE_10]        = EVO_TYPE_LEGENDARY,
    [SPECIES_ZYGARDE_10_POWER_CONSTRUCT] = EVO_TYPE_LEGENDARY,
    [SPECIES_ZYGARDE_50_POWER_CONSTRUCT] = EVO_TYPE_LEGENDARY,
    [SPECIES_ZYGARDE_COMPLETE]  = EVO_TYPE_LEGENDARY,
    [SPECIES_HOOPA_UNBOUND]     = EVO_TYPE_LEGENDARY,
    [SPECIES_ORICORIO_POM_POM]  = EVO_TYPE_0,
    [SPECIES_ORICORIO_PAU]      = EVO_TYPE_0,
    [SPECIES_ORICORIO_SENSU]    = EVO_TYPE_0,
    [SPECIES_ROCKRUFF_OWN_TEMPO] = EVO_TYPE_0,
    [SPECIES_LYCANROC_MIDNIGHT] = EVO_TYPE_1,
    [SPECIES_LYCANROC_DUSK]     = EVO_TYPE_1,
    [SPECIES_WISHIWASHI_SCHOOL] = EVO_TYPE_0,
    [SPECIES_SILVALLY_FIGHTING] = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_FLYING]   = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_POISON]   = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_GROUND]   = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_ROCK]     = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_BUG]      = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_GHOST]    = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_STEEL]    = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_FIRE]     = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_WATER]    = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_GRASS]    = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_ELECTRIC] = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_PSYCHIC]  = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_ICE]      = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_DRAGON]   = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_DARK]     = EVO_TYPE_LEGENDARY,
    [SPECIES_SILVALLY_FAIRY]    = EVO_TYPE_LEGENDARY,
    [SPECIES_MINIOR_METEOR_ORANGE] = EVO_TYPE_0,
    [SPECIES_MINIOR_METEOR_YELLOW] = EVO_TYPE_0,
    [SPECIES_MINIOR_METEOR_GREEN] = EVO_TYPE_0,
    [SPECIES_MINIOR_METEOR_BLUE] = EVO_TYPE_0,
    [SPECIES_MINIOR_METEOR_INDIGO] = EVO_TYPE_0,
    [SPECIES_MINIOR_METEOR_VIOLET] = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_RED]   = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_ORANGE] = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_YELLOW] = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_GREEN] = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_BLUE]  = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_INDIGO] = EVO_TYPE_0,
    [SPECIES_MINIOR_CORE_VIOLET] = EVO_TYPE_0,
    [SPECIES_MIMIKYU_BUSTED]    = EVO_TYPE_0,
    [SPECIES_NECROZMA_DUSK_MANE] = EVO_TYPE_LEGENDARY,
    [SPECIES_NECROZMA_DAWN_WINGS] = EVO_TYPE_LEGENDARY,
    [SPECIES_NECROZMA_ULTRA]    = EVO_TYPE_LEGENDARY,
    [SPECIES_MAGEARNA_ORIGINAL_COLOR] = EVO_TYPE_LEGENDARY,
    [SPECIES_CRAMORANT_GULPING] = EVO_TYPE_0,
    [SPECIES_CRAMORANT_GORGING] = EVO_TYPE_0,
    [SPECIES_TOXTRICITY_LOW_KEY] = EVO_TYPE_1,
    [SPECIES_SINISTEA_ANTIQUE]  = EVO_TYPE_0,
    [SPECIES_POLTEAGEIST_ANTIQUE] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_RUBY_CREAM] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_MATCHA_CREAM] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_MINT_CREAM] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_LEMON_CREAM] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_SALTED_CREAM] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_RUBY_SWIRL] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_CARAMEL_SWIRL] = EVO_TYPE_1,
    [SPECIES_ALCREMIE_RAINBOW_SWIRL] = EVO_TYPE_1,
    [SPECIES_EISCUE_NOICE_FACE] = EVO_TYPE_0,
    [SPECIES_INDEEDEE_FEMALE]   = EVO_TYPE_1,
    [SPECIES_MORPEKO_HANGRY]    = EVO_TYPE_0,
    [SPECIES_ZACIAN_CROWNED_SWORD] = EVO_TYPE_LEGENDARY,
    [SPECIES_ZAMAZENTA_CROWNED_SHIELD] = EVO_TYPE_LEGENDARY,
    [SPECIES_ETERNATUS_ETERNAMAX] = EVO_TYPE_LEGENDARY,
    [SPECIES_URSHIFU_RAPID_STRIKE_STYLE] = EVO_TYPE_LEGENDARY,
    [SPECIES_ZARUDE_DADA]       = EVO_TYPE_LEGENDARY,
    [SPECIES_CALYREX_ICE_RIDER] = EVO_TYPE_LEGENDARY,
    [SPECIES_CALYREX_SHADOW_RIDER] = EVO_TYPE_LEGENDARY,
    #endif
    [SPECIES_EGG]             = EVO_TYPE_SELF,
};


#define RANDOM_SPECIES_COUNT ARRAY_COUNT(sRandomSpecies)
static const u16 sRandomSpecies[] =
{
    //SPECIES_NONE                    ,
    SPECIES_TURTWIG           ,
    SPECIES_GROTLE            ,
    SPECIES_TORTERRA          ,
    SPECIES_CHIMCHAR          ,
    SPECIES_MONFERNO          ,
    SPECIES_INFERNAPE         ,
    SPECIES_PIPLUP            ,
    SPECIES_PRINPLUP          ,
    SPECIES_EMPOLEON          ,
    SPECIES_STARLY            ,
    SPECIES_STARAVIA          ,
    SPECIES_STARAPTOR         ,
    SPECIES_BIDOOF            ,
    SPECIES_BIBAREL           ,
    SPECIES_KRICKETOT         ,
    SPECIES_KRICKETUNE        ,
    SPECIES_SHINX             ,
    SPECIES_LUXIO             ,
    SPECIES_LUXRAY            ,
    SPECIES_BUDEW             ,
    SPECIES_ROSERADE          ,
    SPECIES_CRANIDOS          ,
    SPECIES_RAMPARDOS         ,
    SPECIES_SHIELDON          ,
    SPECIES_BASTIODON         ,
    SPECIES_BURMY             ,
    SPECIES_WORMADAM          ,
    SPECIES_MOTHIM            ,
    SPECIES_COMBEE            ,
    SPECIES_VESPIQUEN         ,
    SPECIES_PACHIRISU         ,
    SPECIES_BUIZEL            ,
    SPECIES_FLOATZEL          ,
    SPECIES_CHERUBI           ,
    SPECIES_CHERRIM           ,
    SPECIES_SHELLOS           ,
    SPECIES_GASTRODON         ,
    SPECIES_AMBIPOM           ,
    SPECIES_DRIFLOON          ,
    SPECIES_DRIFBLIM          ,
    SPECIES_BUNEARY           ,
    SPECIES_LOPUNNY           ,
    SPECIES_MISMAGIUS         ,
    SPECIES_HONCHKROW         ,
    SPECIES_GLAMEOW           ,
    SPECIES_PURUGLY           ,
    SPECIES_CHINGLING         ,
    SPECIES_STUNKY            ,
    SPECIES_SKUNTANK          ,
    SPECIES_BRONZOR           ,
    SPECIES_BRONZONG          ,
    SPECIES_BONSLY            ,
    SPECIES_MIME_JR           ,
    SPECIES_HAPPINY           ,
    SPECIES_CHATOT            ,
    SPECIES_SPIRITOMB         ,
    SPECIES_GIBLE             ,
    SPECIES_GABITE            ,
    SPECIES_GARCHOMP          ,
    SPECIES_MUNCHLAX          ,
    SPECIES_RIOLU             ,
    SPECIES_LUCARIO           ,
    SPECIES_HIPPOPOTAS        ,
    SPECIES_HIPPOWDON         ,
    SPECIES_SKORUPI           ,
    SPECIES_DRAPION           ,
    SPECIES_CROAGUNK          ,
    SPECIES_TOXICROAK         ,
    SPECIES_CARNIVINE         ,
    SPECIES_FINNEON           ,
    SPECIES_LUMINEON          ,
    SPECIES_MANTYKE           ,
    SPECIES_SNOVER            ,
    SPECIES_ABOMASNOW         ,
    SPECIES_WEAVILE           ,
    SPECIES_MAGNEZONE         ,
    SPECIES_LICKILICKY        ,
    SPECIES_RHYPERIOR         ,
    SPECIES_TANGROWTH         ,
    SPECIES_ELECTIVIRE        ,
    SPECIES_MAGMORTAR         ,
    SPECIES_TOGEKISS          ,
    SPECIES_YANMEGA           ,
    SPECIES_LEAFEON           ,
    SPECIES_GLACEON           ,
    SPECIES_GLISCOR           ,
    SPECIES_MAMOSWINE         ,
    SPECIES_PORYGON_Z         ,
    SPECIES_GALLADE           ,
    SPECIES_PROBOPASS         ,
    SPECIES_DUSKNOIR          ,
    SPECIES_FROSLASS          ,
    SPECIES_ROTOM             ,
    //SPECIES_UXIE              ,
    //SPECIES_MESPRIT           ,
    //SPECIES_AZELF             ,
    //SPECIES_DIALGA            ,
    //SPECIES_PALKIA            ,
    //SPECIES_HEATRAN           ,
    //SPECIES_REGIGIGAS         ,
    //SPECIES_GIRATINA          ,
    //SPECIES_CRESSELIA         ,
    //SPECIES_PHIONE            ,
    //SPECIES_MANAPHY           ,
    //SPECIES_DARKRAI           ,
    //SPECIES_SHAYMIN           ,
    //SPECIES_ARCEUS            ,
    //SPECIES_VICTINI           ,
    SPECIES_SNIVY             ,
    SPECIES_SERVINE           ,
    SPECIES_SERPERIOR         ,
    SPECIES_TEPIG             ,
    SPECIES_PIGNITE           ,
    SPECIES_EMBOAR            ,
    SPECIES_OSHAWOTT          ,
    SPECIES_DEWOTT            ,
    SPECIES_SAMUROTT          ,
    SPECIES_PATRAT            ,
    SPECIES_WATCHOG           ,
    SPECIES_LILLIPUP          ,
    SPECIES_HERDIER           ,
    SPECIES_STOUTLAND         ,
    SPECIES_PURRLOIN          ,
    SPECIES_LIEPARD           ,
    SPECIES_PANSAGE           ,
    SPECIES_SIMISAGE          ,
    SPECIES_PANSEAR           ,
    SPECIES_SIMISEAR          ,
    SPECIES_PANPOUR           ,
    SPECIES_SIMIPOUR          ,
    SPECIES_MUNNA             ,
    SPECIES_MUSHARNA          ,
    SPECIES_PIDOVE            ,
    SPECIES_TRANQUILL         ,
    SPECIES_UNFEZANT          ,
    SPECIES_BLITZLE           ,
    SPECIES_ZEBSTRIKA         ,
    SPECIES_ROGGENROLA        ,
    SPECIES_BOLDORE           ,
    SPECIES_GIGALITH          ,
    SPECIES_WOOBAT            ,
    SPECIES_SWOOBAT           ,
    SPECIES_DRILBUR           ,
    SPECIES_EXCADRILL         ,
    SPECIES_AUDINO            ,
    SPECIES_TIMBURR           ,
    SPECIES_GURDURR           ,
    SPECIES_CONKELDURR        ,
    SPECIES_TYMPOLE           ,
    SPECIES_PALPITOAD         ,
    SPECIES_SEISMITOAD        ,
    SPECIES_THROH             ,
    SPECIES_SAWK              ,
    SPECIES_SEWADDLE          ,
    SPECIES_SWADLOON          ,
    SPECIES_LEAVANNY          ,
    SPECIES_VENIPEDE          ,
    SPECIES_WHIRLIPEDE        ,
    SPECIES_SCOLIPEDE         ,
    SPECIES_COTTONEE          ,
    SPECIES_WHIMSICOTT        ,
    SPECIES_PETILIL           ,
    SPECIES_LILLIGANT         ,
    SPECIES_BASCULIN          ,
    SPECIES_SANDILE           ,
    SPECIES_KROKOROK          ,
    SPECIES_KROOKODILE        ,
    SPECIES_DARUMAKA          ,
    SPECIES_DARMANITAN        ,
    SPECIES_MARACTUS          ,
    SPECIES_DWEBBLE           ,
    SPECIES_CRUSTLE           ,
    SPECIES_SCRAGGY           ,
    SPECIES_SCRAFTY           ,
    SPECIES_SIGILYPH          ,
    SPECIES_YAMASK            ,
    SPECIES_COFAGRIGUS        ,
    SPECIES_TIRTOUGA          ,
    SPECIES_CARRACOSTA        ,
    SPECIES_ARCHEN            ,
    SPECIES_ARCHEOPS          ,
    SPECIES_TRUBBISH          ,
    SPECIES_GARBODOR          ,
    SPECIES_ZORUA             ,
    SPECIES_ZOROARK           ,
    SPECIES_MINCCINO          ,
    SPECIES_CINCCINO          ,
    SPECIES_GOTHITA           ,
    SPECIES_GOTHORITA         ,
    SPECIES_GOTHITELLE        ,
    SPECIES_SOLOSIS           ,
    SPECIES_DUOSION           ,
    SPECIES_REUNICLUS         ,
    SPECIES_DUCKLETT          ,
    SPECIES_SWANNA            ,
    SPECIES_VANILLITE         ,
    SPECIES_VANILLISH         ,
    SPECIES_VANILLUXE         ,
    SPECIES_DEERLING          ,
    SPECIES_SAWSBUCK          ,
    SPECIES_EMOLGA            ,
    SPECIES_KARRABLAST        ,
    SPECIES_ESCAVALIER        ,
    SPECIES_FOONGUS           ,
    SPECIES_AMOONGUSS         ,
    SPECIES_FRILLISH          ,
    SPECIES_JELLICENT         ,
    SPECIES_ALOMOMOLA         ,
    SPECIES_JOLTIK            ,
    SPECIES_GALVANTULA        ,
    SPECIES_FERROSEED         ,
    SPECIES_FERROTHORN        ,
    SPECIES_KLINK             ,
    SPECIES_KLANG             ,
    SPECIES_KLINKLANG         ,
    SPECIES_TYNAMO            ,
    SPECIES_EELEKTRIK         ,
    SPECIES_EELEKTROSS        ,
    SPECIES_ELGYEM            ,
    SPECIES_BEHEEYEM          ,
    SPECIES_LITWICK           ,
    SPECIES_LAMPENT           ,
    SPECIES_CHANDELURE        ,
    SPECIES_AXEW              ,
    SPECIES_FRAXURE           ,
    SPECIES_HAXORUS           ,
    SPECIES_CUBCHOO           ,
    SPECIES_BEARTIC           ,
    SPECIES_CRYOGONAL         ,
    SPECIES_SHELMET           ,
    SPECIES_ACCELGOR          ,
    SPECIES_STUNFISK          ,
    SPECIES_MIENFOO           ,
    SPECIES_MIENSHAO          ,
    SPECIES_DRUDDIGON         ,
    SPECIES_GOLETT            ,
    SPECIES_GOLURK            ,
    SPECIES_PAWNIARD          ,
    SPECIES_BISHARP           ,
    SPECIES_BOUFFALANT        ,
    SPECIES_RUFFLET           ,
    SPECIES_BRAVIARY          ,
    SPECIES_VULLABY           ,
    SPECIES_MANDIBUZZ         ,
    SPECIES_HEATMOR           ,
    SPECIES_DURANT            ,
    SPECIES_DEINO             ,
    SPECIES_ZWEILOUS          ,
    SPECIES_HYDREIGON         ,
    SPECIES_LARVESTA          ,
    SPECIES_VOLCARONA         ,
    //SPECIES_COBALION          ,
    //SPECIES_TERRAKION         ,
    //SPECIES_VIRIZION          ,
    //SPECIES_TORNADUS          ,
    //SPECIES_THUNDURUS         ,
    //SPECIES_RESHIRAM          ,
    //SPECIES_ZEKROM            ,
    //SPECIES_LANDORUS          ,
    //SPECIES_KYUREM            ,
    //SPECIES_KELDEO            ,
    //SPECIES_MELOETTA          ,
    //SPECIES_GENESECT          ,
    SPECIES_CHESPIN           ,
    SPECIES_QUILLADIN         ,
    SPECIES_CHESNAUGHT        ,
    SPECIES_FENNEKIN          ,
    SPECIES_BRAIXEN           ,
    SPECIES_DELPHOX           ,
    SPECIES_FROAKIE           ,
    SPECIES_FROGADIER         ,
    SPECIES_GRENINJA          ,
    SPECIES_BUNNELBY          ,
    SPECIES_DIGGERSBY         ,
    SPECIES_FLETCHLING        ,
    SPECIES_FLETCHINDER       ,
    SPECIES_TALONFLAME        ,
    SPECIES_SCATTERBUG        ,
    SPECIES_SPEWPA            ,
    SPECIES_VIVILLON          ,
    SPECIES_LITLEO            ,
    SPECIES_PYROAR            ,
    SPECIES_FLABEBE           ,
    SPECIES_FLOETTE           ,
    SPECIES_FLORGES           ,
    SPECIES_SKIDDO            ,
    SPECIES_GOGOAT            ,
    SPECIES_PANCHAM           ,
    SPECIES_PANGORO           ,
    SPECIES_FURFROU           ,
    SPECIES_ESPURR            ,
    SPECIES_MEOWSTIC          ,
    SPECIES_HONEDGE           ,
    SPECIES_DOUBLADE          ,
    SPECIES_AEGISLASH         ,
    SPECIES_SPRITZEE          ,
    SPECIES_AROMATISSE        ,
    SPECIES_SWIRLIX           ,
    SPECIES_SLURPUFF          ,
    SPECIES_INKAY             ,
    SPECIES_MALAMAR           ,
    SPECIES_BINACLE           ,
    SPECIES_BARBARACLE        ,
    SPECIES_SKRELP            ,
    SPECIES_DRAGALGE          ,
    SPECIES_CLAUNCHER         ,
    SPECIES_CLAWITZER         ,
    SPECIES_HELIOPTILE        ,
    SPECIES_HELIOLISK         ,
    SPECIES_TYRUNT            ,
    SPECIES_TYRANTRUM         ,
    SPECIES_AMAURA            ,
    SPECIES_AURORUS           ,
    SPECIES_SYLVEON           ,
    SPECIES_HAWLUCHA          ,
    SPECIES_DEDENNE           ,
    SPECIES_CARBINK           ,
    SPECIES_GOOMY             ,
    SPECIES_SLIGGOO           ,
    SPECIES_GOODRA            ,
    SPECIES_KLEFKI            ,
    SPECIES_PHANTUMP          ,
    SPECIES_TREVENANT         ,
    SPECIES_PUMPKABOO         ,
    SPECIES_GOURGEIST         ,
    SPECIES_BERGMITE          ,
    SPECIES_AVALUGG           ,
    SPECIES_NOIBAT            ,
    SPECIES_NOIVERN           ,
    //SPECIES_XERNEAS           ,
    //SPECIES_YVELTAL           ,
    //SPECIES_ZYGARDE           ,
    //SPECIES_DIANCIE           ,
    //SPECIES_HOOPA             ,
    //SPECIES_VOLCANION         ,
    SPECIES_ROWLET            ,
    SPECIES_DARTRIX           ,
    SPECIES_DECIDUEYE         ,
    SPECIES_LITTEN            ,
    SPECIES_TORRACAT          ,
    SPECIES_INCINEROAR        ,
    SPECIES_POPPLIO           ,
    SPECIES_BRIONNE           ,
    SPECIES_PRIMARINA         ,
    SPECIES_PIKIPEK           ,
    SPECIES_TRUMBEAK          ,
    SPECIES_TOUCANNON         ,
    SPECIES_YUNGOOS           ,
    SPECIES_GUMSHOOS          ,
    SPECIES_GRUBBIN           ,
    SPECIES_CHARJABUG         ,
    SPECIES_VIKAVOLT          ,
    SPECIES_CRABRAWLER        ,
    SPECIES_CRABOMINABLE      ,
    SPECIES_ORICORIO          ,
    SPECIES_CUTIEFLY          ,
    SPECIES_RIBOMBEE          ,
    SPECIES_ROCKRUFF          ,
    SPECIES_LYCANROC          ,
    SPECIES_WISHIWASHI        ,
    SPECIES_MAREANIE          ,
    SPECIES_TOXAPEX           ,
    SPECIES_MUDBRAY           ,
    SPECIES_MUDSDALE          ,
    SPECIES_DEWPIDER          ,
    SPECIES_ARAQUANID         ,
    SPECIES_FOMANTIS          ,
    SPECIES_LURANTIS          ,
    SPECIES_MORELULL          ,
    SPECIES_SHIINOTIC         ,
    SPECIES_SALANDIT          ,
    SPECIES_SALAZZLE          ,
    SPECIES_STUFFUL           ,
    SPECIES_BEWEAR            ,
    SPECIES_BOUNSWEET         ,
    SPECIES_STEENEE           ,
    SPECIES_TSAREENA          ,
    SPECIES_COMFEY            ,
    SPECIES_ORANGURU          ,
    SPECIES_PASSIMIAN         ,
    SPECIES_WIMPOD            ,
    SPECIES_GOLISOPOD         ,
    SPECIES_SANDYGAST         ,
    SPECIES_PALOSSAND         ,
    SPECIES_PYUKUMUKU         ,
    //SPECIES_TYPE_NULL         ,
    //SPECIES_SILVALLY          ,
    SPECIES_MINIOR            ,
    SPECIES_KOMALA            ,
    SPECIES_TURTONATOR        ,
    SPECIES_TOGEDEMARU        ,
    SPECIES_MIMIKYU           ,
    SPECIES_BRUXISH           ,
    SPECIES_DRAMPA            ,
    SPECIES_DHELMISE          ,
    SPECIES_JANGMO_O          ,
    SPECIES_HAKAMO_O          ,
    SPECIES_KOMMO_O           ,
    //SPECIES_TAPU_KOKO         ,
    //SPECIES_TAPU_LELE         ,
    //SPECIES_TAPU_BULU         ,
    //SPECIES_TAPU_FINI         ,
    //SPECIES_COSMOG            ,
    //SPECIES_COSMOEM           ,
    //SPECIES_SOLGALEO          ,
    //SPECIES_LUNALA            ,
    //SPECIES_NIHILEGO          ,
    //SPECIES_BUZZWOLE          ,
    //SPECIES_PHEROMOSA         ,
    //SPECIES_XURKITREE         ,
    //SPECIES_CELESTEELA        ,
    //SPECIES_KARTANA           ,
    //SPECIES_GUZZLORD          ,
    //SPECIES_NECROZMA          ,
    //SPECIES_MAGEARNA          ,
    //SPECIES_MARSHADOW         ,
    //SPECIES_POIPOLE           ,
    //SPECIES_NAGANADEL         ,
    //SPECIES_STAKATAKA         ,
    //SPECIES_BLACEPHALON       ,
    //SPECIES_ZERAORA           ,
    //SPECIES_MELTAN            ,
    //SPECIES_MELMETAL          ,
    SPECIES_GROOKEY           ,
    SPECIES_THWACKEY          ,
    SPECIES_RILLABOOM         ,
    SPECIES_SCORBUNNY         ,
    SPECIES_RABOOT            ,
    SPECIES_CINDERACE         ,
    SPECIES_SOBBLE            ,
    SPECIES_DRIZZILE          ,
    SPECIES_INTELEON          ,
    SPECIES_SKWOVET           ,
    SPECIES_GREEDENT          ,
    SPECIES_ROOKIDEE          ,
    SPECIES_CORVISQUIRE       ,
    SPECIES_CORVIKNIGHT       ,
    SPECIES_BLIPBUG           ,
    SPECIES_DOTTLER           ,
    SPECIES_ORBEETLE          ,
    SPECIES_NICKIT            ,
    SPECIES_THIEVUL           ,
    SPECIES_GOSSIFLEUR        ,
    SPECIES_ELDEGOSS          ,
    SPECIES_WOOLOO            ,
    SPECIES_DUBWOOL           ,
    SPECIES_CHEWTLE           ,
    SPECIES_DREDNAW           ,
    SPECIES_YAMPER            ,
    SPECIES_BOLTUND           ,
    SPECIES_ROLYCOLY          ,
    SPECIES_CARKOL            ,
    SPECIES_COALOSSAL         ,
    SPECIES_APPLIN            ,
    SPECIES_FLAPPLE           ,
    SPECIES_APPLETUN          ,
    SPECIES_SILICOBRA         ,
    SPECIES_SANDACONDA        ,
    SPECIES_CRAMORANT         ,
    SPECIES_ARROKUDA          ,
    SPECIES_BARRASKEWDA       ,
    SPECIES_TOXEL             ,
    SPECIES_TOXTRICITY        ,
    SPECIES_SIZZLIPEDE        ,
    SPECIES_CENTISKORCH       ,
    SPECIES_CLOBBOPUS         ,
    SPECIES_GRAPPLOCT         ,
    SPECIES_SINISTEA          ,
    SPECIES_POLTEAGEIST       ,
    SPECIES_HATENNA           ,
    SPECIES_HATTREM           ,
    SPECIES_HATTERENE         ,
    SPECIES_IMPIDIMP          ,
    SPECIES_MORGREM           ,
    SPECIES_GRIMMSNARL        ,
    SPECIES_OBSTAGOON         ,
    SPECIES_PERRSERKER        ,
    SPECIES_CURSOLA           ,
    SPECIES_SIRFETCHD         ,
    SPECIES_MR_RIME           ,
    SPECIES_RUNERIGUS         ,
    SPECIES_MILCERY           ,
    SPECIES_ALCREMIE          ,
    SPECIES_FALINKS           ,
    SPECIES_PINCURCHIN        ,
    SPECIES_SNOM              ,
    SPECIES_FROSMOTH          ,
    SPECIES_STONJOURNER       ,
    SPECIES_EISCUE            ,
    SPECIES_INDEEDEE          ,
    SPECIES_MORPEKO           ,
    SPECIES_CUFANT            ,
    SPECIES_COPPERAJAH        ,
    SPECIES_DRACOZOLT         ,
    SPECIES_ARCTOZOLT         ,
    SPECIES_DRACOVISH         ,
    SPECIES_ARCTOVISH         ,
    SPECIES_DURALUDON         ,
    SPECIES_DREEPY            ,
    SPECIES_DRAKLOAK          ,
    SPECIES_DRAGAPULT         ,
    //SPECIES_ZACIAN            ,
    //SPECIES_ZAMAZENTA         ,
    //SPECIES_ETERNATUS         ,
    //SPECIES_KUBFU             ,
    //SPECIES_URSHIFU           ,
    //SPECIES_ZARUDE            ,
    //SPECIES_REGIELEKI         ,
    //SPECIES_REGIDRAGO         ,
    //SPECIES_GLASTRIER         ,
    //SPECIES_SPECTRIER         ,
    //SPECIES_CALYREX           ,
    //SPECIES_VENUSAUR_MEGA     ,
    //SPECIES_CHARIZARD_MEGA_X  ,
    //SPECIES_CHARIZARD_MEGA_Y  ,
    //SPECIES_BLASTOISE_MEGA    ,
    //SPECIES_BEEDRILL_MEGA     ,
    //SPECIES_PIDGEOT_MEGA      ,
    //SPECIES_ALAKAZAM_MEGA     ,
    //SPECIES_SLOWBRO_MEGA      ,
    //SPECIES_GENGAR_MEGA       ,
    //SPECIES_KANGASKHAN_MEGA   ,
    //SPECIES_PINSIR_MEGA       ,
    //SPECIES_GYARADOS_MEGA     ,
    //SPECIES_AERODACTYL_MEGA   ,
    //SPECIES_MEWTWO_MEGA_X     ,
    //SPECIES_MEWTWO_MEGA_Y     ,
    //SPECIES_AMPHAROS_MEGA     ,
    //SPECIES_STEELIX_MEGA      ,
    //SPECIES_SCIZOR_MEGA       ,
    //SPECIES_HERACROSS_MEGA    ,
    //SPECIES_HOUNDOOM_MEGA     ,
    //SPECIES_TYRANITAR_MEGA    ,
    //SPECIES_SCEPTILE_MEGA     ,
    //SPECIES_BLAZIKEN_MEGA     ,
    //SPECIES_SWAMPERT_MEGA     ,
    //SPECIES_GARDEVOIR_MEGA    ,
    //SPECIES_SABLEYE_MEGA      ,
    //SPECIES_MAWILE_MEGA       ,
    //SPECIES_AGGRON_MEGA       ,
    //SPECIES_MEDICHAM_MEGA     ,
    //SPECIES_MANECTRIC_MEGA    ,
    //SPECIES_SHARPEDO_MEGA     ,
    //SPECIES_CAMERUPT_MEGA     ,
    //SPECIES_ALTARIA_MEGA      ,
    //SPECIES_BANETTE_MEGA      ,
    //SPECIES_ABSOL_MEGA        ,
    //SPECIES_GLALIE_MEGA       ,
    //SPECIES_SALAMENCE_MEGA    ,
    //SPECIES_METAGROSS_MEGA    ,
    //SPECIES_LATIAS_MEGA       ,
    //SPECIES_LATIOS_MEGA       ,
    //SPECIES_LOPUNNY_MEGA      ,
    //SPECIES_GARCHOMP_MEGA     ,
    //SPECIES_LUCARIO_MEGA      ,
    //SPECIES_ABOMASNOW_MEGA    ,
    //SPECIES_GALLADE_MEGA      ,
    //SPECIES_AUDINO_MEGA       ,
    //SPECIES_DIANCIE_MEGA      ,
    //SPECIES_RAYQUAZA_MEGA     ,
    //SPECIES_KYOGRE_PRIMAL     ,
    //SPECIES_GROUDON_PRIMAL    ,
    SPECIES_RATTATA_ALOLAN    ,
    SPECIES_RATICATE_ALOLAN   ,
    SPECIES_RAICHU_ALOLAN     ,
    SPECIES_SANDSHREW_ALOLAN  ,
    SPECIES_SANDSLASH_ALOLAN  ,
    SPECIES_VULPIX_ALOLAN     ,
    SPECIES_NINETALES_ALOLAN  ,
    SPECIES_DIGLETT_ALOLAN    ,
    SPECIES_DUGTRIO_ALOLAN    ,
    SPECIES_MEOWTH_ALOLAN     ,
    SPECIES_PERSIAN_ALOLAN    ,
    SPECIES_GEODUDE_ALOLAN    ,
    SPECIES_GRAVELER_ALOLAN   ,
    SPECIES_GOLEM_ALOLAN      ,
    SPECIES_GRIMER_ALOLAN     ,
    SPECIES_MUK_ALOLAN        ,
    SPECIES_EXEGGUTOR_ALOLAN  ,
    SPECIES_MAROWAK_ALOLAN    ,
    SPECIES_MEOWTH_GALARIAN   ,
    SPECIES_PONYTA_GALARIAN   ,
    SPECIES_RAPIDASH_GALARIAN ,
    SPECIES_SLOWPOKE_GALARIAN ,
    SPECIES_SLOWBRO_GALARIAN  ,
    SPECIES_FARFETCHD_GALARIAN ,
    SPECIES_WEEZING_GALARIAN  ,
    SPECIES_MR_MIME_GALARIAN  ,
    //SPECIES_ARTICUNO_GALARIAN ,
    //SPECIES_ZAPDOS_GALARIAN   ,
    //SPECIES_MOLTRES_GALARIAN  ,
    SPECIES_SLOWKING_GALARIAN ,
    SPECIES_CORSOLA_GALARIAN  ,
    SPECIES_ZIGZAGOON_GALARIAN ,
    SPECIES_LINOONE_GALARIAN  ,
    SPECIES_DARUMAKA_GALARIAN ,
    SPECIES_DARMANITAN_GALARIAN ,
    SPECIES_YAMASK_GALARIAN   ,
    SPECIES_STUNFISK_GALARIAN ,
    SPECIES_PIKACHU_COSPLAY   ,
    SPECIES_PIKACHU_ROCK_STAR ,
    SPECIES_PIKACHU_BELLE     ,
    SPECIES_PIKACHU_POP_STAR  ,
    SPECIES_PIKACHU_PH_D      ,
    SPECIES_PIKACHU_LIBRE     ,
    SPECIES_PIKACHU_ORIGINAL_CAP ,
    SPECIES_PIKACHU_HOENN_CAP ,
    SPECIES_PIKACHU_SINNOH_CAP ,
    SPECIES_PIKACHU_UNOVA_CAP ,
    SPECIES_PIKACHU_KALOS_CAP ,
    SPECIES_PIKACHU_ALOLA_CAP ,
    SPECIES_PIKACHU_PARTNER_CAP ,
    SPECIES_PIKACHU_WORLD_CAP ,
    SPECIES_PICHU_SPIKY_EARED ,
    //SPECIES_UNOWN_B           ,
    //SPECIES_UNOWN_C           ,
    //SPECIES_UNOWN_D           ,
    //SPECIES_UNOWN_E           ,
    //SPECIES_UNOWN_F           ,
    //SPECIES_UNOWN_G           ,
    //SPECIES_UNOWN_H           ,
    //SPECIES_UNOWN_I           ,
    //SPECIES_UNOWN_J           ,
    //SPECIES_UNOWN_K           ,
    //SPECIES_UNOWN_L           ,
    //SPECIES_UNOWN_M           ,
    //SPECIES_UNOWN_N           ,
    //SPECIES_UNOWN_O           ,
    //SPECIES_UNOWN_P           ,
    //SPECIES_UNOWN_Q           ,
    //SPECIES_UNOWN_R           ,
    //SPECIES_UNOWN_S           ,
    //SPECIES_UNOWN_T           ,
    //SPECIES_UNOWN_U           ,
    //SPECIES_UNOWN_V           ,
    //SPECIES_UNOWN_W           ,
    //SPECIES_UNOWN_X           ,
    //SPECIES_UNOWN_Y           ,
    //SPECIES_UNOWN_Z           ,
    //SPECIES_UNOWN_EMARK       ,
    //SPECIES_UNOWN_QMARK       ,
    //SPECIES_CASTFORM_SUNNY    ,
    //SPECIES_CASTFORM_RAINY    ,
    //SPECIES_CASTFORM_SNOWY    ,
    //SPECIES_DEOXYS_ATTACK     ,
    //SPECIES_DEOXYS_DEFENSE    ,
    //SPECIES_DEOXYS_SPEED      ,
    SPECIES_BURMY_SANDY_CLOAK ,
    SPECIES_BURMY_TRASH_CLOAK ,
    SPECIES_WORMADAM_SANDY_CLOAK ,
    SPECIES_WORMADAM_TRASH_CLOAK ,
    SPECIES_CHERRIM_SUNSHINE  ,
    SPECIES_SHELLOS_EAST_SEA  ,
    SPECIES_GASTRODON_EAST_SEA ,
    SPECIES_ROTOM_HEAT        ,
    SPECIES_ROTOM_WASH        ,
    SPECIES_ROTOM_FROST       ,
    SPECIES_ROTOM_FAN         ,
    SPECIES_ROTOM_MOW         ,
    //SPECIES_GIRATINA_ORIGIN   ,
    //SPECIES_SHAYMIN_SKY       ,
    //SPECIES_ARCEUS_FIGHTING   ,
    //SPECIES_ARCEUS_FLYING     ,
    //SPECIES_ARCEUS_POISON     ,
    //SPECIES_ARCEUS_GROUND     ,
    //SPECIES_ARCEUS_ROCK       ,
    //SPECIES_ARCEUS_BUG        ,
    //SPECIES_ARCEUS_GHOST      ,
    //SPECIES_ARCEUS_STEEL      ,
    //SPECIES_ARCEUS_FIRE       ,
    //SPECIES_ARCEUS_WATER      ,
    //SPECIES_ARCEUS_GRASS      ,
    //SPECIES_ARCEUS_ELECTRIC   ,
    //SPECIES_ARCEUS_PSYCHIC    ,
    //SPECIES_ARCEUS_ICE        ,
    //SPECIES_ARCEUS_DRAGON     ,
    //SPECIES_ARCEUS_DARK       ,
    //SPECIES_ARCEUS_FAIRY      ,
    SPECIES_BASCULIN_BLUE_STRIPED ,
    SPECIES_DARMANITAN_ZEN_MODE ,
    SPECIES_DARMANITAN_ZEN_MODE_GALARIAN ,
    SPECIES_DEERLING_SUMMER   ,
    SPECIES_DEERLING_AUTUMN   ,
    SPECIES_DEERLING_WINTER   ,
    SPECIES_SAWSBUCK_SUMMER   ,
    SPECIES_SAWSBUCK_AUTUMN   ,
    SPECIES_SAWSBUCK_WINTER   ,
    //SPECIES_TORNADUS_THERIAN  ,
    //SPECIES_THUNDURUS_THERIAN ,
    //SPECIES_LANDORUS_THERIAN  ,
    //SPECIES_KYUREM_WHITE      ,
    //SPECIES_KYUREM_BLACK      ,
    //SPECIES_KELDEO_RESOLUTE   ,
    //SPECIES_MELOETTA_PIROUETTE ,
    //SPECIES_GENESECT_DOUSE_DRIVE ,
    //SPECIES_GENESECT_SHOCK_DRIVE ,
    //SPECIES_GENESECT_BURN_DRIVE ,
    //SPECIES_GENESECT_CHILL_DRIVE ,
    SPECIES_GRENINJA_BATTLE_BOND ,
    SPECIES_GRENINJA_ASH      ,
    SPECIES_VIVILLON_POLAR    ,
    SPECIES_VIVILLON_TUNDRA   ,
    SPECIES_VIVILLON_CONTINENTAL ,
    SPECIES_VIVILLON_GARDEN   ,
    SPECIES_VIVILLON_ELEGANT  ,
    SPECIES_VIVILLON_MEADOW   ,
    SPECIES_VIVILLON_MODERN   ,
    SPECIES_VIVILLON_MARINE   ,
    SPECIES_VIVILLON_ARCHIPELAGO ,
    SPECIES_VIVILLON_HIGH_PLAINS ,
    SPECIES_VIVILLON_SANDSTORM ,
    SPECIES_VIVILLON_RIVER    ,
    SPECIES_VIVILLON_MONSOON  ,
    SPECIES_VIVILLON_SAVANNA  ,
    SPECIES_VIVILLON_SUN      ,
    SPECIES_VIVILLON_OCEAN    ,
    SPECIES_VIVILLON_JUNGLE   ,
    SPECIES_VIVILLON_FANCY    ,
    SPECIES_VIVILLON_POKE_BALL ,
    SPECIES_FLABEBE_YELLOW_FLOWER ,
    SPECIES_FLABEBE_ORANGE_FLOWER ,
    SPECIES_FLABEBE_BLUE_FLOWER ,
    SPECIES_FLABEBE_WHITE_FLOWER ,
    SPECIES_FLOETTE_YELLOW_FLOWER ,
    SPECIES_FLOETTE_ORANGE_FLOWER ,
    SPECIES_FLOETTE_BLUE_FLOWER ,
    SPECIES_FLOETTE_WHITE_FLOWER ,
    SPECIES_FLOETTE_ETERNAL_FLOWER ,
    SPECIES_FLORGES_YELLOW_FLOWER ,
    SPECIES_FLORGES_ORANGE_FLOWER ,
    SPECIES_FLORGES_BLUE_FLOWER ,
    SPECIES_FLORGES_WHITE_FLOWER ,
    SPECIES_FURFROU_HEART_TRIM ,
    SPECIES_FURFROU_STAR_TRIM ,
    SPECIES_FURFROU_DIAMOND_TRIM ,
    SPECIES_FURFROU_DEBUTANTE_TRIM ,
    SPECIES_FURFROU_MATRON_TRIM ,
    SPECIES_FURFROU_DANDY_TRIM ,
    SPECIES_FURFROU_LA_REINE_TRIM ,
    SPECIES_FURFROU_KABUKI_TRIM ,
    SPECIES_FURFROU_PHARAOH_TRIM ,
    SPECIES_MEOWSTIC_FEMALE   ,
    SPECIES_AEGISLASH_BLADE   ,
    SPECIES_PUMPKABOO_SMALL   ,
    SPECIES_PUMPKABOO_LARGE   ,
    SPECIES_PUMPKABOO_SUPER   ,
    SPECIES_GOURGEIST_SMALL   ,
    SPECIES_GOURGEIST_LARGE   ,
    SPECIES_GOURGEIST_SUPER   ,
    //SPECIES_XERNEAS_ACTIVE    ,
    //SPECIES_ZYGARDE_10        ,
    //SPECIES_ZYGARDE_10_POWER_CONSTRUCT ,
    //SPECIES_ZYGARDE_50_POWER_CONSTRUCT ,
    //SPECIES_ZYGARDE_COMPLETE  ,
    //SPECIES_HOOPA_UNBOUND     ,
    SPECIES_ORICORIO_POM_POM  ,
    SPECIES_ORICORIO_PAU      ,
    SPECIES_ORICORIO_SENSU    ,
    SPECIES_ROCKRUFF_OWN_TEMPO ,
    SPECIES_LYCANROC_MIDNIGHT ,
    SPECIES_LYCANROC_DUSK     ,
    SPECIES_WISHIWASHI_SCHOOL ,
    //SPECIES_SILVALLY_FIGHTING ,
    //SPECIES_SILVALLY_FLYING   ,
    //SPECIES_SILVALLY_POISON   ,
    //SPECIES_SILVALLY_GROUND   ,
    //SPECIES_SILVALLY_ROCK     ,
    //SPECIES_SILVALLY_BUG      ,
    //SPECIES_SILVALLY_GHOST    ,
    //SPECIES_SILVALLY_STEEL    ,
    //SPECIES_SILVALLY_FIRE     ,
    //SPECIES_SILVALLY_WATER    ,
    //SPECIES_SILVALLY_GRASS    ,
    //SPECIES_SILVALLY_ELECTRIC ,
    //SPECIES_SILVALLY_PSYCHIC  ,
    //SPECIES_SILVALLY_ICE      ,
    //SPECIES_SILVALLY_DRAGON   ,
    //SPECIES_SILVALLY_DARK     ,
    //SPECIES_SILVALLY_FAIRY    ,
    SPECIES_MINIOR_METEOR_ORANGE ,
    SPECIES_MINIOR_METEOR_YELLOW ,
    SPECIES_MINIOR_METEOR_GREEN ,
    SPECIES_MINIOR_METEOR_BLUE ,
    SPECIES_MINIOR_METEOR_INDIGO ,
    SPECIES_MINIOR_METEOR_VIOLET ,
    SPECIES_MINIOR_CORE_RED   ,
    SPECIES_MINIOR_CORE_ORANGE ,
    SPECIES_MINIOR_CORE_YELLOW ,
    SPECIES_MINIOR_CORE_GREEN ,
    SPECIES_MINIOR_CORE_BLUE  ,
    SPECIES_MINIOR_CORE_INDIGO ,
    SPECIES_MINIOR_CORE_VIOLET ,
    SPECIES_MIMIKYU_BUSTED    ,
    //SPECIES_NECROZMA_DUSK_MANE ,
    //SPECIES_NECROZMA_DAWN_WINGS ,
    //SPECIES_NECROZMA_ULTRA    ,
    //SPECIES_MAGEARNA_ORIGINAL_COLOR ,
    SPECIES_CRAMORANT_GULPING ,
    SPECIES_CRAMORANT_GORGING ,
    SPECIES_TOXTRICITY_LOW_KEY ,
    SPECIES_SINISTEA_ANTIQUE  ,
    SPECIES_POLTEAGEIST_ANTIQUE ,
    SPECIES_ALCREMIE_RUBY_CREAM ,
    SPECIES_ALCREMIE_MATCHA_CREAM ,
    SPECIES_ALCREMIE_MINT_CREAM ,
    SPECIES_ALCREMIE_LEMON_CREAM ,
    SPECIES_ALCREMIE_SALTED_CREAM ,
    SPECIES_ALCREMIE_RUBY_SWIRL ,
    SPECIES_ALCREMIE_CARAMEL_SWIRL ,
    SPECIES_ALCREMIE_RAINBOW_SWIRL ,
    SPECIES_EISCUE_NOICE_FACE ,
    SPECIES_INDEEDEE_FEMALE   ,
    SPECIES_MORPEKO_HANGRY    ,
    //SPECIES_ZACIAN_CROWNED_SWORD ,
    //SPECIES_ZAMAZENTA_CROWNED_SHIELD ,
    //SPECIES_ETERNATUS_ETERNAMAX ,
    //SPECIES_URSHIFU_RAPID_STRIKE_STYLE ,
    //SPECIES_ZARUDE_DADA       ,
    //SPECIES_CALYREX_ICE_RIDER ,
    //SPECIES_CALYREX_SHADOW_RIDER ,
    // SPECIES_EGG       ,
};

#define RANDOM_SPECIES_EVO_0_COUNT ARRAY_COUNT(sRandomSpeciesEvo0)
static const u16 sRandomSpeciesEvo0[] =
{
    SPECIES_BULBASAUR       ,    //= EVO_TYPE_0,
    SPECIES_CHARMANDER      ,    //= EVO_TYPE_0,
    SPECIES_SQUIRTLE        ,    //= EVO_TYPE_0,
    SPECIES_CATERPIE        ,    //= EVO_TYPE_0,
    SPECIES_WEEDLE          ,    //= EVO_TYPE_0,
    SPECIES_PIDGEY          ,    //= EVO_TYPE_0,
    SPECIES_RATTATA         ,    //= EVO_TYPE_0,
    SPECIES_SPEAROW         ,    //= EVO_TYPE_0,
    SPECIES_EKANS           ,    //= EVO_TYPE_0,
    SPECIES_SANDSHREW       ,    //= EVO_TYPE_0,
    SPECIES_NIDORAN_F       ,    //= EVO_TYPE_0,
    SPECIES_NIDORAN_M       ,    //= EVO_TYPE_0,
    SPECIES_VULPIX          ,    //= EVO_TYPE_0,
    SPECIES_ZUBAT           ,    //= EVO_TYPE_0,
    SPECIES_ODDISH          ,    //= EVO_TYPE_0,
    SPECIES_PARAS           ,    //= EVO_TYPE_0,
    SPECIES_VENONAT         ,    //= EVO_TYPE_0,
    SPECIES_DIGLETT         ,    //= EVO_TYPE_0,
    SPECIES_MEOWTH          ,    //= EVO_TYPE_0,
    SPECIES_PSYDUCK         ,    //= EVO_TYPE_0,
    SPECIES_MANKEY          ,    //= EVO_TYPE_0,
    SPECIES_GROWLITHE       ,    //= EVO_TYPE_0,
    SPECIES_POLIWAG         ,    //= EVO_TYPE_0,
    SPECIES_ABRA            ,    //= EVO_TYPE_0,
    SPECIES_MACHOP          ,    //= EVO_TYPE_0,
    SPECIES_BELLSPROUT      ,    //= EVO_TYPE_0,
    SPECIES_TENTACOOL       ,    //= EVO_TYPE_0,
    SPECIES_GEODUDE         ,    //= EVO_TYPE_0,
    SPECIES_PONYTA          ,    //= EVO_TYPE_0,
    SPECIES_SLOWPOKE        ,    //= EVO_TYPE_0,
    SPECIES_MAGNEMITE       ,    //= EVO_TYPE_0,
    SPECIES_FARFETCHD       ,    //= EVO_TYPE_0,
    SPECIES_DODUO           ,    //= EVO_TYPE_0,
    SPECIES_SEEL            ,    //= EVO_TYPE_0,
    SPECIES_GRIMER          ,    //= EVO_TYPE_0,
    SPECIES_SHELLDER        ,    //= EVO_TYPE_0,
    SPECIES_GASTLY          ,    //= EVO_TYPE_0,
    SPECIES_ONIX            ,    //= EVO_TYPE_0,
    SPECIES_DROWZEE         ,    //= EVO_TYPE_0,
    SPECIES_KRABBY          ,    //= EVO_TYPE_0,
    SPECIES_VOLTORB         ,    //= EVO_TYPE_0,
    SPECIES_EXEGGCUTE       ,    //= EVO_TYPE_0,
    SPECIES_CUBONE          ,    //= EVO_TYPE_0,
    SPECIES_LICKITUNG       ,    //= EVO_TYPE_0,
    SPECIES_KOFFING         ,    //= EVO_TYPE_0,
    SPECIES_RHYHORN         ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_CHANSEY         ,    //= EVO_TYPE_0,
    #endif
    SPECIES_TANGELA         ,    //= EVO_TYPE_0,
    SPECIES_KANGASKHAN      ,    //= EVO_TYPE_0,
    SPECIES_HORSEA          ,    //= EVO_TYPE_0,
    SPECIES_GOLDEEN         ,    //= EVO_TYPE_0,
    SPECIES_STARYU          ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_MR_MIME         ,    //= EVO_TYPE_0,
    #endif
    SPECIES_SCYTHER         ,    //= EVO_TYPE_0,
    SPECIES_PINSIR          ,    //= EVO_TYPE_0,
    SPECIES_TAUROS          ,    //= EVO_TYPE_0,
    SPECIES_MAGIKARP        ,    //= EVO_TYPE_0,
    SPECIES_LAPRAS          ,    //= EVO_TYPE_0,
    SPECIES_DITTO           ,    //= EVO_TYPE_0,
    SPECIES_EEVEE           ,    //= EVO_TYPE_0,
    SPECIES_PORYGON         ,    //= EVO_TYPE_0,
    SPECIES_OMANYTE         ,    //= EVO_TYPE_0,
    SPECIES_KABUTO          ,    //= EVO_TYPE_0,
    SPECIES_AERODACTYL      ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_SNORLAX         ,    //= EVO_TYPE_0,
    #endif
    SPECIES_DRATINI         ,    //= EVO_TYPE_0,
    SPECIES_CHIKORITA       ,    //= EVO_TYPE_0,
    SPECIES_CYNDAQUIL       ,    //= EVO_TYPE_0,
    SPECIES_TOTODILE        ,    //= EVO_TYPE_0,
    SPECIES_SENTRET         ,    //= EVO_TYPE_0,
    SPECIES_HOOTHOOT        ,    //= EVO_TYPE_0,
    SPECIES_LEDYBA          ,    //= EVO_TYPE_0,
    SPECIES_SPINARAK        ,    //= EVO_TYPE_0,
    SPECIES_CHINCHOU        ,    //= EVO_TYPE_0,
    SPECIES_PICHU           ,    //= EVO_TYPE_0,
    SPECIES_CLEFFA          ,    //= EVO_TYPE_0,
    SPECIES_IGGLYBUFF       ,    //= EVO_TYPE_0,
    SPECIES_TOGEPI          ,    //= EVO_TYPE_0,
    SPECIES_NATU            ,    //= EVO_TYPE_0,
    SPECIES_MAREEP          ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_SUDOWOODO       ,    //= EVO_TYPE_0,
    #endif
    SPECIES_HOPPIP          ,    //= EVO_TYPE_0,
    SPECIES_AIPOM           ,    //= EVO_TYPE_0,
    SPECIES_SUNKERN         ,    //= EVO_TYPE_0,
    SPECIES_YANMA           ,    //= EVO_TYPE_0,
    SPECIES_WOOPER          ,    //= EVO_TYPE_0,
    SPECIES_MURKROW         ,    //= EVO_TYPE_0,
    SPECIES_MISDREAVUS      ,    //= EVO_TYPE_0,
    SPECIES_UNOWN           ,    //= EVO_TYPE_0,
    SPECIES_GIRAFARIG       ,    //= EVO_TYPE_0,
    SPECIES_PINECO          ,    //= EVO_TYPE_0,
    SPECIES_DUNSPARCE       ,    //= EVO_TYPE_0,
    SPECIES_GLIGAR          ,    //= EVO_TYPE_0,
    SPECIES_SNUBBULL        ,    //= EVO_TYPE_0,
    SPECIES_QWILFISH        ,    //= EVO_TYPE_0,
    SPECIES_SHUCKLE         ,    //= EVO_TYPE_0,
    SPECIES_HERACROSS       ,    //= EVO_TYPE_0,
    SPECIES_SNEASEL         ,    //= EVO_TYPE_0,
    SPECIES_TEDDIURSA       ,    //= EVO_TYPE_0,
    SPECIES_SLUGMA          ,    //= EVO_TYPE_0,
    SPECIES_SWINUB          ,    //= EVO_TYPE_0,
    SPECIES_CORSOLA         ,    //= EVO_TYPE_0,
    SPECIES_REMORAID        ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_MANTINE         ,    //= EVO_TYPE_0,
    #endif
    SPECIES_DELIBIRD        ,    //= EVO_TYPE_0,
    SPECIES_SKARMORY        ,    //= EVO_TYPE_0,
    SPECIES_HOUNDOUR        ,    //= EVO_TYPE_0,
    SPECIES_PHANPY          ,    //= EVO_TYPE_0,
    SPECIES_STANTLER        ,    //= EVO_TYPE_0,
    SPECIES_SMEARGLE        ,    //= EVO_TYPE_0,
    SPECIES_TYROGUE         ,    //= EVO_TYPE_0,
    SPECIES_SMOOCHUM        ,    //= EVO_TYPE_0,
    SPECIES_ELEKID          ,    //= EVO_TYPE_0,
    SPECIES_MAGBY           ,    //= EVO_TYPE_0,
    SPECIES_MILTANK         ,    //= EVO_TYPE_0,
    SPECIES_LARVITAR        ,    //= EVO_TYPE_0,
    SPECIES_TREECKO         ,    //= EVO_TYPE_0,
    SPECIES_TORCHIC         ,    //= EVO_TYPE_0,
    SPECIES_MUDKIP          ,    //= EVO_TYPE_0,
    SPECIES_POOCHYENA       ,    //= EVO_TYPE_0,
    SPECIES_ZIGZAGOON       ,    //= EVO_TYPE_0,
    SPECIES_WURMPLE         ,    //= EVO_TYPE_0,
    SPECIES_LOTAD           ,    //= EVO_TYPE_0,
    SPECIES_SEEDOT          ,    //= EVO_TYPE_0,
    SPECIES_NINCADA         ,    //= EVO_TYPE_0,
    SPECIES_TAILLOW         ,    //= EVO_TYPE_0,
    SPECIES_SHROOMISH       ,    //= EVO_TYPE_0,
    SPECIES_SPINDA          ,    //= EVO_TYPE_0,
    SPECIES_WINGULL         ,    //= EVO_TYPE_0,
    SPECIES_SURSKIT         ,    //= EVO_TYPE_0,
    SPECIES_WAILMER         ,    //= EVO_TYPE_0,
    SPECIES_SKITTY          ,    //= EVO_TYPE_0,
    SPECIES_KECLEON         ,    //= EVO_TYPE_0,
    SPECIES_BALTOY          ,    //= EVO_TYPE_0,
    SPECIES_NOSEPASS        ,    //= EVO_TYPE_0,
    SPECIES_TORKOAL         ,    //= EVO_TYPE_0,
    SPECIES_SABLEYE         ,    //= EVO_TYPE_0,
    SPECIES_BARBOACH        ,    //= EVO_TYPE_0,
    SPECIES_LUVDISC         ,    //= EVO_TYPE_0,
    SPECIES_CORPHISH        ,    //= EVO_TYPE_0,
    SPECIES_FEEBAS          ,    //= EVO_TYPE_0,
    SPECIES_CARVANHA        ,    //= EVO_TYPE_0,
    SPECIES_TRAPINCH        ,    //= EVO_TYPE_0,
    SPECIES_MAKUHITA        ,    //= EVO_TYPE_0,
    SPECIES_ELECTRIKE       ,    //= EVO_TYPE_0,
    SPECIES_NUMEL           ,    //= EVO_TYPE_0,
    SPECIES_SPHEAL          ,    //= EVO_TYPE_0,
    SPECIES_CACNEA          ,    //= EVO_TYPE_0,
    SPECIES_SNORUNT         ,    //= EVO_TYPE_0,
    SPECIES_LUNATONE        ,    //= EVO_TYPE_0,
    SPECIES_SOLROCK         ,    //= EVO_TYPE_0,
    SPECIES_AZURILL         ,    //= EVO_TYPE_0,
    SPECIES_SPOINK          ,    //= EVO_TYPE_0,
    SPECIES_PLUSLE          ,    //= EVO_TYPE_0,
    SPECIES_MINUN           ,    //= EVO_TYPE_0,
    SPECIES_MAWILE          ,    //= EVO_TYPE_0,
    SPECIES_MEDITITE        ,    //= EVO_TYPE_0,
    SPECIES_SWABLU          ,    //= EVO_TYPE_0,
    SPECIES_WYNAUT          ,    //= EVO_TYPE_0,
    SPECIES_DUSKULL         ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_ROSELIA         ,    //= EVO_TYPE_0,
    #endif
    SPECIES_SLAKOTH         ,    //= EVO_TYPE_0,
    SPECIES_GULPIN          ,    //= EVO_TYPE_0,
    SPECIES_TROPIUS         ,    //= EVO_TYPE_0,
    SPECIES_WHISMUR         ,    //= EVO_TYPE_0,
    SPECIES_CLAMPERL        ,    //= EVO_TYPE_0,
    SPECIES_ABSOL           ,    //= EVO_TYPE_0,
    SPECIES_SHUPPET         ,    //= EVO_TYPE_0,
    SPECIES_SEVIPER         ,    //= EVO_TYPE_0,
    SPECIES_ZANGOOSE        ,    //= EVO_TYPE_0,
    SPECIES_RELICANTH       ,    //= EVO_TYPE_0,
    SPECIES_ARON            ,    //= EVO_TYPE_0,
    SPECIES_LILEEP          ,    //= EVO_TYPE_0,
    SPECIES_ANORITH         ,    //= EVO_TYPE_0,
    SPECIES_RALTS           ,    //= EVO_TYPE_0,
    SPECIES_BAGON           ,    //= EVO_TYPE_0,
    SPECIES_BELDUM          ,    //= EVO_TYPE_0,
    #ifndef POKEMON_EXPANSION
    SPECIES_CHIMECHO        ,    //= EVO_TYPE_0,
    #else
    SPECIES_TURTWIG           , //= EVO_TYPE_0,
    SPECIES_CHIMCHAR          , //= EVO_TYPE_0,
    SPECIES_PIPLUP            , //= EVO_TYPE_0,
    SPECIES_STARLY            , //= EVO_TYPE_0,
    SPECIES_BIDOOF            , //= EVO_TYPE_0,
    SPECIES_KRICKETOT         , //= EVO_TYPE_0,
    SPECIES_SHINX             , //= EVO_TYPE_0,
    SPECIES_BUDEW             , //= EVO_TYPE_0,
    SPECIES_CRANIDOS          , //= EVO_TYPE_0,
    SPECIES_SHIELDON          , //= EVO_TYPE_0,
    SPECIES_BURMY             , //= EVO_TYPE_0,
    SPECIES_COMBEE            , //= EVO_TYPE_0,
    SPECIES_PACHIRISU         , //= EVO_TYPE_0,
    SPECIES_BUIZEL            , //= EVO_TYPE_0,
    SPECIES_CHERUBI           , //= EVO_TYPE_0,
    SPECIES_SHELLOS           , //= EVO_TYPE_0,
    SPECIES_DRIFLOON          , //= EVO_TYPE_0,
    SPECIES_BUNEARY           , //= EVO_TYPE_0,
    SPECIES_GLAMEOW           , //= EVO_TYPE_0,
    SPECIES_CHINGLING         , //= EVO_TYPE_0,
    SPECIES_STUNKY            , //= EVO_TYPE_0,
    SPECIES_BRONZOR           , //= EVO_TYPE_0,
    SPECIES_BONSLY            , //= EVO_TYPE_0,
    SPECIES_MIME_JR           , //= EVO_TYPE_0,
    SPECIES_HAPPINY           , //= EVO_TYPE_0,
    SPECIES_CHATOT            , //= EVO_TYPE_0,
    SPECIES_SPIRITOMB         , //= EVO_TYPE_0,
    SPECIES_GIBLE             , //= EVO_TYPE_0,
    SPECIES_MUNCHLAX          , //= EVO_TYPE_0,
    SPECIES_RIOLU             , //= EVO_TYPE_0,
    SPECIES_HIPPOPOTAS        , //= EVO_TYPE_0,
    SPECIES_SKORUPI           , //= EVO_TYPE_0,
    SPECIES_CROAGUNK          , //= EVO_TYPE_0,
    SPECIES_CARNIVINE         , //= EVO_TYPE_0,
    SPECIES_FINNEON           , //= EVO_TYPE_0,
    SPECIES_MANTYKE           , //= EVO_TYPE_0,
    SPECIES_SNOVER            , //= EVO_TYPE_0,
    SPECIES_ROTOM             , //= EVO_TYPE_0,
    SPECIES_SNIVY             , //= EVO_TYPE_0,
    SPECIES_TEPIG             , //= EVO_TYPE_0,
    SPECIES_OSHAWOTT          , //= EVO_TYPE_0,
    SPECIES_PATRAT            , //= EVO_TYPE_0,
    SPECIES_LILLIPUP          , //= EVO_TYPE_0,
    SPECIES_PURRLOIN          , //= EVO_TYPE_0,
    SPECIES_PANSAGE           , //= EVO_TYPE_0,
    SPECIES_PANSEAR           , //= EVO_TYPE_0,
    SPECIES_PANPOUR           , //= EVO_TYPE_0,
    SPECIES_MUNNA             , //= EVO_TYPE_0,
    SPECIES_PIDOVE            , //= EVO_TYPE_0,
    SPECIES_BLITZLE           , //= EVO_TYPE_0,
    SPECIES_ROGGENROLA        , //= EVO_TYPE_0,
    SPECIES_WOOBAT            , //= EVO_TYPE_0,
    SPECIES_DRILBUR           , //= EVO_TYPE_0,
    SPECIES_AUDINO            , //= EVO_TYPE_0,
    SPECIES_TIMBURR           , //= EVO_TYPE_0,
    SPECIES_TYMPOLE           , //= EVO_TYPE_0,
    SPECIES_THROH             , //= EVO_TYPE_0,
    SPECIES_SAWK              , //= EVO_TYPE_0,
    SPECIES_SEWADDLE          , //= EVO_TYPE_0,
    SPECIES_VENIPEDE          , //= EVO_TYPE_0,
    SPECIES_COTTONEE          , //= EVO_TYPE_0,
    SPECIES_PETILIL           , //= EVO_TYPE_0,
    SPECIES_BASCULIN          , //= EVO_TYPE_0,
    SPECIES_SANDILE           , //= EVO_TYPE_0,
    SPECIES_DARUMAKA          , //= EVO_TYPE_0,
    SPECIES_MARACTUS          , //= EVO_TYPE_0,
    SPECIES_DWEBBLE           , //= EVO_TYPE_0,
    SPECIES_SCRAGGY           , //= EVO_TYPE_0,
    SPECIES_SIGILYPH          , //= EVO_TYPE_0,
    SPECIES_YAMASK            , //= EVO_TYPE_0,
    SPECIES_TIRTOUGA          , //= EVO_TYPE_0,
    SPECIES_ARCHEN            , //= EVO_TYPE_0,
    SPECIES_TRUBBISH          , //= EVO_TYPE_0,
    SPECIES_ZORUA             , //= EVO_TYPE_0,
    SPECIES_MINCCINO          , //= EVO_TYPE_0,
    SPECIES_GOTHITA           , //= EVO_TYPE_0,
    SPECIES_SOLOSIS           , //= EVO_TYPE_0,
    SPECIES_DUCKLETT          , //= EVO_TYPE_0,
    SPECIES_VANILLITE         , //= EVO_TYPE_0,
    SPECIES_DEERLING          , //= EVO_TYPE_0,
    SPECIES_EMOLGA            , //= EVO_TYPE_0,
    SPECIES_KARRABLAST        , //= EVO_TYPE_0,
    SPECIES_FOONGUS           , //= EVO_TYPE_0,
    SPECIES_FRILLISH          , //= EVO_TYPE_0,
    SPECIES_ALOMOMOLA         , //= EVO_TYPE_0,
    SPECIES_JOLTIK            , //= EVO_TYPE_0,
    SPECIES_FERROSEED         , //= EVO_TYPE_0,
    SPECIES_KLINK             , //= EVO_TYPE_0,
    SPECIES_TYNAMO            , //= EVO_TYPE_0,
    SPECIES_ELGYEM            , //= EVO_TYPE_0,
    SPECIES_LITWICK           , //= EVO_TYPE_0,
    SPECIES_AXEW              , //= EVO_TYPE_0,
    SPECIES_CUBCHOO           , //= EVO_TYPE_0,
    SPECIES_CRYOGONAL         , //= EVO_TYPE_0,
    SPECIES_SHELMET           , //= EVO_TYPE_0,
    SPECIES_STUNFISK          , //= EVO_TYPE_0,
    SPECIES_MIENFOO           , //= EVO_TYPE_0,
    SPECIES_DRUDDIGON         , //= EVO_TYPE_0,
    SPECIES_GOLETT            , //= EVO_TYPE_0,
    SPECIES_PAWNIARD          , //= EVO_TYPE_0,
    SPECIES_BOUFFALANT        , //= EVO_TYPE_0,
    SPECIES_RUFFLET           , //= EVO_TYPE_0,
    SPECIES_VULLABY           , //= EVO_TYPE_0,
    SPECIES_HEATMOR           , //= EVO_TYPE_0,
    SPECIES_DURANT            , //= EVO_TYPE_0,
    SPECIES_DEINO             , //= EVO_TYPE_0,
    SPECIES_LARVESTA          , //= EVO_TYPE_0,
    SPECIES_CHESPIN           , //= EVO_TYPE_0,
    SPECIES_FENNEKIN          , //= EVO_TYPE_0,
    SPECIES_FROAKIE           , //= EVO_TYPE_0,
    SPECIES_BUNNELBY          , //= EVO_TYPE_0,
    SPECIES_FLETCHLING        , //= EVO_TYPE_0,
    SPECIES_SCATTERBUG        , //= EVO_TYPE_0,
    SPECIES_LITLEO            , //= EVO_TYPE_0,
    SPECIES_FLABEBE           , //= EVO_TYPE_0,
    SPECIES_SKIDDO            , //= EVO_TYPE_0,
    SPECIES_PANCHAM           , //= EVO_TYPE_0,
    SPECIES_FURFROU           , //= EVO_TYPE_0,
    SPECIES_ESPURR            , //= EVO_TYPE_0,
    SPECIES_HONEDGE           , //= EVO_TYPE_0,
    SPECIES_SPRITZEE          , //= EVO_TYPE_0,
    SPECIES_SWIRLIX           , //= EVO_TYPE_0,
    SPECIES_INKAY             , //= EVO_TYPE_0,
    SPECIES_BINACLE           , //= EVO_TYPE_0,
    SPECIES_SKRELP            , //= EVO_TYPE_0,
    SPECIES_CLAUNCHER         , //= EVO_TYPE_0,
    SPECIES_HELIOPTILE        , //= EVO_TYPE_0,
    SPECIES_TYRUNT            , //= EVO_TYPE_0,
    SPECIES_AMAURA            , //= EVO_TYPE_0,
    SPECIES_HAWLUCHA          , //= EVO_TYPE_0,
    SPECIES_DEDENNE           , //= EVO_TYPE_0,
    SPECIES_CARBINK           , //= EVO_TYPE_0,
    SPECIES_GOOMY             , //= EVO_TYPE_0,
    SPECIES_KLEFKI            , //= EVO_TYPE_0,
    SPECIES_PHANTUMP          , //= EVO_TYPE_0,
    SPECIES_PUMPKABOO         , //= EVO_TYPE_0,
    SPECIES_BERGMITE          , //= EVO_TYPE_0,
    SPECIES_NOIBAT            , //= EVO_TYPE_0,
    SPECIES_ROWLET            , //= EVO_TYPE_0,
    SPECIES_LITTEN            , //= EVO_TYPE_0,
    SPECIES_POPPLIO           , //= EVO_TYPE_0,
    SPECIES_PIKIPEK           , //= EVO_TYPE_0,
    SPECIES_YUNGOOS           , //= EVO_TYPE_0,
    SPECIES_GRUBBIN           , //= EVO_TYPE_0,
    SPECIES_CRABRAWLER        , //= EVO_TYPE_0,
    SPECIES_ORICORIO          , //= EVO_TYPE_0,
    SPECIES_CUTIEFLY          , //= EVO_TYPE_0,
    SPECIES_ROCKRUFF          , //= EVO_TYPE_0,
    SPECIES_WISHIWASHI        , //= EVO_TYPE_0,
    SPECIES_MAREANIE          , //= EVO_TYPE_0,
    SPECIES_MUDBRAY           , //= EVO_TYPE_0,
    SPECIES_DEWPIDER          , //= EVO_TYPE_0,
    SPECIES_FOMANTIS          , //= EVO_TYPE_0,
    SPECIES_MORELULL          , //= EVO_TYPE_0,
    SPECIES_SALANDIT          , //= EVO_TYPE_0,
    SPECIES_STUFFUL           , //= EVO_TYPE_0,
    SPECIES_BOUNSWEET         , //= EVO_TYPE_0,
    SPECIES_COMFEY            , //= EVO_TYPE_0,
    SPECIES_ORANGURU          , //= EVO_TYPE_0,
    SPECIES_PASSIMIAN         , //= EVO_TYPE_0,
    SPECIES_WIMPOD            , //= EVO_TYPE_0,
    SPECIES_SANDYGAST         , //= EVO_TYPE_0,
    SPECIES_PYUKUMUKU         , //= EVO_TYPE_0,
    SPECIES_MINIOR            , //= EVO_TYPE_0,
    SPECIES_KOMALA            , //= EVO_TYPE_0,
    SPECIES_TURTONATOR        , //= EVO_TYPE_0,
    SPECIES_TOGEDEMARU        , //= EVO_TYPE_0,
    SPECIES_MIMIKYU           , //= EVO_TYPE_0,
    SPECIES_BRUXISH           , //= EVO_TYPE_0,
    SPECIES_DRAMPA            , //= EVO_TYPE_0,
    SPECIES_DHELMISE          , //= EVO_TYPE_0,
    SPECIES_JANGMO_O          , //= EVO_TYPE_0,
    SPECIES_GROOKEY           , //= EVO_TYPE_0,
    SPECIES_SCORBUNNY         , //= EVO_TYPE_0,
    SPECIES_SOBBLE            , //= EVO_TYPE_0,
    SPECIES_SKWOVET           , //= EVO_TYPE_0,
    SPECIES_ROOKIDEE          , //= EVO_TYPE_0,
    SPECIES_BLIPBUG           , //= EVO_TYPE_0,
    SPECIES_NICKIT            , //= EVO_TYPE_0,
    SPECIES_GOSSIFLEUR        , //= EVO_TYPE_0,
    SPECIES_WOOLOO            , //= EVO_TYPE_0,
    SPECIES_CHEWTLE           , //= EVO_TYPE_0,
    SPECIES_YAMPER            , //= EVO_TYPE_0,
    SPECIES_ROLYCOLY          , //= EVO_TYPE_0,
    SPECIES_APPLIN            , //= EVO_TYPE_0,
    SPECIES_SILICOBRA         , //= EVO_TYPE_0,
    SPECIES_CRAMORANT         , //= EVO_TYPE_0,
    SPECIES_ARROKUDA          , //= EVO_TYPE_0,
    SPECIES_TOXEL             , //= EVO_TYPE_0,
    SPECIES_SIZZLIPEDE        , //= EVO_TYPE_0,
    SPECIES_CLOBBOPUS         , //= EVO_TYPE_0,
    SPECIES_SINISTEA          , //= EVO_TYPE_0,
    SPECIES_HATENNA           , //= EVO_TYPE_0,
    SPECIES_IMPIDIMP          , //= EVO_TYPE_0,
    SPECIES_MILCERY           , //= EVO_TYPE_0,
    SPECIES_FALINKS           , //= EVO_TYPE_0,
    SPECIES_PINCURCHIN        , //= EVO_TYPE_0,
    SPECIES_SNOM              , //= EVO_TYPE_0,
    SPECIES_STONJOURNER       , //= EVO_TYPE_0,
    SPECIES_EISCUE            , //= EVO_TYPE_0,
    SPECIES_INDEEDEE          , //= EVO_TYPE_0,
    SPECIES_MORPEKO           , //= EVO_TYPE_0,
    SPECIES_CUFANT            , //= EVO_TYPE_0,
    SPECIES_DRACOZOLT         , //= EVO_TYPE_0,
    SPECIES_ARCTOZOLT         , //= EVO_TYPE_0,
    SPECIES_DRACOVISH         , //= EVO_TYPE_0,
    SPECIES_ARCTOVISH         , //= EVO_TYPE_0,
    SPECIES_DURALUDON         , //= EVO_TYPE_0,
    SPECIES_DREEPY            , //= EVO_TYPE_0,
    SPECIES_RATTATA_ALOLAN     , //= EVO_TYPE_0,
    SPECIES_SANDSHREW_ALOLAN   , //= EVO_TYPE_0,
    SPECIES_VULPIX_ALOLAN      , //= EVO_TYPE_0,
    SPECIES_DIGLETT_ALOLAN     , //= EVO_TYPE_0,
    SPECIES_MEOWTH_ALOLAN      , //= EVO_TYPE_0,
    SPECIES_GEODUDE_ALOLAN     , //= EVO_TYPE_0,
    SPECIES_GRIMER_ALOLAN      , //= EVO_TYPE_0,
    SPECIES_MEOWTH_GALARIAN    , //= EVO_TYPE_0,
    SPECIES_PONYTA_GALARIAN    , //= EVO_TYPE_0,
    SPECIES_SLOWPOKE_GALARIAN  , //= EVO_TYPE_0,
    SPECIES_FARFETCHD_GALARIAN , //= EVO_TYPE_0,
    SPECIES_ZIGZAGOON_GALARIAN , //= EVO_TYPE_0,
    SPECIES_DARUMAKA_GALARIAN  , //= EVO_TYPE_0,
    SPECIES_YAMASK_GALARIAN    , //= EVO_TYPE_0,
    SPECIES_STUNFISK_GALARIAN  , //= EVO_TYPE_0,
    SPECIES_PIKACHU_COSPLAY    , //= EVO_TYPE_0,
    SPECIES_PIKACHU_ROCK_STAR  , //= EVO_TYPE_0,
    SPECIES_PIKACHU_BELLE      , //= EVO_TYPE_0,
    SPECIES_PIKACHU_POP_STAR   , //= EVO_TYPE_0,
    SPECIES_PIKACHU_PH_D       , //= EVO_TYPE_0,
    SPECIES_PIKACHU_LIBRE      , //= EVO_TYPE_0,
    SPECIES_PIKACHU_ORIGINAL_CAP , //= EVO_TYPE_0,
    SPECIES_PIKACHU_HOENN_CAP  , //= EVO_TYPE_0,
    SPECIES_PIKACHU_SINNOH_CAP , //= EVO_TYPE_0,
    SPECIES_PIKACHU_UNOVA_CAP  , //= EVO_TYPE_0,
    SPECIES_PIKACHU_KALOS_CAP  , //= EVO_TYPE_0,
    SPECIES_PIKACHU_ALOLA_CAP  , //= EVO_TYPE_0,
    SPECIES_PIKACHU_PARTNER_CAP , //= EVO_TYPE_0,
    SPECIES_PIKACHU_WORLD_CAP  , //= EVO_TYPE_0,
    SPECIES_PICHU_SPIKY_EARED  , //= EVO_TYPE_0,
    SPECIES_BURMY_SANDY_CLOAK  , //= EVO_TYPE_0,
    SPECIES_BURMY_TRASH_CLOAK  , //= EVO_TYPE_0,
    SPECIES_SHELLOS_EAST_SEA   , //= EVO_TYPE_0,
    SPECIES_ROTOM_HEAT         , //= EVO_TYPE_0,
    SPECIES_ROTOM_WASH         , //= EVO_TYPE_0,
    SPECIES_ROTOM_FROST        , //= EVO_TYPE_0,
    SPECIES_ROTOM_FAN          , //= EVO_TYPE_0,
    SPECIES_ROTOM_MOW          , //= EVO_TYPE_0,
    SPECIES_BASCULIN_BLUE_STRIPED , //= EVO_TYPE_0,
    SPECIES_DEERLING_SUMMER    , //= EVO_TYPE_0,
    SPECIES_DEERLING_AUTUMN    , //= EVO_TYPE_0,
    SPECIES_DEERLING_WINTER    , //= EVO_TYPE_0,
    SPECIES_FLABEBE_YELLOW_FLOWER , //= EVO_TYPE_0,
    SPECIES_FLABEBE_ORANGE_FLOWER , //= EVO_TYPE_0,
    SPECIES_FLABEBE_BLUE_FLOWER , //= EVO_TYPE_0,
    SPECIES_FLABEBE_WHITE_FLOWER , //= EVO_TYPE_0,
    SPECIES_FURFROU_HEART_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_STAR_TRIM  , //= EVO_TYPE_0,
    SPECIES_FURFROU_DIAMOND_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_DEBUTANTE_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_MATRON_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_DANDY_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_LA_REINE_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_KABUKI_TRIM , //= EVO_TYPE_0,
    SPECIES_FURFROU_PHARAOH_TRIM , //= EVO_TYPE_0,
    SPECIES_PUMPKABOO_SMALL    , //= EVO_TYPE_0,
    SPECIES_PUMPKABOO_LARGE    , //= EVO_TYPE_0,
    SPECIES_PUMPKABOO_SUPER    , //= EVO_TYPE_0,
    SPECIES_ORICORIO_POM_POM   , //= EVO_TYPE_0,
    SPECIES_ORICORIO_PAU       , //= EVO_TYPE_0,
    SPECIES_ORICORIO_SENSU     , //= EVO_TYPE_0,
    SPECIES_ROCKRUFF_OWN_TEMPO , //= EVO_TYPE_0,
    SPECIES_WISHIWASHI_SCHOOL  , //= EVO_TYPE_0,
    SPECIES_MINIOR_METEOR_ORANGE , //= EVO_TYPE_0,
    SPECIES_MINIOR_METEOR_YELLOW , //= EVO_TYPE_0,
    SPECIES_MINIOR_METEOR_GREEN , //= EVO_TYPE_0,
    SPECIES_MINIOR_METEOR_BLUE , //= EVO_TYPE_0,
    SPECIES_MINIOR_METEOR_INDIGO , //= EVO_TYPE_0,
    SPECIES_MINIOR_METEOR_VIOLET , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_RED    , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_ORANGE , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_YELLOW , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_GREEN  , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_BLUE   , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_INDIGO , //= EVO_TYPE_0,
    SPECIES_MINIOR_CORE_VIOLET , //= EVO_TYPE_0,
    SPECIES_MIMIKYU_BUSTED     , //= EVO_TYPE_0,
    SPECIES_CRAMORANT_GULPING  , //= EVO_TYPE_0,
    SPECIES_CRAMORANT_GORGING  , //= EVO_TYPE_0,
    SPECIES_SINISTEA_ANTIQUE   , //= EVO_TYPE_0,
    SPECIES_EISCUE_NOICE_FACE  , //= EVO_TYPE_0,
    SPECIES_MORPEKO_HANGRY     , //= EVO_TYPE_0,
    #endif
};
#define RANDOM_SPECIES_EVO_1_COUNT ARRAY_COUNT(sRandomSpeciesEvo1)
static const u16 sRandomSpeciesEvo1[] =
{
    SPECIES_IVYSAUR         , //= EVO_TYPE_1,
    SPECIES_CHARMELEON      , //= EVO_TYPE_1,
    SPECIES_WARTORTLE       , //= EVO_TYPE_1,
    SPECIES_METAPOD         , //= EVO_TYPE_1,
    SPECIES_KAKUNA          , //= EVO_TYPE_1,
    SPECIES_PIDGEOTTO       , //= EVO_TYPE_1,
    SPECIES_RATICATE        , //= EVO_TYPE_1,
    SPECIES_FEAROW          , //= EVO_TYPE_1,
    SPECIES_ARBOK           , //= EVO_TYPE_1,
    SPECIES_PIKACHU         , //= EVO_TYPE_1,
    SPECIES_SANDSLASH       , //= EVO_TYPE_1,
    SPECIES_NIDORINA        , //= EVO_TYPE_1,
    SPECIES_NIDORINO        , //= EVO_TYPE_1,
    SPECIES_CLEFAIRY        , //= EVO_TYPE_1,
    SPECIES_NINETALES       , //= EVO_TYPE_1,
    SPECIES_JIGGLYPUFF      , //= EVO_TYPE_1,
    SPECIES_GOLBAT          , //= EVO_TYPE_1,
    SPECIES_GLOOM           , //= EVO_TYPE_1,
    SPECIES_PARASECT        , //= EVO_TYPE_1,
    SPECIES_VENOMOTH        , //= EVO_TYPE_1,
    SPECIES_DUGTRIO         , //= EVO_TYPE_1,
    SPECIES_PERSIAN         , //= EVO_TYPE_1,
    SPECIES_GOLDUCK         , //= EVO_TYPE_1,
    SPECIES_PRIMEAPE        , //= EVO_TYPE_1,
    SPECIES_ARCANINE        , //= EVO_TYPE_1,
    SPECIES_POLIWHIRL       , //= EVO_TYPE_1,
    SPECIES_KADABRA         , //= EVO_TYPE_1,
    SPECIES_MACHOKE         , //= EVO_TYPE_1,
    SPECIES_WEEPINBELL      , //= EVO_TYPE_1,
    SPECIES_TENTACRUEL      , //= EVO_TYPE_1,
    SPECIES_GRAVELER        , //= EVO_TYPE_1,
    SPECIES_RAPIDASH        , //= EVO_TYPE_1,
    SPECIES_MAGNETON        , //= EVO_TYPE_1,
    SPECIES_DODRIO          , //= EVO_TYPE_1,
    SPECIES_DEWGONG         , //= EVO_TYPE_1,
    SPECIES_MUK             , //= EVO_TYPE_1,
    SPECIES_CLOYSTER        , //= EVO_TYPE_1,
    SPECIES_HAUNTER         , //= EVO_TYPE_1,
    SPECIES_HYPNO           , //= EVO_TYPE_1,
    SPECIES_KINGLER         , //= EVO_TYPE_1,
    SPECIES_ELECTRODE       , //= EVO_TYPE_1,
    SPECIES_EXEGGUTOR       , //= EVO_TYPE_1,
    SPECIES_MAROWAK         , //= EVO_TYPE_1,
    SPECIES_HITMONLEE       , //= EVO_TYPE_1,
    SPECIES_HITMONCHAN      , //= EVO_TYPE_1,
    SPECIES_WEEZING         , //= EVO_TYPE_1,
    SPECIES_RHYDON          , //= EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    SPECIES_CHANSEY         , //= EVO_TYPE_1,
    #endif
    SPECIES_SEADRA          , //= EVO_TYPE_1,
    SPECIES_SEAKING         , //= EVO_TYPE_1,
    SPECIES_STARMIE         , //= EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    SPECIES_MR_MIME         , //= EVO_TYPE_1,
    #endif
    SPECIES_JYNX            , //= EVO_TYPE_1,
    SPECIES_ELECTABUZZ      , //= EVO_TYPE_1,
    SPECIES_MAGMAR          , //= EVO_TYPE_1,
    SPECIES_VAPOREON        , //= EVO_TYPE_1,
    SPECIES_JOLTEON         , //= EVO_TYPE_1,
    SPECIES_FLAREON         , //= EVO_TYPE_1,
    SPECIES_OMASTAR         , //= EVO_TYPE_1,
    SPECIES_KABUTOPS        , //= EVO_TYPE_1,
    SPECIES_DRAGONAIR       , //= EVO_TYPE_1,
    SPECIES_BAYLEEF         , //= EVO_TYPE_1,
    SPECIES_QUILAVA         , //= EVO_TYPE_1,
    SPECIES_CROCONAW        , //= EVO_TYPE_1,
    SPECIES_FURRET          , //= EVO_TYPE_1,
    SPECIES_NOCTOWL         , //= EVO_TYPE_1,
    SPECIES_LEDIAN          , //= EVO_TYPE_1,
    SPECIES_ARIADOS         , //= EVO_TYPE_1,
    SPECIES_LANTURN         , //= EVO_TYPE_1,
    SPECIES_TOGETIC         , //= EVO_TYPE_1,
    SPECIES_XATU            , //= EVO_TYPE_1,
    SPECIES_FLAAFFY         , //= EVO_TYPE_1,
    SPECIES_MARILL          , //= EVO_TYPE_1,
    SPECIES_SKIPLOOM        , //= EVO_TYPE_1,
    SPECIES_SUNFLORA        , //= EVO_TYPE_1,
    SPECIES_QUAGSIRE        , //= EVO_TYPE_1,
    SPECIES_ESPEON          , //= EVO_TYPE_1,
    SPECIES_UMBREON         , //= EVO_TYPE_1,
    SPECIES_WOBBUFFET       , //= EVO_TYPE_1,
    SPECIES_FORRETRESS      , //= EVO_TYPE_1,
    SPECIES_STEELIX         , //= EVO_TYPE_1,
    SPECIES_GRANBULL        , //= EVO_TYPE_1,
    SPECIES_SCIZOR          , //= EVO_TYPE_1,
    SPECIES_URSARING        , //= EVO_TYPE_1,
    SPECIES_MAGCARGO        , //= EVO_TYPE_1,
    SPECIES_PILOSWINE       , //= EVO_TYPE_1,
    SPECIES_OCTILLERY       , //= EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    SPECIES_MANTINE         , //= EVO_TYPE_1,
    #endif
    SPECIES_HOUNDOOM        , //= EVO_TYPE_1,
    SPECIES_DONPHAN         , //= EVO_TYPE_1,
    SPECIES_PORYGON2        , //= EVO_TYPE_1,
    SPECIES_HITMONTOP       , //= EVO_TYPE_1,
    SPECIES_PUPITAR         , //= EVO_TYPE_1,
    SPECIES_GROVYLE         , //= EVO_TYPE_1,
    SPECIES_COMBUSKEN       , //= EVO_TYPE_1,
    SPECIES_MARSHTOMP       , //= EVO_TYPE_1,
    SPECIES_MIGHTYENA       , //= EVO_TYPE_1,
    SPECIES_LINOONE         , //= EVO_TYPE_1,
    SPECIES_SILCOON         , //= EVO_TYPE_1,
    SPECIES_CASCOON         , //= EVO_TYPE_1,
    SPECIES_LOMBRE          , //= EVO_TYPE_1,
    SPECIES_NUZLEAF         , //= EVO_TYPE_1,
    SPECIES_NINJASK         , //= EVO_TYPE_1,
    SPECIES_SHEDINJA        , //= EVO_TYPE_1,
    SPECIES_SWELLOW         , //= EVO_TYPE_1,
    SPECIES_BRELOOM         , //= EVO_TYPE_1,
    SPECIES_PELIPPER        , //= EVO_TYPE_1,
    SPECIES_MASQUERAIN      , //= EVO_TYPE_1,
    SPECIES_WAILORD         , //= EVO_TYPE_1,
    SPECIES_DELCATTY        , //= EVO_TYPE_1,
    SPECIES_CLAYDOL         , //= EVO_TYPE_1,
    SPECIES_WHISCASH        , //= EVO_TYPE_1,
    SPECIES_CRAWDAUNT       , //= EVO_TYPE_1,
    SPECIES_MILOTIC         , //= EVO_TYPE_1,
    SPECIES_SHARPEDO        , //= EVO_TYPE_1,
    SPECIES_VIBRAVA         , //= EVO_TYPE_1,
    SPECIES_HARIYAMA        , //= EVO_TYPE_1,
    SPECIES_MANECTRIC       , //= EVO_TYPE_1,
    SPECIES_CAMERUPT        , //= EVO_TYPE_1,
    SPECIES_SEALEO          , //= EVO_TYPE_1,
    SPECIES_CACTURNE        , //= EVO_TYPE_1,
    SPECIES_GLALIE          , //= EVO_TYPE_1,
    SPECIES_GRUMPIG         , //= EVO_TYPE_1,
    SPECIES_MEDICHAM        , //= EVO_TYPE_1,
    SPECIES_ALTARIA         , //= EVO_TYPE_1,
    SPECIES_DUSCLOPS        , //= EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    SPECIES_ROSELIA         , //= EVO_TYPE_1,
    #endif
    SPECIES_VIGOROTH        , //= EVO_TYPE_1,
    SPECIES_SWALOT          , //= EVO_TYPE_1,
    SPECIES_LOUDRED         , //= EVO_TYPE_1,
    SPECIES_HUNTAIL         , //= EVO_TYPE_1,
    SPECIES_GOREBYSS        , //= EVO_TYPE_1,
    SPECIES_BANETTE         , //= EVO_TYPE_1,
    SPECIES_LAIRON          , //= EVO_TYPE_1,
    SPECIES_VOLBEAT         , //= EVO_TYPE_1,
    SPECIES_ILLUMISE        , //= EVO_TYPE_1,
    SPECIES_CRADILY         , //= EVO_TYPE_1,
    SPECIES_ARMALDO         , //= EVO_TYPE_1,
    SPECIES_KIRLIA          , //= EVO_TYPE_1,
    SPECIES_SHELGON         , //= EVO_TYPE_1,
    SPECIES_METANG          , //= EVO_TYPE_1,
    #ifdef POKEMON_EXPANSION
    SPECIES_CHIMECHO        , //= EVO_TYPE_1,
    SPECIES_GROTLE            , //= EVO_TYPE_1,
    SPECIES_MONFERNO          , //= EVO_TYPE_1,
    SPECIES_PRINPLUP          , //= EVO_TYPE_1,
    SPECIES_STARAVIA          , //= EVO_TYPE_1,
    SPECIES_BIBAREL           , //= EVO_TYPE_1,
    SPECIES_KRICKETUNE        , //= EVO_TYPE_1,
    SPECIES_LUXIO             , //= EVO_TYPE_1,
    SPECIES_RAMPARDOS         , //= EVO_TYPE_1,
    SPECIES_BASTIODON         , //= EVO_TYPE_1,
    SPECIES_WORMADAM          , //= EVO_TYPE_1,
    SPECIES_MOTHIM            , //= EVO_TYPE_1,
    SPECIES_VESPIQUEN         , //= EVO_TYPE_1,
    SPECIES_FLOATZEL          , //= EVO_TYPE_1,
    SPECIES_CHERRIM           , //= EVO_TYPE_1,
    SPECIES_GASTRODON         , //= EVO_TYPE_1,
    SPECIES_AMBIPOM           , //= EVO_TYPE_1,
    SPECIES_DRIFBLIM          , //= EVO_TYPE_1,
    SPECIES_LOPUNNY           , //= EVO_TYPE_1,
    SPECIES_MISMAGIUS         , //= EVO_TYPE_1,
    SPECIES_HONCHKROW         , //= EVO_TYPE_1,
    SPECIES_PURUGLY           , //= EVO_TYPE_1,
    SPECIES_SKUNTANK          , //= EVO_TYPE_1,
    SPECIES_BRONZONG          , //= EVO_TYPE_1,
    SPECIES_GABITE            , //= EVO_TYPE_1,
    SPECIES_LUCARIO           , //= EVO_TYPE_1,
    SPECIES_HIPPOWDON         , //= EVO_TYPE_1,
    SPECIES_DRAPION           , //= EVO_TYPE_1,
    SPECIES_TOXICROAK         , //= EVO_TYPE_1,
    SPECIES_LUMINEON          , //= EVO_TYPE_1,
    SPECIES_ABOMASNOW         , //= EVO_TYPE_1,
    SPECIES_WEAVILE           , //= EVO_TYPE_1,
    SPECIES_LICKILICKY        , //= EVO_TYPE_1,
    SPECIES_TANGROWTH         , //= EVO_TYPE_1,
    SPECIES_YANMEGA           , //= EVO_TYPE_1,
    SPECIES_LEAFEON           , //= EVO_TYPE_1,
    SPECIES_GLACEON           , //= EVO_TYPE_1,
    SPECIES_GLISCOR           , //= EVO_TYPE_1,
    SPECIES_PROBOPASS         , //= EVO_TYPE_1,
    SPECIES_FROSLASS          , //= EVO_TYPE_1,
    SPECIES_SERVINE           , //= EVO_TYPE_1,
    SPECIES_PIGNITE           , //= EVO_TYPE_1,
    SPECIES_DEWOTT            , //= EVO_TYPE_1,
    SPECIES_WATCHOG           , //= EVO_TYPE_1,
    SPECIES_HERDIER           , //= EVO_TYPE_1,
    SPECIES_LIEPARD           , //= EVO_TYPE_1,
    SPECIES_SIMISAGE          , //= EVO_TYPE_1,
    SPECIES_SIMISEAR          , //= EVO_TYPE_1,
    SPECIES_SIMIPOUR          , //= EVO_TYPE_1,
    SPECIES_MUSHARNA          , //= EVO_TYPE_1,
    SPECIES_TRANQUILL         , //= EVO_TYPE_1,
    SPECIES_ZEBSTRIKA         , //= EVO_TYPE_1,
    SPECIES_BOLDORE           , //= EVO_TYPE_1,
    SPECIES_SWOOBAT           , //= EVO_TYPE_1,
    SPECIES_EXCADRILL         , //= EVO_TYPE_1,
    SPECIES_GURDURR           , //= EVO_TYPE_1,
    SPECIES_PALPITOAD         , //= EVO_TYPE_1,
    SPECIES_SWADLOON          , //= EVO_TYPE_1,
    SPECIES_WHIRLIPEDE        , //= EVO_TYPE_1,
    SPECIES_WHIMSICOTT        , //= EVO_TYPE_1,
    SPECIES_LILLIGANT         , //= EVO_TYPE_1,
    SPECIES_KROKOROK          , //= EVO_TYPE_1,
    SPECIES_DARMANITAN        , //= EVO_TYPE_1,
    SPECIES_CRUSTLE           , //= EVO_TYPE_1,
    SPECIES_SCRAFTY           , //= EVO_TYPE_1,
    SPECIES_COFAGRIGUS        , //= EVO_TYPE_1,
    SPECIES_CARRACOSTA        , //= EVO_TYPE_1,
    SPECIES_ARCHEOPS          , //= EVO_TYPE_1,
    SPECIES_GARBODOR          , //= EVO_TYPE_1,
    SPECIES_ZOROARK           , //= EVO_TYPE_1,
    SPECIES_CINCCINO          , //= EVO_TYPE_1,
    SPECIES_GOTHORITA         , //= EVO_TYPE_1,
    SPECIES_DUOSION           , //= EVO_TYPE_1,
    SPECIES_SWANNA            , //= EVO_TYPE_1,
    SPECIES_VANILLISH         , //= EVO_TYPE_1,
    SPECIES_SAWSBUCK          , //= EVO_TYPE_1,
    SPECIES_ESCAVALIER        , //= EVO_TYPE_1,
    SPECIES_AMOONGUSS         , //= EVO_TYPE_1,
    SPECIES_JELLICENT         , //= EVO_TYPE_1,
    SPECIES_GALVANTULA        , //= EVO_TYPE_1,
    SPECIES_FERROTHORN        , //= EVO_TYPE_1,
    SPECIES_KLANG             , //= EVO_TYPE_1,
    SPECIES_EELEKTRIK         , //= EVO_TYPE_1,
    SPECIES_BEHEEYEM          , //= EVO_TYPE_1,
    SPECIES_LAMPENT           , //= EVO_TYPE_1,
    SPECIES_FRAXURE           , //= EVO_TYPE_1,
    SPECIES_BEARTIC           , //= EVO_TYPE_1,
    SPECIES_ACCELGOR          , //= EVO_TYPE_1,
    SPECIES_MIENSHAO          , //= EVO_TYPE_1,
    SPECIES_GOLURK            , //= EVO_TYPE_1,
    SPECIES_BISHARP           , //= EVO_TYPE_1,
    SPECIES_BRAVIARY          , //= EVO_TYPE_1,
    SPECIES_MANDIBUZZ         , //= EVO_TYPE_1,
    SPECIES_ZWEILOUS          , //= EVO_TYPE_1,
    SPECIES_VOLCARONA         , //= EVO_TYPE_1,
    SPECIES_QUILLADIN         , //= EVO_TYPE_1,
    SPECIES_BRAIXEN           , //= EVO_TYPE_1,
    SPECIES_FROGADIER         , //= EVO_TYPE_1,
    SPECIES_DIGGERSBY         , //= EVO_TYPE_1,
    SPECIES_FLETCHINDER       , //= EVO_TYPE_1,
    SPECIES_SPEWPA            , //= EVO_TYPE_1,
    SPECIES_PYROAR            , //= EVO_TYPE_1,
    SPECIES_FLOETTE           , //= EVO_TYPE_1,
    SPECIES_GOGOAT            , //= EVO_TYPE_1,
    SPECIES_PANGORO           , //= EVO_TYPE_1,
    SPECIES_MEOWSTIC          , //= EVO_TYPE_1,
    SPECIES_DOUBLADE          , //= EVO_TYPE_1,
    SPECIES_AROMATISSE        , //= EVO_TYPE_1,
    SPECIES_SLURPUFF          , //= EVO_TYPE_1,
    SPECIES_MALAMAR           , //= EVO_TYPE_1,
    SPECIES_BARBARACLE        , //= EVO_TYPE_1,
    SPECIES_DRAGALGE          , //= EVO_TYPE_1,
    SPECIES_CLAWITZER         , //= EVO_TYPE_1,
    SPECIES_HELIOLISK         , //= EVO_TYPE_1,
    SPECIES_TYRANTRUM         , //= EVO_TYPE_1,
    SPECIES_AURORUS           , //= EVO_TYPE_1,
    SPECIES_SYLVEON           , //= EVO_TYPE_1,
    SPECIES_SLIGGOO           , //= EVO_TYPE_1,
    SPECIES_TREVENANT         , //= EVO_TYPE_1,
    SPECIES_GOURGEIST         , //= EVO_TYPE_1,
    SPECIES_AVALUGG           , //= EVO_TYPE_1,
    SPECIES_NOIVERN           , //= EVO_TYPE_1,
    SPECIES_DARTRIX           , //= EVO_TYPE_1,
    SPECIES_TORRACAT          , //= EVO_TYPE_1,
    SPECIES_BRIONNE           , //= EVO_TYPE_1,
    SPECIES_TRUMBEAK          , //= EVO_TYPE_1,
    SPECIES_GUMSHOOS          , //= EVO_TYPE_1,
    SPECIES_CHARJABUG         , //= EVO_TYPE_1,
    SPECIES_CRABOMINABLE      , //= EVO_TYPE_1,
    SPECIES_RIBOMBEE          , //= EVO_TYPE_1,
    SPECIES_LYCANROC          , //= EVO_TYPE_1,
    SPECIES_TOXAPEX           , //= EVO_TYPE_1,
    SPECIES_MUDSDALE          , //= EVO_TYPE_1,
    SPECIES_ARAQUANID         , //= EVO_TYPE_1,
    SPECIES_LURANTIS          , //= EVO_TYPE_1,
    SPECIES_SHIINOTIC         , //= EVO_TYPE_1,
    SPECIES_SALAZZLE          , //= EVO_TYPE_1,
    SPECIES_BEWEAR            , //= EVO_TYPE_1,
    SPECIES_STEENEE           , //= EVO_TYPE_1,
    SPECIES_GOLISOPOD         , //= EVO_TYPE_1,
    SPECIES_PALOSSAND         , //= EVO_TYPE_1,
    SPECIES_HAKAMO_O          , //= EVO_TYPE_1,
    SPECIES_THWACKEY          , //= EVO_TYPE_1,
    SPECIES_RABOOT            , //= EVO_TYPE_1,
    SPECIES_DRIZZILE          , //= EVO_TYPE_1,
    SPECIES_GREEDENT          , //= EVO_TYPE_1,
    SPECIES_CORVISQUIRE       , //= EVO_TYPE_1,
    SPECIES_DOTTLER           , //= EVO_TYPE_1,
    SPECIES_THIEVUL           , //= EVO_TYPE_1,
    SPECIES_ELDEGOSS          , //= EVO_TYPE_1,
    SPECIES_DUBWOOL           , //= EVO_TYPE_1,
    SPECIES_DREDNAW           , //= EVO_TYPE_1,
    SPECIES_BOLTUND           , //= EVO_TYPE_1,
    SPECIES_CARKOL            , //= EVO_TYPE_1,
    SPECIES_FLAPPLE           , //= EVO_TYPE_1,
    SPECIES_APPLETUN          , //= EVO_TYPE_1,
    SPECIES_SANDACONDA        , //= EVO_TYPE_1,
    SPECIES_BARRASKEWDA       , //= EVO_TYPE_1,
    SPECIES_TOXTRICITY        , //= EVO_TYPE_1,
    SPECIES_CENTISKORCH       , //= EVO_TYPE_1,
    SPECIES_GRAPPLOCT         , //= EVO_TYPE_1,
    SPECIES_POLTEAGEIST       , //= EVO_TYPE_1,
    SPECIES_HATTREM           , //= EVO_TYPE_1,
    SPECIES_MORGREM           , //= EVO_TYPE_1,
    SPECIES_PERRSERKER        , //= EVO_TYPE_1,
    SPECIES_CURSOLA           , //= EVO_TYPE_1,
    SPECIES_SIRFETCHD         , //= EVO_TYPE_1,
    SPECIES_RUNERIGUS         , //= EVO_TYPE_1,
    SPECIES_ALCREMIE          , //= EVO_TYPE_1,
    SPECIES_FROSMOTH          , //= EVO_TYPE_1,
    SPECIES_COPPERAJAH        , //= EVO_TYPE_1,
    SPECIES_DRAKLOAK          , //= EVO_TYPE_1,
    SPECIES_RATICATE_ALOLAN    , //= EVO_TYPE_1,
    SPECIES_SANDSLASH_ALOLAN   , //= EVO_TYPE_1,
    SPECIES_NINETALES_ALOLAN   , //= EVO_TYPE_1,
    SPECIES_DUGTRIO_ALOLAN     , //= EVO_TYPE_1,
    SPECIES_PERSIAN_ALOLAN     , //= EVO_TYPE_1,
    SPECIES_GRAVELER_ALOLAN    , //= EVO_TYPE_1,
    SPECIES_MUK_ALOLAN         , //= EVO_TYPE_1,
    SPECIES_EXEGGUTOR_ALOLAN   , //= EVO_TYPE_1,
    SPECIES_MAROWAK_ALOLAN     , //= EVO_TYPE_1,
    SPECIES_RAPIDASH_GALARIAN  , //= EVO_TYPE_1,
    SPECIES_SLOWBRO_GALARIAN   , //= EVO_TYPE_1,
    SPECIES_WEEZING_GALARIAN   , //= EVO_TYPE_1,
    SPECIES_MR_MIME_GALARIAN   , //= EVO_TYPE_1,
    SPECIES_SLOWKING_GALARIAN  , //= EVO_TYPE_1,
    SPECIES_CORSOLA_GALARIAN   , //= EVO_TYPE_1,
    SPECIES_LINOONE_GALARIAN   , //= EVO_TYPE_1,
    SPECIES_DARMANITAN_GALARIAN , //= EVO_TYPE_1,
    SPECIES_WORMADAM_SANDY_CLOAK , //= EVO_TYPE_1,
    SPECIES_WORMADAM_TRASH_CLOAK , //= EVO_TYPE_1,
    SPECIES_CHERRIM_SUNSHINE   , //= EVO_TYPE_1,
    SPECIES_GASTRODON_EAST_SEA , //= EVO_TYPE_1,
    SPECIES_DARMANITAN_ZEN_MODE , //= EVO_TYPE_1,
    SPECIES_DARMANITAN_ZEN_MODE_GALARIAN , //= EVO_TYPE_1,
    SPECIES_SAWSBUCK_SUMMER    , //= EVO_TYPE_1,
    SPECIES_SAWSBUCK_AUTUMN    , //= EVO_TYPE_1,
    SPECIES_SAWSBUCK_WINTER    , //= EVO_TYPE_1,
    SPECIES_FLOETTE_YELLOW_FLOWER , //= EVO_TYPE_1,
    SPECIES_FLOETTE_ORANGE_FLOWER , //= EVO_TYPE_1,
    SPECIES_FLOETTE_BLUE_FLOWER , //= EVO_TYPE_1,
    SPECIES_FLOETTE_WHITE_FLOWER , //= EVO_TYPE_1,
    SPECIES_MEOWSTIC_FEMALE    , //= EVO_TYPE_1,
    SPECIES_GOURGEIST_SMALL    , //= EVO_TYPE_1,
    SPECIES_GOURGEIST_LARGE    , //= EVO_TYPE_1,
    SPECIES_GOURGEIST_SUPER    , //= EVO_TYPE_1,
    SPECIES_LYCANROC_MIDNIGHT  , //= EVO_TYPE_1,
    SPECIES_LYCANROC_DUSK      , //= EVO_TYPE_1,
    SPECIES_TOXTRICITY_LOW_KEY , //= EVO_TYPE_1,
    SPECIES_POLTEAGEIST_ANTIQUE , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_RUBY_CREAM , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_MATCHA_CREAM , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_MINT_CREAM , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_LEMON_CREAM , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_SALTED_CREAM , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_RUBY_SWIRL , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_CARAMEL_SWIRL , //= EVO_TYPE_1,
    SPECIES_ALCREMIE_RAINBOW_SWIRL , //= EVO_TYPE_1,
    SPECIES_INDEEDEE_FEMALE    , //= EVO_TYPE_1,
    #endif
};
#define RANDOM_SPECIES_EVO_2_COUNT ARRAY_COUNT(sRandomSpeciesEvo2)
static const u16 sRandomSpeciesEvo2[] =
{
    SPECIES_VENUSAUR        , //= EVO_TYPE_2,
    SPECIES_CHARIZARD       , //= EVO_TYPE_2,
    SPECIES_BLASTOISE       , //= EVO_TYPE_2,
    SPECIES_BUTTERFREE      , //= EVO_TYPE_2,
    SPECIES_BEEDRILL        , //= EVO_TYPE_2,
    SPECIES_PIDGEOT         , //= EVO_TYPE_2,
    SPECIES_RAICHU          , //= EVO_TYPE_2,
    SPECIES_NIDOQUEEN       , //= EVO_TYPE_2,
    SPECIES_NIDOKING        , //= EVO_TYPE_2,
    SPECIES_CLEFABLE        , //= EVO_TYPE_2,
    SPECIES_WIGGLYTUFF      , //= EVO_TYPE_2,
    SPECIES_VILEPLUME       , //= EVO_TYPE_2,
    SPECIES_POLIWRATH       , //= EVO_TYPE_2,
    SPECIES_ALAKAZAM        , //= EVO_TYPE_2,
    SPECIES_MACHAMP         , //= EVO_TYPE_2,
    SPECIES_VICTREEBEL      , //= EVO_TYPE_2,
    SPECIES_GOLEM           , //= EVO_TYPE_2,
    SPECIES_SLOWBRO         , //= EVO_TYPE_2,
    SPECIES_GENGAR          , //= EVO_TYPE_2,
    SPECIES_GYARADOS        , //= EVO_TYPE_2,
    SPECIES_DRAGONITE       , //= EVO_TYPE_2,
    SPECIES_MEGANIUM        , //= EVO_TYPE_2,
    SPECIES_TYPHLOSION      , //= EVO_TYPE_2,
    SPECIES_FERALIGATR      , //= EVO_TYPE_2,
    SPECIES_CROBAT          , //= EVO_TYPE_2,
    SPECIES_AMPHAROS        , //= EVO_TYPE_2,
    SPECIES_BELLOSSOM       , //= EVO_TYPE_2,
    SPECIES_AZUMARILL       , //= EVO_TYPE_2,
    SPECIES_POLITOED        , //= EVO_TYPE_2,
    SPECIES_JUMPLUFF        , //= EVO_TYPE_2,
    SPECIES_SLOWKING        , //= EVO_TYPE_2,
    SPECIES_KINGDRA         , //= EVO_TYPE_2,
    SPECIES_BLISSEY         , //= EVO_TYPE_2,
    SPECIES_TYRANITAR       , //= EVO_TYPE_2,
    SPECIES_SCEPTILE        , //= EVO_TYPE_2,
    SPECIES_BLAZIKEN        , //= EVO_TYPE_2,
    SPECIES_SWAMPERT        , //= EVO_TYPE_2,
    SPECIES_BEAUTIFLY       , //= EVO_TYPE_2,
    SPECIES_DUSTOX          , //= EVO_TYPE_2,
    SPECIES_LUDICOLO        , //= EVO_TYPE_2,
    SPECIES_SHIFTRY         , //= EVO_TYPE_2,
    SPECIES_FLYGON          , //= EVO_TYPE_2,
    SPECIES_WALREIN         , //= EVO_TYPE_2,
    SPECIES_SLAKING         , //= EVO_TYPE_2,
    SPECIES_EXPLOUD         , //= EVO_TYPE_2,
    SPECIES_AGGRON          , //= EVO_TYPE_2,
    SPECIES_GARDEVOIR       , //= EVO_TYPE_2,
    SPECIES_SALAMENCE       , //= EVO_TYPE_2,
    SPECIES_METAGROSS       , //= EVO_TYPE_2,
    #ifdef POKEMON_EXPANSION
    SPECIES_TORTERRA          , //= EVO_TYPE_2,
    SPECIES_INFERNAPE         , //= EVO_TYPE_2,
    SPECIES_EMPOLEON          , //= EVO_TYPE_2,
    SPECIES_STARAPTOR         , //= EVO_TYPE_2,
    SPECIES_LUXRAY            , //= EVO_TYPE_2,
    SPECIES_ROSERADE          , //= EVO_TYPE_2,
    SPECIES_GARCHOMP          , //= EVO_TYPE_2,
    SPECIES_MAGNEZONE         , //= EVO_TYPE_2,
    SPECIES_RHYPERIOR         , //= EVO_TYPE_2,
    SPECIES_ELECTIVIRE        , //= EVO_TYPE_2,
    SPECIES_MAGMORTAR         , //= EVO_TYPE_2,
    SPECIES_TOGEKISS          , //= EVO_TYPE_2,
    SPECIES_MAMOSWINE         , //= EVO_TYPE_2,
    SPECIES_PORYGON_Z         , //= EVO_TYPE_2,
    SPECIES_GALLADE           , //= EVO_TYPE_2,
    SPECIES_DUSKNOIR          , //= EVO_TYPE_2,
    SPECIES_SERPERIOR         , //= EVO_TYPE_2,
    SPECIES_SAMUROTT          , //= EVO_TYPE_2,
    SPECIES_STOUTLAND         , //= EVO_TYPE_2,
    SPECIES_UNFEZANT          , //= EVO_TYPE_2,
    SPECIES_GIGALITH          , //= EVO_TYPE_2,
    SPECIES_CONKELDURR        , //= EVO_TYPE_2,
    SPECIES_SEISMITOAD        , //= EVO_TYPE_2,
    SPECIES_LEAVANNY          , //= EVO_TYPE_2,
    SPECIES_SCOLIPEDE         , //= EVO_TYPE_2,
    SPECIES_KROOKODILE        , //= EVO_TYPE_2,
    SPECIES_GOTHITELLE        , //= EVO_TYPE_2,
    SPECIES_REUNICLUS         , //= EVO_TYPE_2,
    SPECIES_VANILLUXE         , //= EVO_TYPE_2,
    SPECIES_KLINKLANG         , //= EVO_TYPE_2,
    SPECIES_EELEKTROSS        , //= EVO_TYPE_2,
    SPECIES_CHANDELURE        , //= EVO_TYPE_2,
    SPECIES_HAXORUS           , //= EVO_TYPE_2,
    SPECIES_HYDREIGON         , //= EVO_TYPE_2,
    SPECIES_CHESNAUGHT        , //= EVO_TYPE_2,
    SPECIES_DELPHOX           , //= EVO_TYPE_2,
    SPECIES_GRENINJA          , //= EVO_TYPE_2,
    SPECIES_TALONFLAME        , //= EVO_TYPE_2,
    SPECIES_VIVILLON          , //= EVO_TYPE_2,
    SPECIES_FLORGES           , //= EVO_TYPE_2,
    SPECIES_AEGISLASH         , //= EVO_TYPE_2,
    SPECIES_GOODRA            , //= EVO_TYPE_2,
    SPECIES_DECIDUEYE         , //= EVO_TYPE_2,
    SPECIES_INCINEROAR        , //= EVO_TYPE_2,
    SPECIES_PRIMARINA         , //= EVO_TYPE_2,
    SPECIES_TOUCANNON         , //= EVO_TYPE_2,
    SPECIES_VIKAVOLT          , //= EVO_TYPE_2,
    SPECIES_TSAREENA          , //= EVO_TYPE_2,
    SPECIES_KOMMO_O           , //= EVO_TYPE_2,
    SPECIES_RILLABOOM         , //= EVO_TYPE_2,
    SPECIES_CINDERACE         , //= EVO_TYPE_2,
    SPECIES_INTELEON          , //= EVO_TYPE_2,
    SPECIES_CORVIKNIGHT       , //= EVO_TYPE_2,
    SPECIES_ORBEETLE          , //= EVO_TYPE_2,
    SPECIES_COALOSSAL         , //= EVO_TYPE_2,
    SPECIES_HATTERENE         , //= EVO_TYPE_2,
    SPECIES_GRIMMSNARL        , //= EVO_TYPE_2,
    SPECIES_OBSTAGOON         , //= EVO_TYPE_2,
    SPECIES_MR_RIME           , //= EVO_TYPE_2,
    SPECIES_DRAGAPULT         , //= EVO_TYPE_2,
    SPECIES_RAICHU_ALOLAN      , //= EVO_TYPE_2,
    SPECIES_GOLEM_ALOLAN       , //= EVO_TYPE_2,
    SPECIES_GRENINJA_BATTLE_BOND , //= EVO_TYPE_2,
    SPECIES_GRENINJA_ASH       , //= EVO_TYPE_2,
    SPECIES_VIVILLON_POLAR     , //= EVO_TYPE_2,
    SPECIES_VIVILLON_TUNDRA    , //= EVO_TYPE_2,
    SPECIES_VIVILLON_CONTINENTAL , //= EVO_TYPE_2,
    SPECIES_VIVILLON_GARDEN    , //= EVO_TYPE_2,
    SPECIES_VIVILLON_ELEGANT   , //= EVO_TYPE_2,
    SPECIES_VIVILLON_MEADOW    , //= EVO_TYPE_2,
    SPECIES_VIVILLON_MODERN    , //= EVO_TYPE_2,
    SPECIES_VIVILLON_MARINE    , //= EVO_TYPE_2,
    SPECIES_VIVILLON_ARCHIPELAGO , //= EVO_TYPE_2,
    SPECIES_VIVILLON_HIGH_PLAINS , //= EVO_TYPE_2,
    SPECIES_VIVILLON_SANDSTORM , //= EVO_TYPE_2,
    SPECIES_VIVILLON_RIVER     , //= EVO_TYPE_2,
    SPECIES_VIVILLON_MONSOON   , //= EVO_TYPE_2,
    SPECIES_VIVILLON_SAVANNA   , //= EVO_TYPE_2,
    SPECIES_VIVILLON_SUN       , //= EVO_TYPE_2,
    SPECIES_VIVILLON_OCEAN     , //= EVO_TYPE_2,
    SPECIES_VIVILLON_JUNGLE    , //= EVO_TYPE_2,
    SPECIES_VIVILLON_FANCY     , //= EVO_TYPE_2,
    SPECIES_VIVILLON_POKE_BALL , //= EVO_TYPE_2,
    SPECIES_FLORGES_YELLOW_FLOWER , //= EVO_TYPE_2,
    SPECIES_FLORGES_ORANGE_FLOWER , //= EVO_TYPE_2,
    SPECIES_FLORGES_BLUE_FLOWER , //= EVO_TYPE_2,
    SPECIES_FLORGES_WHITE_FLOWER , //= EVO_TYPE_2,
    SPECIES_AEGISLASH_BLADE    , //= EVO_TYPE_2,
    #endif
};

static u8 CreateNPCTrainerParty(struct Pokemon *party, u16 trainerNum, bool8 firstTrainer)
{
    u32 nameHash = 0;
    u32 personalityValue;
    u8 fixedIV;
    s32 i, j;
    u8 monsCount;
    u8 localFlags;
    u16 species;

    if (trainerNum == TRAINER_SECRET_BASE)
        return 0;

    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER && !(gBattleTypeFlags & (BATTLE_TYPE_FRONTIER
                                                                        | BATTLE_TYPE_EREADER_TRAINER
                                                                        | BATTLE_TYPE_TRAINER_HILL)))
    {
        if (firstTrainer == TRUE)
            ZeroEnemyPartyMons();

        if (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS)
        {
            if (gTrainers[trainerNum].partySize > 3)
                monsCount = 3;
            else
                monsCount = gTrainers[trainerNum].partySize;
        }
        else
        {
            monsCount = gTrainers[trainerNum].partySize;
        }

        for (i = 0; i < monsCount; i++)
        {

            if (gTrainers[trainerNum].doubleBattle == TRUE)
                personalityValue = 0x80;
            else if (gTrainers[trainerNum].encounterMusic_gender & 0x80)
                personalityValue = 0x78;
            else
                personalityValue = 0x88;

            for (j = 0; gTrainers[trainerNum].trainerName[j] != EOS; j++)
                nameHash += gTrainers[trainerNum].trainerName[j];

            // If not an important fight, set the moveset to their default
            // level up one in order to allow for easy replacement of
            // trainer mons without having to do a million different
            // moveset edits to every generic trainer.
            if (gTrainers[gTrainerBattleOpponent_A].trainerClass != TRAINER_CLASS_LEADER || TRAINER_CLASS_PKMN_TRAINER_3 || TRAINER_CLASS_CHAMPION || TRAINER_CLASS_ELITE_FOUR) {
                localFlags = gTrainers[trainerNum].partyFlags;
                if (localFlags == F_TRAINER_PARTY_CUSTOM_MOVESET)
                    localFlags = 0;
            }

            switch (localFlags)
            {
            case 0:
            {
                const struct TrainerMonNoItemDefaultMoves *partyData = gTrainers[trainerNum].party.NoItemDefaultMoves;

                for (j = 0; gSpeciesNames[partyData[i].species][j] != EOS; j++)
                    nameHash += gSpeciesNames[partyData[i].species][j];

                personalityValue += nameHash << 8;
                fixedIV = partyData[i].iv * MAX_PER_STAT_IVS / 255;

                // Lets see if I can do this "randomization"
                species = partyData[i].species;
                if (species < SPECIES_TURTWIG) {
                    // Set the 2 types, tho I think I'm only
                    // gonna care about the primary (type1)
                    // type.
                    u8 type1 = gBaseStats[species].type1;
                    u8 type2 = gBaseStats[species].type2;

                    // Full credit to tx_randomizer_challenges
                    u16 speciesResult;
                    u8 slot = gSpeciesMapping[species];

                    do {
                        switch (slot)
                        {
                        case EVO_TYPE_0:
                            speciesResult = sRandomSpeciesEvo0[RandomSeededModulo(species, RANDOM_SPECIES_EVO_0_COUNT)];
                            break;
                        case EVO_TYPE_1:
                            speciesResult = sRandomSpeciesEvo1[RandomSeededModulo(species, RANDOM_SPECIES_EVO_1_COUNT)];
                            break;
                        case EVO_TYPE_2:
                            speciesResult = sRandomSpeciesEvo2[RandomSeededModulo(species, RANDOM_SPECIES_EVO_2_COUNT)];
                            break;
                        }
                        // Make sure the result is a gen 4-7 mon
                    } while ((speciesResult < SPECIES_TURTWIG || speciesResult >= SPECIES_GROOKEY) || gBaseStats[speciesResult].type1 != type1);
                    species = speciesResult;
                }

                CreateMon(&party[i], species, partyData[i].lvl, fixedIV, TRUE, personalityValue, OT_ID_RANDOM_NO_SHINY, 0);
                break;
            }
            case F_TRAINER_PARTY_CUSTOM_MOVESET:
            {
                const struct TrainerMonNoItemCustomMoves *partyData = gTrainers[trainerNum].party.NoItemCustomMoves;

                for (j = 0; gSpeciesNames[partyData[i].species][j] != EOS; j++)
                    nameHash += gSpeciesNames[partyData[i].species][j];

                personalityValue += nameHash << 8;
                fixedIV = partyData[i].iv * MAX_PER_STAT_IVS / 255;
                CreateMon(&party[i], partyData[i].species, partyData[i].lvl, fixedIV, TRUE, personalityValue, OT_ID_RANDOM_NO_SHINY, 0);

                for (j = 0; j < MAX_MON_MOVES; j++)
                {
                    SetMonData(&party[i], MON_DATA_MOVE1 + j, &partyData[i].moves[j]);
                    SetMonData(&party[i], MON_DATA_PP1 + j, &gBattleMoves[partyData[i].moves[j]].pp);
                }
                break;
            }
            case F_TRAINER_PARTY_HELD_ITEM:
            {
                const struct TrainerMonItemDefaultMoves *partyData = gTrainers[trainerNum].party.ItemDefaultMoves;

                for (j = 0; gSpeciesNames[partyData[i].species][j] != EOS; j++)
                    nameHash += gSpeciesNames[partyData[i].species][j];

                personalityValue += nameHash << 8;
                fixedIV = partyData[i].iv * MAX_PER_STAT_IVS / 255;
                CreateMon(&party[i], partyData[i].species, partyData[i].lvl, fixedIV, TRUE, personalityValue, OT_ID_RANDOM_NO_SHINY, 0);

                SetMonData(&party[i], MON_DATA_HELD_ITEM, &partyData[i].heldItem);
                break;
            }
            case F_TRAINER_PARTY_CUSTOM_MOVESET | F_TRAINER_PARTY_HELD_ITEM:
            {
                const struct TrainerMonItemCustomMoves *partyData = gTrainers[trainerNum].party.ItemCustomMoves;

                for (j = 0; gSpeciesNames[partyData[i].species][j] != EOS; j++)
                    nameHash += gSpeciesNames[partyData[i].species][j];

                personalityValue += nameHash << 8;
                fixedIV = partyData[i].iv * MAX_PER_STAT_IVS / 255;
                CreateMon(&party[i], partyData[i].species, partyData[i].lvl, fixedIV, TRUE, personalityValue, OT_ID_RANDOM_NO_SHINY, 0);

                SetMonData(&party[i], MON_DATA_HELD_ITEM, &partyData[i].heldItem);

                for (j = 0; j < MAX_MON_MOVES; j++)
                {
                    SetMonData(&party[i], MON_DATA_MOVE1 + j, &partyData[i].moves[j]);
                    SetMonData(&party[i], MON_DATA_PP1 + j, &gBattleMoves[partyData[i].moves[j]].pp);
                }
                break;
            }
            }
        }

        gBattleTypeFlags |= gTrainers[trainerNum].doubleBattle;
    }

    return gTrainers[trainerNum].partySize;
}

void sub_8038A04(void) // unused
{
    if (REG_VCOUNT < 0xA0 && REG_VCOUNT >= 0x6F)
        SetGpuReg(REG_OFFSET_BG0CNT, BGCNT_SCREENBASE(24) | BGCNT_TXT256x512);
}

void VBlankCB_Battle(void)
{
    // Change gRngSeed every vblank unless the battle could be recorded.
    if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_RECORDED)))
        Random();

    SetGpuReg(REG_OFFSET_BG0HOFS, gBattle_BG0_X);
    SetGpuReg(REG_OFFSET_BG0VOFS, gBattle_BG0_Y);
    SetGpuReg(REG_OFFSET_BG1HOFS, gBattle_BG1_X);
    SetGpuReg(REG_OFFSET_BG1VOFS, gBattle_BG1_Y);
    SetGpuReg(REG_OFFSET_BG2HOFS, gBattle_BG2_X);
    SetGpuReg(REG_OFFSET_BG2VOFS, gBattle_BG2_Y);
    SetGpuReg(REG_OFFSET_BG3HOFS, gBattle_BG3_X);
    SetGpuReg(REG_OFFSET_BG3VOFS, gBattle_BG3_Y);
    SetGpuReg(REG_OFFSET_WIN0H, gBattle_WIN0H);
    SetGpuReg(REG_OFFSET_WIN0V, gBattle_WIN0V);
    SetGpuReg(REG_OFFSET_WIN1H, gBattle_WIN1H);
    SetGpuReg(REG_OFFSET_WIN1V, gBattle_WIN1V);
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
    ScanlineEffect_InitHBlankDmaTransfer();
}

void SpriteCB_VsLetterDummy(struct Sprite *sprite)
{

}

static void SpriteCB_VsLetter(struct Sprite *sprite)
{
    if (sprite->data[0] != 0)
        sprite->pos1.x = sprite->data[1] + ((sprite->data[2] & 0xFF00) >> 8);
    else
        sprite->pos1.x = sprite->data[1] - ((sprite->data[2] & 0xFF00) >> 8);

    sprite->data[2] += 0x180;

    if (sprite->affineAnimEnded)
    {
        FreeSpriteTilesByTag(ANIM_SPRITES_START);
        FreeSpritePaletteByTag(ANIM_SPRITES_START);
        FreeSpriteOamMatrix(sprite);
        DestroySprite(sprite);
    }
}

void SpriteCB_VsLetterInit(struct Sprite *sprite)
{
    StartSpriteAffineAnim(sprite, 1);
    sprite->callback = SpriteCB_VsLetter;
    PlaySE(SE_MUGSHOT);
}

static void BufferPartyVsScreenHealth_AtEnd(u8 taskId)
{
    struct Pokemon *party1 = NULL;
    struct Pokemon *party2 = NULL;
    u8 multiplayerId = gBattleScripting.multiplayerId;
    u32 flags;
    s32 i;

    if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
    {
        switch (gLinkPlayers[multiplayerId].id)
        {
        case 0:
        case 2:
            party1 = gPlayerParty;
            party2 = gEnemyParty;
            break;
        case 1:
        case 3:
            party1 = gEnemyParty;
            party2 = gPlayerParty;
            break;
        }
    }
    else
    {
        party1 = gPlayerParty;
        party2 = gEnemyParty;
    }

    flags = 0;
    BUFFER_PARTY_VS_SCREEN_STATUS(party1, flags, i);
    gTasks[taskId].data[3] = flags;

    flags = 0;
    BUFFER_PARTY_VS_SCREEN_STATUS(party2, flags, i);
    gTasks[taskId].data[4] = flags;
}

void CB2_InitEndLinkBattle(void)
{
    s32 i;
    u8 taskId;

    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);
    gBattleTypeFlags &= ~(BATTLE_TYPE_LINK_IN_BATTLE);

    if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
    {
        SetMainCallback2(gMain.savedCallback);
        FreeBattleResources();
        FreeBattleSpritesData();
        FreeMonSpritesGfx();
    }
    else
    {
        CpuFill32(0, (void*)(VRAM), VRAM_SIZE);
        SetGpuReg(REG_OFFSET_MOSAIC, 0);
        SetGpuReg(REG_OFFSET_WIN0H, DISPLAY_WIDTH);
        SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 2 + 1));
        SetGpuReg(REG_OFFSET_WININ, 0);
        SetGpuReg(REG_OFFSET_WINOUT, 0);
        gBattle_WIN0H = DISPLAY_WIDTH;
        gBattle_WIN0V = WIN_RANGE(DISPLAY_HEIGHT / 2, DISPLAY_HEIGHT / 2 + 1);
        ScanlineEffect_Clear();

        i = 0;
        while (i < 80)
        {
            gScanlineEffectRegBuffers[0][i] = 0xF0;
            gScanlineEffectRegBuffers[1][i] = 0xF0;
            i++;
        }

        while (i < 160)
        {
            gScanlineEffectRegBuffers[0][i] = 0xFF10;
            gScanlineEffectRegBuffers[1][i] = 0xFF10;
            i++;
        }

        ResetPaletteFade();

        gBattle_BG0_X = 0;
        gBattle_BG0_Y = 0;
        gBattle_BG1_X = 0;
        gBattle_BG1_Y = 0;
        gBattle_BG2_X = 0;
        gBattle_BG2_Y = 0;
        gBattle_BG3_X = 0;
        gBattle_BG3_Y = 0;

        InitBattleBgsVideo();
        LoadCompressedPalette(gBattleTextboxPalette, 0, 64);
        LoadBattleMenuWindowGfx();
        ResetSpriteData();
        ResetTasks();
        DrawBattleEntryBackground();
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR);
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 4;
        SetVBlankCallback(VBlankCB_Battle);

        // Show end Vs screen with battle results
        taskId = CreateTask(InitLinkBattleVsScreen, 0);
        gTasks[taskId].data[1] = 0x10E;
        gTasks[taskId].data[2] = 0x5A;
        gTasks[taskId].data[5] = 1;
        BufferPartyVsScreenHealth_AtEnd(taskId);

        SetMainCallback2(CB2_EndLinkBattle);
        gBattleCommunication[MULTIUSE_STATE] = 0;
    }
}

static void CB2_EndLinkBattle(void)
{
    EndLinkBattleInSteps();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    RunTasks();
}

static void EndLinkBattleInSteps(void)
{
    s32 i;

    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        gBattleCommunication[1] = 0xFF;
        gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 1:
        if (--gBattleCommunication[1] == 0)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 2:
        if (!gPaletteFade.active)
        {
            u8 monsCount;

            gMain.anyLinkBattlerHasFrontierPass = RecordedBattle_GetFrontierPassFlag();

            if (gBattleTypeFlags & BATTLE_TYPE_MULTI)
                monsCount = 4;
            else
                monsCount = 2;

            for (i = 0; i < monsCount && (gLinkPlayers[i].version & 0xFF) == VERSION_EMERALD; i++);

            if (!gSaveBlock2Ptr->frontier.disableRecordBattle && i == monsCount)
            {
                if (FlagGet(FLAG_SYS_FRONTIER_PASS))
                {
                    FreeAllWindowBuffers();
                    SetMainCallback2(sub_80392A8);
                }
                else if (!gMain.anyLinkBattlerHasFrontierPass)
                {
                    SetMainCallback2(gMain.savedCallback);
                    FreeBattleResources();
                    FreeBattleSpritesData();
                    FreeMonSpritesGfx();
                }
                else if (gReceivedRemoteLinkPlayers == 0)
                {
                    CreateTask(Task_ReconnectWithLinkPlayers, 5);
                    gBattleCommunication[MULTIUSE_STATE]++;
                }
                else
                {
                    gBattleCommunication[MULTIUSE_STATE]++;
                }
            }
            else
            {
                SetMainCallback2(gMain.savedCallback);
                FreeBattleResources();
                FreeBattleSpritesData();
                FreeMonSpritesGfx();
            }
        }
        break;
    case 3:
        CpuFill32(0, (void*)(VRAM), VRAM_SIZE);

        for (i = 0; i < 2; i++)
            LoadChosenBattleElement(i);

        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 4:
        if (!gPaletteFade.active)
            gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 5:
        if (!FuncIsActiveTask(Task_ReconnectWithLinkPlayers))
            gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 6:
        if (IsLinkTaskFinished() == TRUE)
        {
            SetLinkStandbyCallback();
            BattlePutTextOnWindow(gText_LinkStandby3, 0);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 7:
        if (!IsTextPrinterActive(0))
        {
            if (IsLinkTaskFinished() == TRUE)
                gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 8:
        if (!gWirelessCommType)
            SetCloseLinkCallback();
        gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 9:
        if (!gMain.anyLinkBattlerHasFrontierPass || gWirelessCommType || gReceivedRemoteLinkPlayers != 1)
        {
            gMain.anyLinkBattlerHasFrontierPass = 0;
            SetMainCallback2(gMain.savedCallback);
            FreeBattleResources();
            FreeBattleSpritesData();
            FreeMonSpritesGfx();
        }
        break;
    }
}

u32 GetBattleBgTemplateData(u8 arrayId, u8 caseId)
{
    u32 ret = 0;

    switch (caseId)
    {
    case 0:
        ret = gBattleBgTemplates[arrayId].bg;
        break;
    case 1:
        ret = gBattleBgTemplates[arrayId].charBaseIndex;
        break;
    case 2:
        ret = gBattleBgTemplates[arrayId].mapBaseIndex;
        break;
    case 3:
        ret = gBattleBgTemplates[arrayId].screenSize;
        break;
    case 4:
        ret = gBattleBgTemplates[arrayId].paletteMode;
        break;
    case 5: // Only this case is used
        ret = gBattleBgTemplates[arrayId].priority;
        break;
    case 6:
        ret = gBattleBgTemplates[arrayId].baseTile;
        break;
    }

    return ret;
}

static void sub_80392A8(void)
{
    s32 i;

    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);
    CpuFill32(0, (void*)(VRAM), VRAM_SIZE);
    ResetPaletteFade();
    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    gBattle_BG1_X = 0;
    gBattle_BG1_Y = 0;
    gBattle_BG2_X = 0;
    gBattle_BG2_Y = 0;
    gBattle_BG3_X = 0;
    gBattle_BG3_Y = 0;
    InitBattleBgsVideo();
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    LoadBattleMenuWindowGfx();

    for (i = 0; i < 2; i++)
        LoadChosenBattleElement(i);

    ResetSpriteData();
    ResetTasks();
    FreeAllSpritePalettes();
    gReservedSpritePaletteCount = 4;
    SetVBlankCallback(VBlankCB_Battle);
    SetMainCallback2(sub_803937C);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
    gBattleCommunication[MULTIUSE_STATE] = 0;
}

static void sub_803937C(void)
{
    sub_803939C();
    AnimateSprites();
    BuildOamBuffer();
    RunTextPrinters();
    UpdatePaletteFade();
    RunTasks();
}

static void sub_803939C(void)
{
    switch (gBattleCommunication[MULTIUSE_STATE])
    {
    case 0:
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 1:
        if (gMain.anyLinkBattlerHasFrontierPass && gReceivedRemoteLinkPlayers == 0)
            CreateTask(Task_ReconnectWithLinkPlayers, 5);
        gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 2:
        if (!FuncIsActiveTask(Task_ReconnectWithLinkPlayers))
            gBattleCommunication[MULTIUSE_STATE]++;
        break;
    case 3:
        if (!gPaletteFade.active)
        {
            BattlePutTextOnWindow(gText_RecordBattleToPass, 0);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 4:
        if (!IsTextPrinterActive(0))
        {
            HandleBattleWindow(0x18, 8, 0x1D, 0xD, 0);
            BattlePutTextOnWindow(gText_BattleYesNoChoice, 0xC);
            gBattleCommunication[CURSOR_POSITION] = 1;
            BattleCreateYesNoCursorAt(1);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 5:
        if (JOY_NEW(DPAD_UP))
        {
            if (gBattleCommunication[CURSOR_POSITION] != 0)
            {
                PlaySE(SE_SELECT);
                BattleDestroyYesNoCursorAt(gBattleCommunication[CURSOR_POSITION]);
                gBattleCommunication[CURSOR_POSITION] = 0;
                BattleCreateYesNoCursorAt(0);
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (gBattleCommunication[CURSOR_POSITION] == 0)
            {
                PlaySE(SE_SELECT);
                BattleDestroyYesNoCursorAt(gBattleCommunication[CURSOR_POSITION]);
                gBattleCommunication[CURSOR_POSITION] = 1;
                BattleCreateYesNoCursorAt(1);
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            if (gBattleCommunication[CURSOR_POSITION] == 0)
            {
                HandleBattleWindow(0x18, 8, 0x1D, 0xD, WINDOW_CLEAR);
                gBattleCommunication[1] = MoveRecordedBattleToSaveData();
                gBattleCommunication[MULTIUSE_STATE] = 10;
            }
            else
            {
                gBattleCommunication[MULTIUSE_STATE]++;
            }
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 6:
        if (IsLinkTaskFinished() == TRUE)
        {
            HandleBattleWindow(0x18, 8, 0x1D, 0xD, WINDOW_CLEAR);
            if (gMain.anyLinkBattlerHasFrontierPass)
            {
                SetLinkStandbyCallback();
                BattlePutTextOnWindow(gText_LinkStandby3, 0);
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 8:
        if (--gBattleCommunication[1] == 0)
        {
            if (gMain.anyLinkBattlerHasFrontierPass && !gWirelessCommType)
                SetCloseLinkCallback();
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 9:
        if (!gMain.anyLinkBattlerHasFrontierPass || gWirelessCommType || gReceivedRemoteLinkPlayers != 1)
        {
            gMain.anyLinkBattlerHasFrontierPass = 0;
            if (!gPaletteFade.active)
            {
                SetMainCallback2(gMain.savedCallback);
                FreeBattleResources();
                FreeBattleSpritesData();
                FreeMonSpritesGfx();
            }
        }
        break;
    case 10:
        if (gBattleCommunication[1] == 1)
        {
            PlaySE(SE_SAVE);
            BattleStringExpandPlaceholdersToDisplayedString(gText_BattleRecordedOnPass);
            BattlePutTextOnWindow(gDisplayedStringBattle, 0);
            gBattleCommunication[1] = 0x80;
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        else
        {
            BattleStringExpandPlaceholdersToDisplayedString(BattleFrontier_BattleTowerBattleRoom_Text_RecordCouldntBeSaved);
            BattlePutTextOnWindow(gDisplayedStringBattle, 0);
            gBattleCommunication[1] = 0x80;
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 11:
        if (IsLinkTaskFinished() == TRUE && !IsTextPrinterActive(0) && --gBattleCommunication[1] == 0)
        {
            if (gMain.anyLinkBattlerHasFrontierPass)
            {
                SetLinkStandbyCallback();
                BattlePutTextOnWindow(gText_LinkStandby3, 0);
            }
            gBattleCommunication[MULTIUSE_STATE]++;
        }
        break;
    case 12:
    case 7:
        if (!IsTextPrinterActive(0))
        {
            if (gMain.anyLinkBattlerHasFrontierPass)
            {
                if (IsLinkTaskFinished() == TRUE)
                {
                    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                    gBattleCommunication[1] = 0x20;
                    gBattleCommunication[MULTIUSE_STATE] = 8;
                }

            }
            else
            {
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                gBattleCommunication[1] = 0x20;
                gBattleCommunication[MULTIUSE_STATE] = 8;
            }
        }
        break;
    }
}

static void TryCorrectShedinjaLanguage(struct Pokemon *mon)
{
    u8 nickname[POKEMON_NAME_LENGTH + 1];
    u8 language = LANGUAGE_JAPANESE;

    if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_SHEDINJA
     && GetMonData(mon, MON_DATA_LANGUAGE) != language)
    {
        GetMonData(mon, MON_DATA_NICKNAME, nickname);
        if (StringCompareWithoutExtCtrlCodes(nickname, sText_ShedinjaJpnName) == 0)
            SetMonData(mon, MON_DATA_LANGUAGE, &language);
    }
}

u32 GetBattleWindowTemplatePixelWidth(u32 setId, u32 tableId)
{
    return gBattleWindowTemplates[setId][tableId].width * 8;
}

#define sBattler            data[0]
#define sSpeciesId          data[2]

void SpriteCb_WildMon(struct Sprite *sprite)
{
    sprite->callback = SpriteCb_MoveWildMonToRight;
    StartSpriteAnimIfDifferent(sprite, 0);
    if (WILD_DOUBLE_BATTLE)
        BeginNormalPaletteFade((0x10000 << sprite->sBattler) | (0x10000 << BATTLE_PARTNER(sprite->sBattler)), 0, 10, 10, RGB(8, 8, 8));
    else
        BeginNormalPaletteFade((0x10000 << sprite->sBattler), 0, 10, 10, RGB(8, 8, 8));
}

static void SpriteCb_MoveWildMonToRight(struct Sprite *sprite)
{
    if ((gIntroSlideFlags & 1) == 0)
    {
        sprite->pos2.x += 3; // x1.5x SPEEDCHOICE
        if (sprite->pos2.x == 0)
        {
            sprite->callback = SpriteCb_WildMonShowHealthbox;
        }
    }
}

static void SpriteCb_WildMonShowHealthbox(struct Sprite *sprite)
{
    if (sprite->animEnded)
    {
        StartHealthboxSlideIn(sprite->sBattler);
        SetHealthboxSpriteVisible(gHealthboxSpriteIds[sprite->sBattler]);
        sprite->callback = SpriteCb_WildMonAnimate;
        StartSpriteAnimIfDifferent(sprite, 0);
        if (WILD_DOUBLE_BATTLE)
            BeginNormalPaletteFade((0x10000 << sprite->sBattler) | (0x10000 << BATTLE_PARTNER(sprite->sBattler)), 0, 10, 0, RGB(8, 8, 8));
        else
            BeginNormalPaletteFade((0x10000 << sprite->sBattler), 0, 10, 0, RGB(8, 8, 8));
    }
}

static void SpriteCb_WildMonAnimate(struct Sprite *sprite)
{
    if (!gPaletteFade.active)
    {
        BattleAnimateFrontSprite(sprite, sprite->sSpeciesId, FALSE, 1);
    }
}

void SpriteCallbackDummy_2(struct Sprite *sprite)
{

}

static void sub_80398D0(struct Sprite *sprite)
{
    sprite->data[4]--;
    if (sprite->data[4] == 0)
    {
        sprite->data[4] = 8;
        sprite->invisible ^= 1;
        sprite->data[3]--;
        if (sprite->data[3] == 0)
        {
            sprite->invisible = FALSE;
            sprite->callback = SpriteCallbackDummy_2;
            // sUnusedUnknownArray[0] = 0;
        }
    }
}

extern const struct MonCoords gMonFrontPicCoords[];
extern const struct MonCoords gCastformFrontSpriteCoords[];

void SpriteCB_FaintOpponentMon(struct Sprite *sprite)
{
    u8 battler = sprite->sBattler;
    u32 personality = GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_PERSONALITY);
    u16 species;
    u8 yOffset;

    if (gBattleSpritesDataPtr->battlerData[battler].transformSpecies != 0)
        species = gBattleSpritesDataPtr->battlerData[battler].transformSpecies;
    else
        species = sprite->sSpeciesId;

    GetMonData(&gEnemyParty[gBattlerPartyIndexes[battler]], MON_DATA_PERSONALITY);  // Unused return value.

    if (species == SPECIES_UNOWN)
    {
        species = GetUnownSpeciesId(personality);
        yOffset = gMonFrontPicCoords[species].y_offset;
    }
    else if (species == SPECIES_CASTFORM)
    {
        yOffset = gCastformFrontSpriteCoords[gBattleMonForms[battler]].y_offset;
    }
    else if (species > NUM_SPECIES)
    {
        yOffset = gMonFrontPicCoords[SPECIES_NONE].y_offset;
    }
    else
    {
        yOffset = gMonFrontPicCoords[species].y_offset;
    }

    sprite->data[3] = 8 - yOffset / 8;
    sprite->data[4] = 1;
    sprite->callback = SpriteCB_AnimFaintOpponent;
}

static void SpriteCB_AnimFaintOpponent(struct Sprite *sprite)
{
    s32 i;

    if (--sprite->data[4] == 0)
    {
        sprite->data[4] = 2;
        sprite->pos2.y += 8; // Move the sprite down.
        if (--sprite->data[3] < 0)
        {
            FreeSpriteOamMatrix(sprite);
            DestroySprite(sprite);
        }
        else // Erase bottom part of the sprite to create a smooth illusion of mon falling down.
        {
            u8* dst = gMonSpritesGfxPtr->sprites.byte[GetBattlerPosition(sprite->sBattler)] + (gBattleMonForms[sprite->sBattler] << 11) + (sprite->data[3] << 8);

            for (i = 0; i < 0x100; i++)
                *(dst++) = 0;

            StartSpriteAnim(sprite, gBattleMonForms[sprite->sBattler]);
        }
    }
}

// Used when selecting a move, which can hit multiple targets, in double battles.
void SpriteCb_ShowAsMoveTarget(struct Sprite *sprite)
{
    sprite->data[3] = 8;
    sprite->data[4] = sprite->invisible;
    sprite->callback = SpriteCb_BlinkVisible;
}

static void SpriteCb_BlinkVisible(struct Sprite *sprite)
{
    if (--sprite->data[3] == 0)
    {
        sprite->invisible ^= 1;
        sprite->data[3] = 8;
    }
}

void SpriteCb_HideAsMoveTarget(struct Sprite *sprite)
{
    sprite->invisible = sprite->data[4];
    sprite->data[4] = FALSE;
    sprite->callback = SpriteCallbackDummy_2;
}

void SpriteCb_OpponentMonFromBall(struct Sprite *sprite)
{
    if (sprite->affineAnimEnded)
    {
        if (!(gHitMarker & HITMARKER_NO_ANIMATIONS) || gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
        {
            if (HasTwoFramesAnimation(sprite->sSpeciesId))
                StartSpriteAnim(sprite, 1);
        }
        BattleAnimateFrontSprite(sprite, sprite->sSpeciesId, TRUE, 1);
    }
}

// This callback is frequently overwritten by SpriteCB_TrainerSlideIn
void SpriteCB_BattleSpriteStartSlideLeft(struct Sprite *sprite)
{
    sprite->callback = SpriteCB_BattleSpriteSlideLeft;
}

static void SpriteCB_BattleSpriteSlideLeft(struct Sprite *sprite)
{
    if (!(gIntroSlideFlags & 1))
    {
        sprite->pos2.x -= 2;
        if (sprite->pos2.x == 0)
        {
            sprite->callback = SpriteCallbackDummy_3;
            sprite->data[1] = 0;
        }
    }
}

// Unused
static void sub_80105DC(struct Sprite *sprite)
{
    sprite->callback = SpriteCallbackDummy_3;
}

static void SpriteCallbackDummy_3(struct Sprite *sprite)
{
}

#define sSpeedX data[1]
#define sSpeedY data[2]

void SpriteCB_FaintSlideAnim(struct Sprite *sprite)
{
    if (!(gIntroSlideFlags & 1))
    {
        sprite->pos2.x += sprite->sSpeedX;
        sprite->pos2.y += sprite->sSpeedY;
    }
}

#undef sSpeedX
#undef sSpeedY

#define sSinIndex           data[3]
#define sDelta              data[4]
#define sAmplitude          data[5]
#define sBouncerSpriteId    data[6]
#define sWhich              data[7]

void DoBounceEffect(u8 battler, u8 which, s8 delta, s8 amplitude)
{
    u8 invisibleSpriteId;
    u8 bouncerSpriteId;

    switch (which)
    {
    case BOUNCE_HEALTHBOX:
    default:
        if (gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing)
            return;
        break;
    case BOUNCE_MON:
        if (gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing)
            return;
        break;
    }

    invisibleSpriteId = CreateInvisibleSpriteWithCallback(SpriteCB_BounceEffect);
    if (which == BOUNCE_HEALTHBOX)
    {
        bouncerSpriteId = gHealthboxSpriteIds[battler];
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxBounceSpriteId = invisibleSpriteId;
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing = 1;
        gSprites[invisibleSpriteId].sSinIndex = 128; // 0
    }
    else
    {
        bouncerSpriteId = gBattlerSpriteIds[battler];
        gBattleSpritesDataPtr->healthBoxesData[battler].battlerBounceSpriteId = invisibleSpriteId;
        gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing = 1;
        gSprites[invisibleSpriteId].sSinIndex = 192; // -1
    }
    gSprites[invisibleSpriteId].sDelta = delta;
    gSprites[invisibleSpriteId].sAmplitude = amplitude;
    gSprites[invisibleSpriteId].sBouncerSpriteId = bouncerSpriteId;
    gSprites[invisibleSpriteId].sWhich = which;
    gSprites[invisibleSpriteId].sBattler = battler;
    gSprites[bouncerSpriteId].pos2.x = 0;
    gSprites[bouncerSpriteId].pos2.y = 0;
}

void EndBounceEffect(u8 battler, u8 which)
{
    u8 bouncerSpriteId;

    if (which == BOUNCE_HEALTHBOX)
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing)
            return;

        bouncerSpriteId = gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].healthboxBounceSpriteId].sBouncerSpriteId;
        DestroySprite(&gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].healthboxBounceSpriteId]);
        gBattleSpritesDataPtr->healthBoxesData[battler].healthboxIsBouncing = 0;
    }
    else
    {
        if (!gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing)
            return;

        bouncerSpriteId = gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].battlerBounceSpriteId].sBouncerSpriteId;
        DestroySprite(&gSprites[gBattleSpritesDataPtr->healthBoxesData[battler].battlerBounceSpriteId]);
        gBattleSpritesDataPtr->healthBoxesData[battler].battlerIsBouncing = 0;
    }

    gSprites[bouncerSpriteId].pos2.x = 0;
    gSprites[bouncerSpriteId].pos2.y = 0;
}

static void SpriteCB_BounceEffect(struct Sprite *sprite)
{
    u8 bouncerSpriteId = sprite->sBouncerSpriteId;
    s32 index = sprite->sSinIndex;
    s32 y = Sin(index, sprite->sAmplitude) + sprite->sAmplitude;

    gSprites[bouncerSpriteId].pos2.y = y;
    sprite->sSinIndex = (sprite->sSinIndex + sprite->sDelta) & 0xFF;

    bouncerSpriteId = GetMegaIndicatorSpriteId(sprite->sBouncerSpriteId);
    if (sprite->sWhich == BOUNCE_HEALTHBOX && bouncerSpriteId != 0xFF)
        gSprites[bouncerSpriteId].pos2.y = y;
}

#undef sSinIndex
#undef sDelta
#undef sAmplitude
#undef sBouncerSpriteId
#undef sWhich

void SpriteCb_PlayerMonFromBall(struct Sprite *sprite)
{
    if (sprite->affineAnimEnded)
        BattleAnimateBackSprite(sprite, sprite->sSpeciesId);
}

void sub_8039E60(struct Sprite *sprite)
{
    sub_8039E9C(sprite);
    if (sprite->animEnded)
        sprite->callback = SpriteCallbackDummy_3;
}

void SpriteCB_TrainerThrowObject(struct Sprite *sprite)
{
    StartSpriteAnim(sprite, 1);
    sprite->callback = sub_8039E60;
}

void sub_8039E9C(struct Sprite *sprite)
{
    if (sprite->animDelayCounter == 0)
        sprite->centerToCornerVecX = gUnknown_0831ACE0[sprite->animCmdIndex];
}

void BeginBattleIntroDummy(void)
{

}

void BeginBattleIntro(void)
{
    BattleStartClearSetData();
    gBattleCommunication[1] = 0;
    gBattleStruct->introState = 0;
    gBattleMainFunc = DoBattleIntro;
}

#define BATTLE_SPEED 3
extern void OpponentHandleHealthBarUpdate(void); // opponent HP Bar (battle_7)
extern void PlayerHandleHealthBarUpdate(void); // player HP bar
extern void CompleteOnHealthbarDone(void);
extern void CompleteOnHealthbarDone2(void);

static void BattleMainCB1(void)
{
    u8 i;
    gBattleMainFunc();

    for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
	{
		if(gBattlerControllerFuncs[gActiveBattler] == OpponentHandleHealthBarUpdate ||
		gBattlerControllerFuncs[gActiveBattler] == PlayerHandleHealthBarUpdate || 
		gBattlerControllerFuncs[gActiveBattler] == CompleteOnHealthbarDone || 
		gBattlerControllerFuncs[gActiveBattler] == CompleteOnHealthbarDone2)
		{
			for(i = 0; i < BATTLE_SPEED; i++)
				gBattlerControllerFuncs[gActiveBattler]();
		}
		else
			gBattlerControllerFuncs[gActiveBattler]();
	}
}

static void BattleStartClearSetData(void)
{
    s32 i;

    TurnValuesCleanUp(FALSE);
    SpecialStatusesClear();

    memset(&gDisableStructs, 0, sizeof(gDisableStructs));
    memset(&gFieldTimers, 0, sizeof(gFieldTimers));
    memset(&gSideStatuses, 0, sizeof(gSideStatuses));
    memset(&gSideTimers, 0, sizeof(gSideTimers));
    memset(&gWishFutureKnock, 0, sizeof(gWishFutureKnock));
    memset(&gBattleResults, 0, sizeof(gBattleResults));

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        gStatuses3[i] = 0;
        gDisableStructs[i].isFirstTurn = 2;
        gLastMoves[i] = 0;
        gLastLandedMoves[i] = 0;
        gLastHitByType[i] = 0;
        gLastResultingMoves[i] = 0;
        gLastHitBy[i] = 0xFF;
        gLockedMoves[i] = 0;
        gLastPrintedMoves[i] = 0;
        gBattleResources->flags->flags[i] = 0;
        gPalaceSelectionBattleScripts[i] = 0;
        gBattleStruct->lastTakenMove[i] = 0;
        gBattleStruct->usedHeldItems[i] = 0;
        gBattleStruct->choicedMove[i] = 0;
        gBattleStruct->changedItems[i] = 0;
        gBattleStruct->lastTakenMoveFrom[i][0] = 0;
        gBattleStruct->lastTakenMoveFrom[i][1] = 0;
        gBattleStruct->lastTakenMoveFrom[i][2] = 0;
        gBattleStruct->lastTakenMoveFrom[i][3] = 0;
        gBattleStruct->AI_monToSwitchIntoId[i] = PARTY_SIZE;
    }

    gLastUsedMove = 0;
    gFieldStatuses = 0;

    gHasFetchedBall = FALSE;
    gLastUsedBall = 0;

    gBattlerAttacker = 0;
    gBattlerTarget = 0;
    gBattleWeather = 0;
    gHitMarker = 0;

    if (!(gBattleTypeFlags & BATTLE_TYPE_RECORDED))
    {
        if (!(gBattleTypeFlags & BATTLE_TYPE_LINK) && gSaveBlock2Ptr->optionsBattleSceneOff == TRUE)
            gHitMarker |= HITMARKER_NO_ANIMATIONS;
    }
    else if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK)) && GetBattleSceneInRecordedBattle())
    {
        gHitMarker |= HITMARKER_NO_ANIMATIONS;
    }

    gBattleScripting.battleStyle = gSaveBlock2Ptr->optionsBattleStyle;
	gBattleScripting.expOnCatch = (B_EXP_CATCH >= GEN_6);
	gBattleScripting.monCaught = FALSE;

    gMultiHitCounter = 0;
    gBattleOutcome = 0;
    gBattleControllerExecFlags = 0;
    gPaydayMoney = 0;
    gBattleResources->battleScriptsStack->size = 0;
    gBattleResources->battleCallbackStack->size = 0;

    for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
        gBattleCommunication[i] = 0;

    gPauseCounterBattle = 0;
    gBattleMoveDamage = 0;
    gIntroSlideFlags = 0;
    gBattleScripting.animTurn = 0;
    gBattleScripting.animTargetsHit = 0;
    gLeveledUpInBattle = 0;
    gAbsentBattlerFlags = 0;
    gBattleStruct->runTries = 0;
    gBattleStruct->safariGoNearCounter = 0;
    gBattleStruct->safariPkblThrowCounter = 0;
    gBattleStruct->safariCatchFactor = gBaseStats[GetMonData(&gEnemyParty[0], MON_DATA_SPECIES)].catchRate * 100 / 1275;
    gBattleStruct->safariEscapeFactor = 3;
    gBattleStruct->wildVictorySong = 0;
    gBattleStruct->moneyMultiplier = 1;

    gBattleStruct->givenExpMons = 0;
    gBattleStruct->palaceFlags = 0;

    gRandomTurnNumber = Random();

    gBattleResults.shinyWildMon = IsMonShiny(&gEnemyParty[0]);

    gBattleStruct->arenaLostPlayerMons = 0;
    gBattleStruct->arenaLostOpponentMons = 0;

    gBattleStruct->mega.triggerSpriteId = 0xFF;
}

void SwitchInClearSetData(void)
{
    s32 i;
    struct DisableStruct disableStructCopy = gDisableStructs[gActiveBattler];

    ClearIllusionMon(gActiveBattler);
    if (gBattleMoves[gCurrentMove].effect != EFFECT_BATON_PASS)
    {
        for (i = 0; i < NUM_BATTLE_STATS; i++)
            gBattleMons[gActiveBattler].statStages[i] = DEFAULT_STAT_STAGE;
        for (i = 0; i < gBattlersCount; i++)
        {
            if ((gBattleMons[i].status2 & STATUS2_ESCAPE_PREVENTION) && gDisableStructs[i].battlerPreventingEscape == gActiveBattler)
                gBattleMons[i].status2 &= ~STATUS2_ESCAPE_PREVENTION;
            if ((gStatuses3[i] & STATUS3_ALWAYS_HITS) && gDisableStructs[i].battlerWithSureHit == gActiveBattler)
            {
                gStatuses3[i] &= ~STATUS3_ALWAYS_HITS;
                gDisableStructs[i].battlerWithSureHit = 0;
            }
        }
    }
    if (gBattleMoves[gCurrentMove].effect == EFFECT_BATON_PASS)
    {
        gBattleMons[gActiveBattler].status2 &= (STATUS2_CONFUSION | STATUS2_FOCUS_ENERGY | STATUS2_SUBSTITUTE | STATUS2_ESCAPE_PREVENTION | STATUS2_CURSED);
        gStatuses3[gActiveBattler] &= (STATUS3_LEECHSEED_BATTLER | STATUS3_LEECHSEED | STATUS3_ALWAYS_HITS | STATUS3_PERISH_SONG | STATUS3_ROOTED);

        for (i = 0; i < gBattlersCount; i++)
        {
            if (GetBattlerSide(gActiveBattler) != GetBattlerSide(i)
             && (gStatuses3[i] & STATUS3_ALWAYS_HITS) != 0
             && (gDisableStructs[i].battlerWithSureHit == gActiveBattler))
            {
                gStatuses3[i] &= ~(STATUS3_ALWAYS_HITS);
                gStatuses3[i] |= STATUS3_ALWAYS_HITS_TURN(2);
            }
        }
        if (gStatuses3[gActiveBattler] & STATUS3_POWER_TRICK)
            SWAP(gBattleMons[gActiveBattler].attack, gBattleMons[gActiveBattler].defense, i);
    }
    else
    {
        gBattleMons[gActiveBattler].status2 = 0;
        gStatuses3[gActiveBattler] = 0;
    }

    for (i = 0; i < gBattlersCount; i++)
    {
        if (gBattleMons[i].status2 & STATUS2_INFATUATED_WITH(gActiveBattler))
            gBattleMons[i].status2 &= ~(STATUS2_INFATUATED_WITH(gActiveBattler));
        if ((gBattleMons[i].status2 & STATUS2_WRAPPED) && *(gBattleStruct->wrappedBy + i) == gActiveBattler)
            gBattleMons[i].status2 &= ~(STATUS2_WRAPPED);
    }

    gActionSelectionCursor[gActiveBattler] = 0;
    gMoveSelectionCursor[gActiveBattler] = 0;

    memset(&gDisableStructs[gActiveBattler], 0, sizeof(struct DisableStruct));

    if (gBattleMoves[gCurrentMove].effect == EFFECT_BATON_PASS)
    {
        gDisableStructs[gActiveBattler].substituteHP = disableStructCopy.substituteHP;
        gDisableStructs[gActiveBattler].battlerWithSureHit = disableStructCopy.battlerWithSureHit;
        gDisableStructs[gActiveBattler].perishSongTimer = disableStructCopy.perishSongTimer;
        gDisableStructs[gActiveBattler].perishSongTimerStartValue = disableStructCopy.perishSongTimerStartValue;
        gDisableStructs[gActiveBattler].battlerPreventingEscape = disableStructCopy.battlerPreventingEscape;
    }

    gMoveResultFlags = 0;
    gDisableStructs[gActiveBattler].isFirstTurn = 2;
    gDisableStructs[gActiveBattler].truantSwitchInHack = disableStructCopy.truantSwitchInHack;
    gLastMoves[gActiveBattler] = 0;
    gLastLandedMoves[gActiveBattler] = 0;
    gLastHitByType[gActiveBattler] = 0;
    gLastResultingMoves[gActiveBattler] = 0;
    gLastPrintedMoves[gActiveBattler] = 0;
    gLastHitBy[gActiveBattler] = 0xFF;

    gBattleStruct->lastTakenMove[gActiveBattler] = 0;
    gBattleStruct->sameMoveTurns[gActiveBattler] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][0] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][1] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][2] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][3] = 0;
    gBattleStruct->lastMoveFailed &= ~(gBitTable[gActiveBattler]);
    gBattleStruct->palaceFlags &= ~(gBitTable[gActiveBattler]);

    for (i = 0; i < gBattlersCount; i++)
    {
        if (i != gActiveBattler && GetBattlerSide(i) != GetBattlerSide(gActiveBattler))
            gBattleStruct->lastTakenMove[i] = 0;

        gBattleStruct->lastTakenMoveFrom[i][gActiveBattler] = 0;
    }

    gBattleStruct->choicedMove[gActiveBattler] = 0;
    gBattleResources->flags->flags[gActiveBattler] = 0;
    gCurrentMove = 0;
    gBattleStruct->arenaTurnCounter = 0xFF;

    ClearBattlerMoveHistory(gActiveBattler);
    ClearBattlerAbilityHistory(gActiveBattler);
}

void FaintClearSetData(void)
{
    s32 i;

    for (i = 0; i < NUM_BATTLE_STATS; i++)
        gBattleMons[gActiveBattler].statStages[i] = DEFAULT_STAT_STAGE;

    gBattleMons[gActiveBattler].status2 = 0;
    gStatuses3[gActiveBattler] = 0;

    for (i = 0; i < gBattlersCount; i++)
    {
        if ((gBattleMons[i].status2 & STATUS2_ESCAPE_PREVENTION) && gDisableStructs[i].battlerPreventingEscape == gActiveBattler)
            gBattleMons[i].status2 &= ~STATUS2_ESCAPE_PREVENTION;
        if (gBattleMons[i].status2 & STATUS2_INFATUATED_WITH(gActiveBattler))
            gBattleMons[i].status2 &= ~(STATUS2_INFATUATED_WITH(gActiveBattler));
        if ((gBattleMons[i].status2 & STATUS2_WRAPPED) && *(gBattleStruct->wrappedBy + i) == gActiveBattler)
            gBattleMons[i].status2 &= ~(STATUS2_WRAPPED);
    }

    gActionSelectionCursor[gActiveBattler] = 0;
    gMoveSelectionCursor[gActiveBattler] = 0;

    memset(&gDisableStructs[gActiveBattler], 0, sizeof(struct DisableStruct));

    gProtectStructs[gActiveBattler].protected = 0;
    gProtectStructs[gActiveBattler].spikyShielded = 0;
    gProtectStructs[gActiveBattler].kingsShielded = 0;
    gProtectStructs[gActiveBattler].banefulBunkered = 0;
    gProtectStructs[gActiveBattler].endured = 0;
    gProtectStructs[gActiveBattler].noValidMoves = 0;
    gProtectStructs[gActiveBattler].helpingHand = 0;
    gProtectStructs[gActiveBattler].bounceMove = 0;
    gProtectStructs[gActiveBattler].stealMove = 0;
    gProtectStructs[gActiveBattler].prlzImmobility = 0;
    gProtectStructs[gActiveBattler].confusionSelfDmg = 0;
    gProtectStructs[gActiveBattler].targetNotAffected = 0;
    gProtectStructs[gActiveBattler].chargingTurn = 0;
    gProtectStructs[gActiveBattler].fleeFlag = 0;
    gProtectStructs[gActiveBattler].usedImprisonedMove = 0;
    gProtectStructs[gActiveBattler].loveImmobility = 0;
    gProtectStructs[gActiveBattler].usedDisabledMove = 0;
    gProtectStructs[gActiveBattler].usedTauntedMove = 0;
    gProtectStructs[gActiveBattler].flag2Unknown = 0;
    gProtectStructs[gActiveBattler].flinchImmobility = 0;
    gProtectStructs[gActiveBattler].notFirstStrike = 0;
    gProtectStructs[gActiveBattler].usedHealBlockedMove = 0;
    gProtectStructs[gActiveBattler].usesBouncedMove = 0;
    gProtectStructs[gActiveBattler].usedGravityPreventedMove = 0;
    gProtectStructs[gActiveBattler].usedThroatChopPreventedMove = 0;

    gDisableStructs[gActiveBattler].isFirstTurn = 2;

    gLastMoves[gActiveBattler] = 0;
    gLastLandedMoves[gActiveBattler] = 0;
    gLastHitByType[gActiveBattler] = 0;
    gLastResultingMoves[gActiveBattler] = 0;
    gLastPrintedMoves[gActiveBattler] = 0;
    gLastHitBy[gActiveBattler] = 0xFF;

    gBattleStruct->choicedMove[gActiveBattler] = 0;
    gBattleStruct->sameMoveTurns[gActiveBattler] = 0;
    gBattleStruct->lastTakenMove[gActiveBattler] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][0] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][1] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][2] = 0;
    gBattleStruct->lastTakenMoveFrom[gActiveBattler][3] = 0;

    gBattleStruct->palaceFlags &= ~(gBitTable[gActiveBattler]);

    for (i = 0; i < gBattlersCount; i++)
    {
        if (i != gActiveBattler && GetBattlerSide(i) != GetBattlerSide(gActiveBattler))
            gBattleStruct->lastTakenMove[i] = 0;

        gBattleStruct->lastTakenMoveFrom[i][gActiveBattler] = 0;
    }

    gBattleResources->flags->flags[gActiveBattler] = 0;

    gBattleMons[gActiveBattler].type1 = gBaseStats[gBattleMons[gActiveBattler].species].type1;
    gBattleMons[gActiveBattler].type2 = gBaseStats[gBattleMons[gActiveBattler].species].type2;
    gBattleMons[gActiveBattler].type3 = TYPE_MYSTERY;

    ClearBattlerMoveHistory(gActiveBattler);
    ClearBattlerAbilityHistory(gActiveBattler);
    UndoFormChange(gBattlerPartyIndexes[gActiveBattler], GET_BATTLER_SIDE(gActiveBattler));
    if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
        UndoMegaEvolution(gBattlerPartyIndexes[gActiveBattler]);
}

static void DoBattleIntro(void)
{
    s32 i;
    u8 *state = &gBattleStruct->introState;

    switch (*state)
    {
    case 0: // Get Data of all battlers.
        gActiveBattler = gBattleCommunication[1];
        BtlController_EmitGetMonData(0, REQUEST_ALL_BATTLE, 0);
        MarkBattlerForControllerExec(gActiveBattler);
        (*state)++;
        break;
    case 1: // Loop through all battlers.
        if (!gBattleControllerExecFlags)
        {
            if (++gBattleCommunication[1] == gBattlersCount)
                (*state)++;
            else
                *state = 0;
        }
        break;
    case 2: // Start graphical intro slide.
        if (!gBattleControllerExecFlags)
        {
            gActiveBattler = GetBattlerAtPosition(0);
            BtlController_EmitIntroSlide(0, gBattleTerrain);
            MarkBattlerForControllerExec(gActiveBattler);
            gBattleCommunication[0] = 0;
            gBattleCommunication[1] = 0;
            (*state)++;
        }
        break;
    case 3: // Wait for intro slide.
        if (!gBattleControllerExecFlags)
            (*state)++;
        break;
    case 4: // Copy battler data gotten in cases 0 and 1. Draw trainer/mon sprite.
        for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
        {
            if ((gBattleTypeFlags & BATTLE_TYPE_SAFARI) && GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
            {
                memset(&gBattleMons[gActiveBattler], 0, sizeof(struct BattlePokemon));
            }
            else
            {
                memcpy(&gBattleMons[gActiveBattler], &gBattleResources->bufferB[gActiveBattler][4], sizeof(struct BattlePokemon));
                gBattleMons[gActiveBattler].type1 = gBaseStats[gBattleMons[gActiveBattler].species].type1;
                gBattleMons[gActiveBattler].type2 = gBaseStats[gBattleMons[gActiveBattler].species].type2;
                gBattleMons[gActiveBattler].type3 = TYPE_MYSTERY;
                gBattleMons[gActiveBattler].ability = GetAbilityBySpecies(gBattleMons[gActiveBattler].species, gBattleMons[gActiveBattler].abilityNum);
                gBattleStruct->hpOnSwitchout[GetBattlerSide(gActiveBattler)] = gBattleMons[gActiveBattler].hp;
                gBattleMons[gActiveBattler].status2 = 0;
                for (i = 0; i < NUM_BATTLE_STATS; i++)
                    gBattleMons[gActiveBattler].statStages[i] = 6;
            }

            // Draw sprite.
            switch (GetBattlerPosition(gActiveBattler))
            {
            case B_POSITION_PLAYER_LEFT: // player sprite
                BtlController_EmitDrawTrainerPic(0);
                MarkBattlerForControllerExec(gActiveBattler);
                break;
            case B_POSITION_OPPONENT_LEFT:
                if (gBattleTypeFlags & BATTLE_TYPE_TRAINER) // opponent 1 sprite
                {
                    BtlController_EmitDrawTrainerPic(0);
                    MarkBattlerForControllerExec(gActiveBattler);
                }
                else // wild mon 1
                {
                    BtlController_EmitLoadMonSprite(0);
                    MarkBattlerForControllerExec(gActiveBattler);
                    gBattleResults.lastOpponentSpecies = GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_SPECIES, NULL);
                }
                break;
            case B_POSITION_PLAYER_RIGHT:
                if (gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER)) // partner sprite
                {
                    BtlController_EmitDrawTrainerPic(0);
                    MarkBattlerForControllerExec(gActiveBattler);
                }
                break;
            case B_POSITION_OPPONENT_RIGHT:
                if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
                {
                    if (gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_TWO_OPPONENTS) && !BATTLE_TWO_VS_ONE_OPPONENT) // opponent 2 if exists
                    {
                        BtlController_EmitDrawTrainerPic(0);
                        MarkBattlerForControllerExec(gActiveBattler);
                    }
                }
                else // wild mon 2
                {
                    BtlController_EmitLoadMonSprite(0);
                    MarkBattlerForControllerExec(gActiveBattler);
                    gBattleResults.lastOpponentSpecies = GetMonData(&gEnemyParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_SPECIES, NULL);
                }
                break;
            }

            if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
                BattleArena_InitPoints();
        }

        if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        {
            (*state)++;
        }
        else // Skip party summary since it is a wild battle.
        {
            if (B_FAST_INTRO)
                *state = 7; // Don't wait for sprite, print message at the same time.
            else
                *state = 6; // Wait for sprite to load.
        }
        break;
    case 5: // draw party summary in trainer battles
        if (!gBattleControllerExecFlags)
        {
            struct HpAndStatus hpStatus[PARTY_SIZE];

            for (i = 0; i < PARTY_SIZE; i++)
            {
                if (GetMonData(&gEnemyParty[i], MON_DATA_SPECIES2) == SPECIES_NONE
                 || GetMonData(&gEnemyParty[i], MON_DATA_SPECIES2) == SPECIES_EGG)
                {
                    hpStatus[i].hp = 0xFFFF;
                    hpStatus[i].status = 0;
                }
                else
                {
                    hpStatus[i].hp = GetMonData(&gEnemyParty[i], MON_DATA_HP);
                    hpStatus[i].status = GetMonData(&gEnemyParty[i], MON_DATA_STATUS);
                }
            }

            gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            BtlController_EmitDrawPartyStatusSummary(0, hpStatus, 0x80);
            MarkBattlerForControllerExec(gActiveBattler);

            for (i = 0; i < PARTY_SIZE; i++)
            {
                if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2) == SPECIES_NONE
                 || GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2) == SPECIES_EGG)
                {
                    hpStatus[i].hp = 0xFFFF;
                    hpStatus[i].status = 0;
                }
                else
                {
                    hpStatus[i].hp = GetMonData(&gPlayerParty[i], MON_DATA_HP);
                    hpStatus[i].status = GetMonData(&gPlayerParty[i], MON_DATA_STATUS);
                }
            }

            gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
            BtlController_EmitDrawPartyStatusSummary(0, hpStatus, 0x80);
            MarkBattlerForControllerExec(gActiveBattler);

            (*state)++;
        }
        break;
    case 6: // wait for previous action to complete
        if (!gBattleControllerExecFlags)
            (*state)++;
        break;
    case 7: // print battle intro message
        if (!IsBattlerMarkedForControllerExec(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
        {
            PrepareStringBattle(STRINGID_INTROMSG, GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
            (*state)++;
        }
        break;
    case 8: // wait for intro message to be printed
        if (!IsBattlerMarkedForControllerExec(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
        {
            if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
            {
                (*state)++;
            }
            else
            {
                if (B_FAST_INTRO)
                    *state = 15; // Wait for text to be printed.
                else
                    *state = 14; // Wait for text and sprite.
            }
        }
        break;
    case 9: // print opponent sends out
        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
            PrepareStringBattle(STRINGID_INTROSENDOUT, GetBattlerAtPosition(B_POSITION_PLAYER_LEFT));
        else
            PrepareStringBattle(STRINGID_INTROSENDOUT, GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT));
        (*state)++;
        break;
    case 10: // wait for opponent sends out text
        if (!gBattleControllerExecFlags)
            (*state)++;
        break;
    case 11: // first opponent's mon send out animation
        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
            gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        else
            gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);

        BtlController_EmitIntroTrainerBallThrow(0);
        MarkBattlerForControllerExec(gActiveBattler);
        (*state)++;
        break;
    case 12: // nothing
        (*state)++;
    case 13: // second opponent's mon send out
        if (gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_TWO_OPPONENTS) && !BATTLE_TWO_VS_ONE_OPPONENT)
        {
            if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
                gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);
            else
                gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);

            BtlController_EmitIntroTrainerBallThrow(0);
            MarkBattlerForControllerExec(gActiveBattler);
        }
        if (B_FAST_INTRO && !(gBattleTypeFlags & (BATTLE_TYPE_RECORDED | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_RECORDED_IS_MASTER | BATTLE_TYPE_LINK)))
            *state = 15; // Print at the same time as trainer sends out second mon.
        else
            (*state)++;
        break;
    case 14: // wait for opponent 2 send out
        if (!gBattleControllerExecFlags)
            (*state)++;
        break;
    case 15: // wait for wild battle message
        if (!IsBattlerMarkedForControllerExec(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)))
            (*state)++;
        break;
    case 16: // print player sends out
        if (!(gBattleTypeFlags & BATTLE_TYPE_SAFARI))
        {
            if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
                gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            else
                gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

            // A hack that makes fast intro work in trainer battles too.
            if (B_FAST_INTRO
                && gBattleTypeFlags & BATTLE_TYPE_TRAINER
                && !(gBattleTypeFlags & (BATTLE_TYPE_RECORDED | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_RECORDED_IS_MASTER | BATTLE_TYPE_LINK))
                && gSprites[gHealthboxSpriteIds[gActiveBattler ^ BIT_SIDE]].callback == SpriteCallbackDummy)
            {
                return;
            }

            PrepareStringBattle(STRINGID_INTROSENDOUT, gActiveBattler);
        }
        (*state)++;
        break;
    case 17: // wait for player send out message
        if (!(gBattleTypeFlags & BATTLE_TYPE_LINK && gBattleControllerExecFlags))
        {
            if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
                gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
            else
                gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

            if (!IsBattlerMarkedForControllerExec(gActiveBattler))
                (*state)++;
        }
        break;
    case 18: // player 1 send out
        if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
            gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
        else
            gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);

        BtlController_EmitIntroTrainerBallThrow(0);
        MarkBattlerForControllerExec(gActiveBattler);
        (*state)++;
        break;
    case 19: // player 2 send out
        if (gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_INGAME_PARTNER))
        {
            if (gBattleTypeFlags & BATTLE_TYPE_RECORDED_LINK && !(gBattleTypeFlags & BATTLE_TYPE_RECORDED_IS_MASTER))
                gActiveBattler = GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT);
            else
                gActiveBattler = GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT);

            BtlController_EmitIntroTrainerBallThrow(0);
            MarkBattlerForControllerExec(gActiveBattler);
        }
        (*state)++;
        break;
    case 20: // set dex and battle vars
        if (!gBattleControllerExecFlags)
        {
            for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
            {
                if (GetBattlerSide(gActiveBattler) == B_SIDE_OPPONENT
                 && !(gBattleTypeFlags & (BATTLE_TYPE_EREADER_TRAINER
                                          | BATTLE_TYPE_FRONTIER
                                          | BATTLE_TYPE_LINK
                                          | BATTLE_TYPE_RECORDED_LINK
                                          | BATTLE_TYPE_TRAINER_HILL)))
                {
                    HandleSetPokedexFlag(SpeciesToNationalPokedexNum(gBattleMons[gActiveBattler].species), FLAG_SET_SEEN, gBattleMons[gActiveBattler].personality);
                }
            }

            gBattleStruct->switchInAbilitiesCounter = 0;
            gBattleStruct->switchInItemsCounter = 0;
            gBattleStruct->overworldWeatherDone = FALSE;

            gBattleMainFunc = TryDoEventsBeforeFirstTurn;
        }
        break;
    }
}

static void TryDoEventsBeforeFirstTurn(void)
{
    s32 i, j;

    if (gBattleControllerExecFlags)
        return;

    if (gBattleStruct->switchInAbilitiesCounter == 0)
    {
        for (i = 0; i < gBattlersCount; i++)
            gBattlerByTurnOrder[i] = i;
        for (i = 0; i < gBattlersCount - 1; i++)
        {
            for (j = i + 1; j < gBattlersCount; j++)
            {
                if (GetWhoStrikesFirst(gBattlerByTurnOrder[i], gBattlerByTurnOrder[j], TRUE) != 0)
                    SwapTurnOrder(i, j);
            }
        }
    }
    if (!gBattleStruct->overworldWeatherDone
        && AbilityBattleEffects(0, 0, 0, ABILITYEFFECT_SWITCH_IN_WEATHER, 0) != 0)
    {
        gBattleStruct->overworldWeatherDone = TRUE;
        return;
    }

    if (!gBattleStruct->terrainDone && AbilityBattleEffects(0, 0, 0, ABILITYEFFECT_SWITCH_IN_TERRAIN, 0) != 0)
    {
        gBattleStruct->terrainDone = TRUE;
        return;
    }

    // Totem boosts
    for (i = 0; i < gBattlersCount; i++)
    {
        if (gTotemBoosts[i].stats != 0)
        {
            gBattlerAttacker = i;
            BattleScriptExecute(BattleScript_TotemVar);
            return;
        }
    }
    memset(gTotemBoosts, 0, sizeof(gTotemBoosts));  // erase all totem boosts just to be safe

    // Check all switch in abilities happening from the fastest mon to slowest.
    while (gBattleStruct->switchInAbilitiesCounter < gBattlersCount)
    {
        gBattlerAttacker = gBattlerByTurnOrder[gBattleStruct->switchInAbilitiesCounter++];
        if (AbilityBattleEffects(ABILITYEFFECT_ON_SWITCHIN, gBattlerAttacker, 0, 0, 0) != 0)
            return;
    }
    if (AbilityBattleEffects(ABILITYEFFECT_INTIMIDATE1, 0, 0, 0, 0) != 0)
        return;
    if (AbilityBattleEffects(ABILITYEFFECT_TRACE1, 0, 0, 0, 0) != 0)
        return;
    // Check all switch in items having effect from the fastest mon to slowest.
    while (gBattleStruct->switchInItemsCounter < gBattlersCount)
    {
        if (ItemBattleEffects(ITEMEFFECT_ON_SWITCH_IN, gBattlerByTurnOrder[gBattleStruct->switchInItemsCounter++], FALSE))
            return;
    }

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        *(gBattleStruct->monToSwitchIntoId + i) = PARTY_SIZE;
        gChosenActionByBattler[i] = B_ACTION_NONE;
        gChosenMoveByBattler[i] = MOVE_NONE;
    }
    TurnValuesCleanUp(FALSE);
    SpecialStatusesClear();
    *(&gBattleStruct->field_91) = gAbsentBattlerFlags;
    BattlePutTextOnWindow(gText_EmptyString3, 0);
    gBattleMainFunc = HandleTurnActionSelectionState;
    ResetSentPokesToOpponentValue();

    for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
        gBattleCommunication[i] = 0;

    for (i = 0; i < gBattlersCount; i++)
        gBattleMons[i].status2 &= ~(STATUS2_FLINCHED);

    *(&gBattleStruct->turnEffectsTracker) = 0;
    *(&gBattleStruct->turnEffectsBattlerId) = 0;
    *(&gBattleStruct->wishPerishSongState) = 0;
    *(&gBattleStruct->wishPerishSongBattlerId) = 0;
    gBattleScripting.moveendState = 0;
    gBattleStruct->faintedActionsState = 0;
    gBattleStruct->turnCountersTracker = 0;
    gMoveResultFlags = 0;

    gRandomTurnNumber = Random();

    if (gBattleTypeFlags & BATTLE_TYPE_ARENA)
    {
        StopCryAndClearCrySongs();
        BattleScriptExecute(BattleScript_ArenaTurnBeginning);
    }
}

static void HandleEndTurn_ContinueBattle(void)
{
    s32 i;

    if (gBattleControllerExecFlags == 0)
    {
        gBattleMainFunc = BattleTurnPassed;
        for (i = 0; i < BATTLE_COMMUNICATION_ENTRIES_COUNT; i++)
            gBattleCommunication[i] = 0;
        for (i = 0; i < gBattlersCount; i++)
        {
            gBattleMons[i].status2 &= ~(STATUS2_FLINCHED);
            if ((gBattleMons[i].status1 & STATUS1_SLEEP) && (gBattleMons[i].status2 & STATUS2_MULTIPLETURNS))
                CancelMultiTurnMoves(i);
        }
        gBattleStruct->turnEffectsTracker = 0;
        gBattleStruct->turnEffectsBattlerId = 0;
        gBattleStruct->wishPerishSongState = 0;
        gBattleStruct->wishPerishSongBattlerId = 0;
        gBattleStruct->turnCountersTracker = 0;
        gMoveResultFlags = 0;
    }
}

void BattleTurnPassed(void)
{
    s32 i;

    TurnValuesCleanUp(TRUE);
    if (gBattleOutcome == 0)
    {
        if (DoFieldEndTurnEffects())
            return;
        if (DoBattlerEndTurnEffects())
            return;
    }
    if (HandleFaintedMonActions())
        return;
    gBattleStruct->faintedActionsState = 0;
    if (HandleWishPerishSongOnTurnEnd())
        return;

    TurnValuesCleanUp(FALSE);
    gHitMarker &= ~(HITMARKER_NO_ATTACKSTRING);
    gHitMarker &= ~(HITMARKER_UNABLE_TO_USE_MOVE);
    gHitMarker &= ~(HITMARKER_x400000);
    gHitMarker &= ~(HITMARKER_x100000);
    gBattleScripting.animTurn = 0;
    gBattleScripting.animTargetsHit = 0;
    gBattleScripting.moveendState = 0;
    gBattleMoveDamage = 0;
    gMoveResultFlags = 0;

    for (i = 0; i < 5; i++)
        gBattleCommunication[i] = 0;

    if (gBattleOutcome != 0)
    {
        gCurrentActionFuncId = B_ACTION_FINISHED;
        gBattleMainFunc = RunTurnActionsFunctions;
        return;
    }

    if (gBattleResults.battleTurnCounter < 0xFF)
    {
        gBattleResults.battleTurnCounter++;
        gBattleStruct->arenaTurnCounter++;
    }

    for (i = 0; i < gBattlersCount; i++)
    {
        gChosenActionByBattler[i] = B_ACTION_NONE;
        gChosenMoveByBattler[i] = MOVE_NONE;
    }

    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
        *(gBattleStruct->monToSwitchIntoId + i) = PARTY_SIZE;

    *(&gBattleStruct->field_91) = gAbsentBattlerFlags;
    BattlePutTextOnWindow(gText_EmptyString3, 0);
    gBattleMainFunc = HandleTurnActionSelectionState;
    gRandomTurnNumber = Random();

    if (gBattleTypeFlags & BATTLE_TYPE_PALACE)
        BattleScriptExecute(BattleScript_PalacePrintFlavorText);
    else if (gBattleTypeFlags & BATTLE_TYPE_ARENA && gBattleStruct->arenaTurnCounter == 0)
        BattleScriptExecute(BattleScript_ArenaTurnBeginning);
    else if (ShouldDoTrainerSlide(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT), gTrainerBattleOpponent_A, TRAINER_SLIDE_LAST_LOW_HP))
        BattleScriptExecute(BattleScript_TrainerSlideMsgEnd2);
}

u8 IsRunningFromBattleImpossible(void)
{
    u32 holdEffect, i;

    if (gBattleMons[gActiveBattler].item == ITEM_ENIGMA_BERRY)
        holdEffect = gEnigmaBerries[gActiveBattler].holdEffect;
    else
        holdEffect = ItemId_GetHoldEffect(gBattleMons[gActiveBattler].item);

    gPotentialItemEffectBattler = gActiveBattler;

    if (gBattleTypeFlags & BATTLE_TYPE_FIRST_BATTLE) // Cannot ever run from saving Birch's battle.
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = 1;
        return 1;
    }
    if (GetBattlerPosition(gActiveBattler) == B_POSITION_PLAYER_RIGHT && WILD_DOUBLE_BATTLE
        && IsBattlerAlive(GetBattlerAtPosition(B_POSITION_PLAYER_LEFT))) // The second pokemon cannot run from a double wild battle, unless it's the only alive mon.
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = 0;
        return 1;
    }

    if (holdEffect == HOLD_EFFECT_CAN_ALWAYS_RUN)
        return 0;
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        return 0;
    if (gBattleMons[gActiveBattler].ability == ABILITY_RUN_AWAY)
        return 0;

    if ((i = IsAbilityPreventingEscape(gActiveBattler)))
    {
        gBattleScripting.battler = i - 1;
        gLastUsedAbility = gBattleMons[i - 1].ability;
        gBattleCommunication[MULTISTRING_CHOOSER] = 2;
        return 2;
    }

    if (!CanBattlerEscape(gActiveBattler))
    {
        gBattleCommunication[MULTISTRING_CHOOSER] = 0;
        return 1;
    }
    return 0;
}

void SwitchPartyOrder(u8 battler)
{
    s32 i;
    u8 partyId1;
    u8 partyId2;

    // gBattleStruct->field_60[battler][i]

    for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = *(battler * 3 + i + (u8*)(gBattleStruct->field_60));

    partyId1 = GetPartyIdFromBattlePartyId(gBattlerPartyIndexes[battler]);
    partyId2 = GetPartyIdFromBattlePartyId(*(gBattleStruct->monToSwitchIntoId + battler));
    SwitchPartyMonSlots(partyId1, partyId2);

    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE)
    {
        for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        {
            *(battler * 3 + i + (u8*)(gBattleStruct->field_60)) = gBattlePartyCurrentOrder[i];
            *(BATTLE_PARTNER(battler) * 3 + i + (u8*)(gBattleStruct->field_60)) = gBattlePartyCurrentOrder[i];
        }
    }
    else
    {
        for (i = 0; i < (int)ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        {
            *(battler * 3 + i + (u8*)(gBattleStruct->field_60)) = gBattlePartyCurrentOrder[i];
        }
    }
}

enum
{
    STATE_TURN_START_RECORD,
    STATE_BEFORE_ACTION_CHOSEN,
    STATE_WAIT_ACTION_CHOSEN,
    STATE_WAIT_ACTION_CASE_CHOSEN,
    STATE_WAIT_ACTION_CONFIRMED_STANDBY,
    STATE_WAIT_ACTION_CONFIRMED,
    STATE_SELECTION_SCRIPT,
    STATE_WAIT_SET_BEFORE_ACTION,
    STATE_SELECTION_SCRIPT_MAY_RUN
};

static void HandleTurnActionSelectionState(void)
{
    s32 i;

    gBattleCommunication[ACTIONS_CONFIRMED_COUNT] = 0;
    for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
    {
        u8 position = GetBattlerPosition(gActiveBattler);
        switch (gBattleCommunication[gActiveBattler])
        {
        case STATE_TURN_START_RECORD: // Recorded battle related action on start of every turn.
            RecordedBattle_CopyBattlerMoves();
            gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
            break;
        case STATE_BEFORE_ACTION_CHOSEN: // Choose an action.
            *(gBattleStruct->monToSwitchIntoId + gActiveBattler) = PARTY_SIZE;
            if (gBattleTypeFlags & BATTLE_TYPE_MULTI
                || (position & BIT_FLANK) == B_FLANK_LEFT
                || gBattleStruct->field_91 & gBitTable[GetBattlerAtPosition(BATTLE_PARTNER(position))]
                || gBattleCommunication[GetBattlerAtPosition(BATTLE_PARTNER(position))] == STATE_WAIT_ACTION_CONFIRMED)
            {
                if (gBattleStruct->field_91 & gBitTable[gActiveBattler])
                {
                    gChosenActionByBattler[gActiveBattler] = B_ACTION_NOTHING_FAINTED;
                    if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
                        gBattleCommunication[gActiveBattler] = STATE_WAIT_ACTION_CONFIRMED;
                    else
                        gBattleCommunication[gActiveBattler] = STATE_WAIT_ACTION_CONFIRMED_STANDBY;
                }
                else
                {
                    if (gBattleMons[gActiveBattler].status2 & STATUS2_MULTIPLETURNS
                        || gBattleMons[gActiveBattler].status2 & STATUS2_RECHARGE)
                    {
                        gChosenActionByBattler[gActiveBattler] = B_ACTION_USE_MOVE;
                        gBattleCommunication[gActiveBattler] = STATE_WAIT_ACTION_CONFIRMED_STANDBY;
                    }
                    else if (WILD_DOUBLE_BATTLE
                             && position == B_POSITION_PLAYER_RIGHT
                             && (gBattleStruct->throwingPokeBall || gChosenActionByBattler[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)] == B_ACTION_RUN))
                    {
                        gBattleStruct->throwingPokeBall = FALSE;
                        gChosenActionByBattler[gActiveBattler] = B_ACTION_NOTHING_FAINTED; // Not fainted, but it cannot move, because of the throwing ball.
                        gBattleCommunication[gActiveBattler] = STATE_WAIT_ACTION_CONFIRMED_STANDBY;
                    }
                    else
                    {
                        BtlController_EmitChooseAction(0, gChosenActionByBattler[0], gBattleResources->bufferB[0][1] | (gBattleResources->bufferB[0][2] << 8));
                        MarkBattlerForControllerExec(gActiveBattler);
                        gBattleCommunication[gActiveBattler]++;
                    }
                }
            }
            break;
        case STATE_WAIT_ACTION_CHOSEN: // Try to perform an action.
            if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) | (0xF << 28) | (gBitTable[gActiveBattler] << 4) | (gBitTable[gActiveBattler] << 8) | (gBitTable[gActiveBattler] << 12))))
            {
                RecordedBattle_SetBattlerAction(gActiveBattler, gBattleResources->bufferB[gActiveBattler][1]);
                gChosenActionByBattler[gActiveBattler] = gBattleResources->bufferB[gActiveBattler][1];

                switch (gBattleResources->bufferB[gActiveBattler][1])
                {
                case B_ACTION_USE_MOVE:
                    if (AreAllMovesUnusable())
                    {
                        gBattleCommunication[gActiveBattler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + gActiveBattler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + gActiveBattler) = STATE_WAIT_ACTION_CONFIRMED_STANDBY;
                        *(gBattleStruct->moveTarget + gActiveBattler) = gBattleResources->bufferB[gActiveBattler][3];
                        return;
                    }
                    else if (gDisableStructs[gActiveBattler].encoredMove != 0)
                    {
                        gChosenMoveByBattler[gActiveBattler] = gDisableStructs[gActiveBattler].encoredMove;
                        *(gBattleStruct->chosenMovePositions + gActiveBattler) = gDisableStructs[gActiveBattler].encoredMovePos;
                        gBattleCommunication[gActiveBattler] = STATE_WAIT_ACTION_CONFIRMED_STANDBY;
                        return;
                    }
                    else
                    {
                        struct ChooseMoveStruct moveInfo;

                        moveInfo.zmove = gBattleStruct->zmove;
                        moveInfo.mega = gBattleStruct->mega;
                        moveInfo.species = gBattleMons[gActiveBattler].species;
                        moveInfo.monType1 = gBattleMons[gActiveBattler].type1;
                        moveInfo.monType2 = gBattleMons[gActiveBattler].type2;
                        moveInfo.monType3 = gBattleMons[gActiveBattler].type3;

                        for (i = 0; i < MAX_MON_MOVES; i++)
                        {
                            moveInfo.moves[i] = gBattleMons[gActiveBattler].moves[i];
                            moveInfo.currentPp[i] = gBattleMons[gActiveBattler].pp[i];
                            moveInfo.maxPp[i] = CalculatePPWithBonus(
                                                            gBattleMons[gActiveBattler].moves[i],
                                                            gBattleMons[gActiveBattler].ppBonuses,
                                                            i);
                        }

                        BtlController_EmitChooseMove(0, (gBattleTypeFlags & BATTLE_TYPE_DOUBLE) != 0, FALSE, &moveInfo);
                        MarkBattlerForControllerExec(gActiveBattler);
                    }
                    break;
                case B_ACTION_USE_ITEM:
                    if (((CheckSpeedchoiceOption(DEBUG_MENUS, DEBUG_MENUS_ON) == TRUE) && FlagGet(FLAG_SYS_NO_BAG_USE)) || (gBattleTypeFlags & (BATTLE_TYPE_LINK
                                            | BATTLE_TYPE_FRONTIER_NO_PYRAMID
                                            | BATTLE_TYPE_EREADER_TRAINER
                                            | BATTLE_TYPE_RECORDED_LINK))
                                            // Or if currently held by Sky Drop
                                            || ((gStatuses3[gActiveBattler] & STATUS3_ON_AIR) && (gStatuses3[gActiveBattler] & STATUS3_UNDERGROUND)))
                    {
                        RecordedBattle_ClearBattlerAction(gActiveBattler, 1);
                        gSelectionBattleScripts[gActiveBattler] = BattleScript_ActionSelectionItemsCantBeUsed;
                        gBattleCommunication[gActiveBattler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + gActiveBattler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + gActiveBattler) = STATE_BEFORE_ACTION_CHOSEN;
                        return;
                    }
                    else
                    {
                        BtlController_EmitChooseItem(0, gBattleStruct->field_60[gActiveBattler]);
                        MarkBattlerForControllerExec(gActiveBattler);
                    }
                    break;
                case B_ACTION_SWITCH:
                    *(gBattleStruct->field_58 + gActiveBattler) = gBattlerPartyIndexes[gActiveBattler];
                    if (gBattleTypeFlags & BATTLE_TYPE_ARENA
                        || !CanBattlerEscape(gActiveBattler))
                    {
                        BtlController_EmitChoosePokemon(0, PARTY_ACTION_CANT_SWITCH, PARTY_SIZE, ABILITY_NONE, gBattleStruct->field_60[gActiveBattler]);
                    }
                    else if ((i = IsAbilityPreventingEscape(gActiveBattler)))
                    {
                        BtlController_EmitChoosePokemon(0, ((i - 1) << 4) | PARTY_ACTION_ABILITY_PREVENTS, PARTY_SIZE, gBattleMons[i - 1].ability, gBattleStruct->field_60[gActiveBattler]);
                    }
                    else
                    {
                        if (gActiveBattler == 2 && gChosenActionByBattler[0] == B_ACTION_SWITCH)
                            BtlController_EmitChoosePokemon(0, PARTY_ACTION_CHOOSE_MON, *(gBattleStruct->monToSwitchIntoId + 0), ABILITY_NONE, gBattleStruct->field_60[gActiveBattler]);
                        else if (gActiveBattler == 3 && gChosenActionByBattler[1] == B_ACTION_SWITCH)
                            BtlController_EmitChoosePokemon(0, PARTY_ACTION_CHOOSE_MON, *(gBattleStruct->monToSwitchIntoId + 1), ABILITY_NONE, gBattleStruct->field_60[gActiveBattler]);
                        else
                            BtlController_EmitChoosePokemon(0, PARTY_ACTION_CHOOSE_MON, PARTY_SIZE, ABILITY_NONE, gBattleStruct->field_60[gActiveBattler]);
                    }
                    MarkBattlerForControllerExec(gActiveBattler);
                    break;
                case B_ACTION_SAFARI_BALL:
                    if (IsPlayerPartyAndPokemonStorageFull())
                    {
                        gSelectionBattleScripts[gActiveBattler] = BattleScript_PrintFullBox;
                        gBattleCommunication[gActiveBattler] = STATE_SELECTION_SCRIPT;
                        *(gBattleStruct->selectionScriptFinished + gActiveBattler) = FALSE;
                        *(gBattleStruct->stateIdAfterSelScript + gActiveBattler) = STATE_BEFORE_ACTION_CHOSEN;
                        return;
                    }
                    break;
                case B_ACTION_SAFARI_POKEBLOCK:
                    BtlController_EmitChooseItem(0, gBattleStruct->field_60[gActiveBattler]);
                    MarkBattlerForControllerExec(gActiveBattler);
                    break;
                case B_ACTION_CANCEL_PARTNER:
                    gBattleCommunication[gActiveBattler] = STATE_WAIT_SET_BEFORE_ACTION;
                    gBattleCommunication[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))] = STATE_BEFORE_ACTION_CHOSEN;
                    RecordedBattle_ClearBattlerAction(gActiveBattler, 1);
                    if (gBattleMons[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))].status2 & STATUS2_MULTIPLETURNS
                        || gBattleMons[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))].status2 & STATUS2_RECHARGE)
                    {
                        BtlController_EmitEndBounceEffect(0);
                        MarkBattlerForControllerExec(gActiveBattler);
                        return;
                    }
                    else if (gChosenActionByBattler[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))] == B_ACTION_SWITCH)
                    {
                        RecordedBattle_ClearBattlerAction(GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))), 2);
                    }
                    else if (gChosenActionByBattler[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))] == B_ACTION_RUN)
                    {
                        RecordedBattle_ClearBattlerAction(GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))), 1);
                    }
                    else if (gChosenActionByBattler[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))] == B_ACTION_USE_MOVE
                             && (gProtectStructs[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))].noValidMoves
                                || gDisableStructs[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))].encoredMove))
                    {
                        RecordedBattle_ClearBattlerAction(GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))), 1);
                    }
                    else if (gBattleTypeFlags & BATTLE_TYPE_PALACE
                             && gChosenActionByBattler[GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler)))] == B_ACTION_USE_MOVE)
                    {
                        gRngValue = gBattlePalaceMoveSelectionRngValue;
                        RecordedBattle_ClearBattlerAction(GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))), 1);
                    }
                    else
                    {
                        RecordedBattle_ClearBattlerAction(GetBattlerAtPosition(BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))), 3);
                    }

                    gBattleStruct->mega.toEvolve &= ~(gBitTable[BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))]);
                    gBattleStruct->zmove.toBeUsed[BATTLE_PARTNER(GetBattlerPosition(gActiveBattler))] = MOVE_NONE;
                    BtlController_EmitEndBounceEffect(0);
                    MarkBattlerForControllerExec(gActiveBattler);
                    return;
                case B_ACTION_DEBUG:
                    BtlController_EmitDebugMenu(0);
                    MarkBattlerForControllerExec(gActiveBattler);
                    break;
                }

                if (gBattleTypeFlags & BATTLE_TYPE_TRAINER
                    && gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL)
                    && gBattleResources->bufferB[gActiveBattler][1] == B_ACTION_RUN)
                {
                    gSelectionBattleScripts[gActiveBattler] = BattleScript_AskIfWantsToForfeitMatch;
                    gBattleCommunication[gActiveBattler] = STATE_SELECTION_SCRIPT_MAY_RUN;
                    *(gBattleStruct->selectionScriptFinished + gActiveBattler) = FALSE;
                    *(gBattleStruct->stateIdAfterSelScript + gActiveBattler) = STATE_BEFORE_ACTION_CHOSEN;
                    return;
                }
                else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER
                         && !(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
                         && gBattleResources->bufferB[gActiveBattler][1] == B_ACTION_RUN)
                {
                    BattleScriptExecute(BattleScript_PrintCantRunFromTrainer);
                    gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
                }
                else if (IsRunningFromBattleImpossible()
                         && gBattleResources->bufferB[gActiveBattler][1] == B_ACTION_RUN)
                {
                    gSelectionBattleScripts[gActiveBattler] = BattleScript_PrintCantEscapeFromBattle;
                    gBattleCommunication[gActiveBattler] = STATE_SELECTION_SCRIPT;
                    *(gBattleStruct->selectionScriptFinished + gActiveBattler) = FALSE;
                    *(gBattleStruct->stateIdAfterSelScript + gActiveBattler) = STATE_BEFORE_ACTION_CHOSEN;
                    return;
                }
                else
                {
                    gBattleCommunication[gActiveBattler]++;
                }
            }
            break;
        case STATE_WAIT_ACTION_CASE_CHOSEN:
            if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) | (0xF << 28) | (gBitTable[gActiveBattler] << 4) | (gBitTable[gActiveBattler] << 8) | (gBitTable[gActiveBattler] << 12))))
            {
                switch (gChosenActionByBattler[gActiveBattler])
                {
                case B_ACTION_USE_MOVE:
                    switch (gBattleResources->bufferB[gActiveBattler][1])
                    {
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                    case 8:
                    case 9:
                        gChosenActionByBattler[gActiveBattler] = gBattleResources->bufferB[gActiveBattler][1];
                        return;
                    case 15:
                        gChosenActionByBattler[gActiveBattler] = B_ACTION_SWITCH;
                        sub_803CDF8();
                        return;
                    default:
                        sub_818603C(2);
                        if ((gBattleResources->bufferB[gActiveBattler][2] | (gBattleResources->bufferB[gActiveBattler][3] << 8)) == 0xFFFF)
                        {
                            gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
                            RecordedBattle_ClearBattlerAction(gActiveBattler, 1);
                        }
                        else if (TrySetCantSelectMoveBattleScript())
                        {
                            RecordedBattle_ClearBattlerAction(gActiveBattler, 1);
                            gBattleCommunication[gActiveBattler] = STATE_SELECTION_SCRIPT;
                            *(gBattleStruct->selectionScriptFinished + gActiveBattler) = FALSE;
                            gBattleResources->bufferB[gActiveBattler][1] = B_ACTION_USE_MOVE;
                            *(gBattleStruct->stateIdAfterSelScript + gActiveBattler) = STATE_WAIT_ACTION_CHOSEN;
                            return;
                        }
                        else
                        {
                            if (!(gBattleTypeFlags & BATTLE_TYPE_PALACE))
                            {
                                RecordedBattle_SetBattlerAction(gActiveBattler, gBattleResources->bufferB[gActiveBattler][2]);
                                RecordedBattle_SetBattlerAction(gActiveBattler, gBattleResources->bufferB[gActiveBattler][3]);
                            }
                            *(gBattleStruct->chosenMovePositions + gActiveBattler) = gBattleResources->bufferB[gActiveBattler][2] & ~(RET_MEGA_EVOLUTION);
                            gChosenMoveByBattler[gActiveBattler] = gBattleMons[gActiveBattler].moves[*(gBattleStruct->chosenMovePositions + gActiveBattler)];
                            *(gBattleStruct->moveTarget + gActiveBattler) = gBattleResources->bufferB[gActiveBattler][3];
                            if (gBattleResources->bufferB[gActiveBattler][2] & RET_MEGA_EVOLUTION)
                                gBattleStruct->mega.toEvolve |= gBitTable[gActiveBattler];
                            gBattleCommunication[gActiveBattler]++;
                        }
                        break;
                    }
                    break;
                case B_ACTION_USE_ITEM:
                    if ((gBattleResources->bufferB[gActiveBattler][1] | (gBattleResources->bufferB[gActiveBattler][2] << 8)) == 0)
                    {
                        gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
                    }
                    else
                    {
                        gLastUsedItem = (gBattleResources->bufferB[gActiveBattler][1] | (gBattleResources->bufferB[gActiveBattler][2] << 8));
                        if (ItemId_GetPocket(gLastUsedItem) == POCKET_POKE_BALLS)
                            gBattleStruct->throwingPokeBall = TRUE;
                        gBattleCommunication[gActiveBattler]++;
                    }
                    break;
                case B_ACTION_SWITCH:
                    if (gBattleResources->bufferB[gActiveBattler][1] == PARTY_SIZE)
                    {
                        gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
                        RecordedBattle_ClearBattlerAction(gActiveBattler, 1);
                    }
                    else
                    {
                        sub_803CDF8();
                        gBattleCommunication[gActiveBattler]++;
                    }
                    break;
                case B_ACTION_RUN:
                    gHitMarker |= HITMARKER_RUN;
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_SAFARI_WATCH_CAREFULLY:
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_SAFARI_BALL:
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_THROW_BALL:
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_SAFARI_POKEBLOCK:
                    if ((gBattleResources->bufferB[gActiveBattler][1] | (gBattleResources->bufferB[gActiveBattler][2] << 8)) != 0)
                    {
                        gBattleCommunication[gActiveBattler]++;
                    }
                    else
                    {
                        gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
                    }
                    break;
                case B_ACTION_SAFARI_GO_NEAR:
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_SAFARI_RUN:
                    gHitMarker |= HITMARKER_RUN;
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_WALLY_THROW:
                    gBattleCommunication[gActiveBattler]++;
                    break;
                case B_ACTION_DEBUG:
                    gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
                    break;
                }
            }
            break;
        case STATE_WAIT_ACTION_CONFIRMED_STANDBY:
            if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) 
                                                | (0xF << 28)
                                                | (gBitTable[gActiveBattler] << 4) 
                                                | (gBitTable[gActiveBattler] << 8) 
                                                | (gBitTable[gActiveBattler] << 12))))
            {
                if (AllAtActionConfirmed())
                    i = TRUE;
                else
                    i = FALSE;

                if (((gBattleTypeFlags & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE)) != BATTLE_TYPE_DOUBLE)
                    || (position & BIT_FLANK) != B_FLANK_LEFT
                    || (*(&gBattleStruct->field_91) & gBitTable[GetBattlerAtPosition(position ^ BIT_FLANK)]))
                {
                    BtlController_EmitLinkStandbyMsg(0, 0, i);
                }
                else
                {
                    BtlController_EmitLinkStandbyMsg(0, 1, i);
                }
                MarkBattlerForControllerExec(gActiveBattler);
                gBattleCommunication[gActiveBattler]++;
            }
            break;
        case STATE_WAIT_ACTION_CONFIRMED:
            if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) | (0xF << 28) | (gBitTable[gActiveBattler] << 4) | (gBitTable[gActiveBattler] << 8) | (gBitTable[gActiveBattler] << 12))))
            {
                gBattleCommunication[ACTIONS_CONFIRMED_COUNT]++;
            }
            break;
        case STATE_SELECTION_SCRIPT:
            if (*(gBattleStruct->selectionScriptFinished + gActiveBattler))
            {
                gBattleCommunication[gActiveBattler] = *(gBattleStruct->stateIdAfterSelScript + gActiveBattler);
            }
            else
            {
                gBattlerAttacker = gActiveBattler;
                gBattlescriptCurrInstr = gSelectionBattleScripts[gActiveBattler];
                if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) | (0xF << 28) | (gBitTable[gActiveBattler] << 4) | (gBitTable[gActiveBattler] << 8) | (gBitTable[gActiveBattler] << 12))))
                {
                    gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
                }
                gSelectionBattleScripts[gActiveBattler] = gBattlescriptCurrInstr;
            }
            break;
        case STATE_WAIT_SET_BEFORE_ACTION:
            if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) | (0xF << 28) | (gBitTable[gActiveBattler] << 4) | (gBitTable[gActiveBattler] << 8) | (gBitTable[gActiveBattler] << 12))))
            {
                gBattleCommunication[gActiveBattler] = STATE_BEFORE_ACTION_CHOSEN;
            }
            break;
        case STATE_SELECTION_SCRIPT_MAY_RUN:
            if (*(gBattleStruct->selectionScriptFinished + gActiveBattler))
            {
                if (gBattleResources->bufferB[gActiveBattler][1] == B_ACTION_NOTHING_FAINTED)
                {
                    gHitMarker |= HITMARKER_RUN;
                    gChosenActionByBattler[gActiveBattler] = B_ACTION_RUN;
                    gBattleCommunication[gActiveBattler] = STATE_WAIT_ACTION_CONFIRMED_STANDBY;
                }
                else
                {
                    RecordedBattle_ClearBattlerAction(gActiveBattler, 1);
                    gBattleCommunication[gActiveBattler] = *(gBattleStruct->stateIdAfterSelScript + gActiveBattler);
                }
            }
            else
            {
                gBattlerAttacker = gActiveBattler;
                gBattlescriptCurrInstr = gSelectionBattleScripts[gActiveBattler];
                if (!(gBattleControllerExecFlags & ((gBitTable[gActiveBattler]) | (0xF << 28) | (gBitTable[gActiveBattler] << 4) | (gBitTable[gActiveBattler] << 8) | (gBitTable[gActiveBattler] << 12))))
                {
                    gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
                }
                gSelectionBattleScripts[gActiveBattler] = gBattlescriptCurrInstr;
            }
            break;
        }
    }

    // Check if everyone chose actions.
    if (gBattleCommunication[ACTIONS_CONFIRMED_COUNT] == gBattlersCount)
    {
        sub_818603C(1);
        gBattleMainFunc = SetActionsAndBattlersTurnOrder;

        if (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER)
        {
            for (i = 0; i < gBattlersCount; i++)
            {
                if (gChosenActionByBattler[i] == B_ACTION_SWITCH)
                    SwitchPartyOrderInGameMulti(i, *(gBattleStruct->monToSwitchIntoId + i));
            }
        }
    }
}

static bool8 AllAtActionConfirmed(void)
{
    s32 i, count;

    for (count = 0, i = 0; i < gBattlersCount; i++)
    {
        if (gBattleCommunication[i] == STATE_WAIT_ACTION_CONFIRMED)
            count++;
    }

    if (count + 1 == gBattlersCount)
        return TRUE;
    else
        return FALSE;
}

static void sub_803CDF8(void)
{
    *(gBattleStruct->monToSwitchIntoId + gActiveBattler) = gBattleResources->bufferB[gActiveBattler][1];
    RecordedBattle_SetBattlerAction(gActiveBattler, gBattleResources->bufferB[gActiveBattler][1]);

    if (gBattleTypeFlags & BATTLE_TYPE_LINK && gBattleTypeFlags & BATTLE_TYPE_MULTI)
    {
        *(gActiveBattler * 3 + (u8*)(gBattleStruct->field_60) + 0) &= 0xF;
        *(gActiveBattler * 3 + (u8*)(gBattleStruct->field_60) + 0) |= (gBattleResources->bufferB[gActiveBattler][2] & 0xF0);
        *(gActiveBattler * 3 + (u8*)(gBattleStruct->field_60) + 1) = gBattleResources->bufferB[gActiveBattler][3];

        *((gActiveBattler ^ BIT_FLANK) * 3 + (u8*)(gBattleStruct->field_60) + 0) &= (0xF0);
        *((gActiveBattler ^ BIT_FLANK) * 3 + (u8*)(gBattleStruct->field_60) + 0) |= (gBattleResources->bufferB[gActiveBattler][2] & 0xF0) >> 4;
        *((gActiveBattler ^ BIT_FLANK) * 3 + (u8*)(gBattleStruct->field_60) + 2) = gBattleResources->bufferB[gActiveBattler][3];
    }
}

void SwapTurnOrder(u8 id1, u8 id2)
{
    u32 temp;

    SWAP(gActionsByTurnOrder[id1], gActionsByTurnOrder[id2], temp);
    SWAP(gBattlerByTurnOrder[id1], gBattlerByTurnOrder[id2], temp);
}

u32 GetBattlerTotalSpeedStat(u8 battlerId)
{
    u32 speed = gBattleMons[battlerId].speed;
    u32 ability = GetBattlerAbility(battlerId);
    u32 holdEffect = GetBattlerHoldEffect(battlerId, TRUE);

    // weather abilities
    if (WEATHER_HAS_EFFECT)
    {
        if (ability == ABILITY_SWIFT_SWIM       && gBattleWeather & WEATHER_RAIN_ANY)
            speed *= 2;
        else if (ability == ABILITY_CHLOROPHYLL && gBattleWeather & WEATHER_SUN_ANY)
            speed *= 2;
        else if (ability == ABILITY_SAND_RUSH   && gBattleWeather & WEATHER_SANDSTORM_ANY)
            speed *= 2;
        else if (ability == ABILITY_SLUSH_RUSH  && gBattleWeather & WEATHER_HAIL_ANY)
            speed *= 2;
    }

    // other abilities
    if (ability == ABILITY_QUICK_FEET && gBattleMons[battlerId].status1 & STATUS1_ANY)
        speed = (speed * 150) / 100;
    else if (ability == ABILITY_SURGE_SURFER && gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN)
        speed *= 2;
    else if (ability == ABILITY_SLOW_START && gDisableStructs[battlerId].slowStartTimer != 0)
        speed /= 2;

    // stat stages
    speed *= gStatStageRatios[gBattleMons[battlerId].statStages[STAT_SPEED]][0];
    speed /= gStatStageRatios[gBattleMons[battlerId].statStages[STAT_SPEED]][1];

    // player's badge boost
    if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_FRONTIER))
        && ShouldGetStatBadgeBoost(FLAG_BADGE03_GET, battlerId)
        && GetBattlerSide(battlerId) == B_SIDE_PLAYER)
    {
        speed = (speed * 110) / 100;
    }

    // item effects
    if (holdEffect == HOLD_EFFECT_MACHO_BRACE || holdEffect == HOLD_EFFECT_POWER_ITEM)
        speed /= 2;
    else if (holdEffect == HOLD_EFFECT_IRON_BALL)
        speed /= 2;
    else if (holdEffect == HOLD_EFFECT_CHOICE_SCARF)
        speed = (speed * 150) / 100;
    else if (holdEffect == HOLD_EFFECT_QUICK_POWDER && gBattleMons[battlerId].species == SPECIES_DITTO && !(gBattleMons[battlerId].status2 & STATUS2_TRANSFORMED))
        speed *= 2;

    // various effects
    if (gSideStatuses[GET_BATTLER_SIDE(battlerId)] & SIDE_STATUS_TAILWIND)
        speed *= 2;
    if (gBattleResources->flags->flags[battlerId] & RESOURCE_FLAG_UNBURDEN)
        speed *= 2;

    // paralysis drop
    if (gBattleMons[battlerId].status1 & STATUS1_PARALYSIS && ability != ABILITY_QUICK_FEET)
        speed /= (B_PARALYSIS_SPEED >= GEN_7 ? 2 : 4);

    return speed;
}

s8 GetChosenMovePriority(u32 battlerId)
{
    u16 move;

    if (gProtectStructs[battlerId].noValidMoves)
        move = MOVE_STRUGGLE;
    else
        move = gBattleMons[battlerId].moves[*(gBattleStruct->chosenMovePositions + battlerId)];

    return GetMovePriority(battlerId, move);
}

s8 GetMovePriority(u32 battlerId, u16 move)
{
    s8 priority;

    priority = gBattleMoves[move].priority;
    if (GetBattlerAbility(battlerId) == ABILITY_GALE_WINGS
        && gBattleMoves[move].type == TYPE_FLYING
        && (B_GALE_WINGS <= GEN_6 || BATTLER_MAX_HP(battlerId)))
    {
        priority++;
    }
    else if (GetBattlerAbility(battlerId) == ABILITY_PRANKSTER && IS_MOVE_STATUS(move))
    {
        priority++;
    }
    else if (GetBattlerAbility(battlerId) == ABILITY_TRIAGE)
    {
        switch (gBattleMoves[move].effect)
        {
        case EFFECT_RESTORE_HP:
        case EFFECT_REST:
        case EFFECT_MORNING_SUN:
        case EFFECT_MOONLIGHT:
        case EFFECT_SYNTHESIS:
        case EFFECT_HEAL_PULSE:
        case EFFECT_HEALING_WISH:
        case EFFECT_SWALLOW:
        case EFFECT_WISH:
        case EFFECT_SOFTBOILED:
        case EFFECT_ABSORB:
        case EFFECT_ROOST:
            priority += 3;
            break;
        }
    }

    return priority;
}

u8 GetWhoStrikesFirst(u8 battler1, u8 battler2, bool8 ignoreChosenMoves)
{
    u8 strikesFirst = 0;
    u32 speedBattler1 = 0, speedBattler2 = 0;
    u32 holdEffectBattler1 = 0, holdEffectBattler2 = 0;
    s8 priority1 = 0, priority2 = 0;

    speedBattler1 = GetBattlerTotalSpeedStat(battler1);
    holdEffectBattler1 = GetBattlerHoldEffect(battler1, TRUE);
    if ((holdEffectBattler1 == HOLD_EFFECT_QUICK_CLAW && gRandomTurnNumber < (0xFFFF * GetBattlerHoldEffectParam(battler1)) / 100)
     || (!IsAbilityOnOpposingSide(battler1, ABILITY_UNNERVE)
      && holdEffectBattler1 == HOLD_EFFECT_CUSTAP_BERRY
      && HasEnoughHpToEatBerry(battler1, 4, gBattleMons[battler1].item)))
        gProtectStructs[battler1].custap = TRUE;

    speedBattler2 = GetBattlerTotalSpeedStat(battler2);
    holdEffectBattler2 = GetBattlerHoldEffect(battler2, TRUE);
    if ((holdEffectBattler2 == HOLD_EFFECT_QUICK_CLAW && gRandomTurnNumber < (0xFFFF * GetBattlerHoldEffectParam(battler2)) / 100)
     || (!IsAbilityOnOpposingSide(battler2, ABILITY_UNNERVE)
      && holdEffectBattler2 == HOLD_EFFECT_CUSTAP_BERRY
      && HasEnoughHpToEatBerry(battler2, 4, gBattleMons[battler2].item)))
        gProtectStructs[battler2].custap = TRUE;

    if (!ignoreChosenMoves)
    {
        if (gChosenActionByBattler[battler1] == B_ACTION_USE_MOVE)
            priority1 = GetChosenMovePriority(battler1);
        if (gChosenActionByBattler[battler2] == B_ACTION_USE_MOVE)
            priority2 = GetChosenMovePriority(battler2);
    }

    if (priority1 == priority2)
    {
        // QUICK CLAW / CUSTAP - always first
        // LAGGING TAIL - always last
        // STALL - always last

        if (gProtectStructs[battler1].custap && !gProtectStructs[battler2].custap)
            strikesFirst = 0;
        else if (gProtectStructs[battler2].custap && !gProtectStructs[battler1].custap)
            strikesFirst = 1;
        else if (holdEffectBattler1 == HOLD_EFFECT_LAGGING_TAIL && holdEffectBattler2 != HOLD_EFFECT_LAGGING_TAIL)
            strikesFirst = 1;
        else if (holdEffectBattler2 == HOLD_EFFECT_LAGGING_TAIL && holdEffectBattler1 != HOLD_EFFECT_LAGGING_TAIL)
            strikesFirst = 0;
        else if (GetBattlerAbility(battler1) == ABILITY_STALL && GetBattlerAbility(battler2) != ABILITY_STALL)
            strikesFirst = 1;
        else if (GetBattlerAbility(battler2) == ABILITY_STALL && GetBattlerAbility(battler1) != ABILITY_STALL)
            strikesFirst = 0;
        else
        {
            if (speedBattler1 == speedBattler2 && Random() & 1)
            {
                strikesFirst = 2; // same speeds, same priorities
            }
            else if (speedBattler1 < speedBattler2)
            {
                // battler2 has more speed
                if (gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
                    strikesFirst = 0;
                else
                    strikesFirst = 1;
            }
            else
            {
                // battler1 has more speed
                if (gFieldStatuses & STATUS_FIELD_TRICK_ROOM)
                    strikesFirst = 1;
                else
                    strikesFirst = 0;
            }
        }
    }
    else if (priority1 < priority2)
    {
        strikesFirst = 1; // battler2's move has greater priority
    }
    else
    {
        strikesFirst = 0; // battler1's move has greater priority
    }

    return strikesFirst;
}

static void SetActionsAndBattlersTurnOrder(void)
{
    s32 turnOrderId = 0;
    s32 i, j;

    if (gBattleTypeFlags & BATTLE_TYPE_SAFARI)
    {
        for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
        {
            gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[gActiveBattler];
            gBattlerByTurnOrder[turnOrderId] = gActiveBattler;
            turnOrderId++;
        }
    }
    else
    {
        if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        {
            for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
            {
                if (gChosenActionByBattler[gActiveBattler] == B_ACTION_RUN)
                {
                    turnOrderId = 5;
                    break;
                }
            }
        }
        else
        {
            if (gChosenActionByBattler[0] == B_ACTION_RUN)
            {
                gActiveBattler = 0;
                turnOrderId = 5;
            }
            if (gChosenActionByBattler[2] == B_ACTION_RUN)
            {
                gActiveBattler = 2;
                turnOrderId = 5;
            }
        }

        if (turnOrderId == 5) // One of battlers wants to run.
        {
            gActionsByTurnOrder[0] = gChosenActionByBattler[gActiveBattler];
            gBattlerByTurnOrder[0] = gActiveBattler;
            turnOrderId = 1;
            for (i = 0; i < gBattlersCount; i++)
            {
                if (i != gActiveBattler)
                {
                    gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[i];
                    gBattlerByTurnOrder[turnOrderId] = i;
                    turnOrderId++;
                }
            }
            gBattleMainFunc = CheckMegaEvolutionBeforeTurn;
            gBattleStruct->mega.battlerId = 0;
            return;
        }
        else
        {
            for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
            {
                if (gChosenActionByBattler[gActiveBattler] == B_ACTION_USE_ITEM
                  || gChosenActionByBattler[gActiveBattler] == B_ACTION_SWITCH
                  || gChosenActionByBattler[gActiveBattler] == B_ACTION_THROW_BALL)
                {
                    gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[gActiveBattler];
                    gBattlerByTurnOrder[turnOrderId] = gActiveBattler;
                    turnOrderId++;
                }
            }
            for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
            {
                if (gChosenActionByBattler[gActiveBattler] != B_ACTION_USE_ITEM
                  && gChosenActionByBattler[gActiveBattler] != B_ACTION_SWITCH
                  && gChosenActionByBattler[gActiveBattler] != B_ACTION_THROW_BALL)
                {
                    gActionsByTurnOrder[turnOrderId] = gChosenActionByBattler[gActiveBattler];
                    gBattlerByTurnOrder[turnOrderId] = gActiveBattler;
                    turnOrderId++;
                }
            }
            for (i = 0; i < gBattlersCount - 1; i++)
            {
                for (j = i + 1; j < gBattlersCount; j++)
                {
                    u8 battler1 = gBattlerByTurnOrder[i];
                    u8 battler2 = gBattlerByTurnOrder[j];
                    if (gActionsByTurnOrder[i] != B_ACTION_USE_ITEM
                        && gActionsByTurnOrder[j] != B_ACTION_USE_ITEM
                        && gActionsByTurnOrder[i] != B_ACTION_SWITCH
                        && gActionsByTurnOrder[j] != B_ACTION_SWITCH
                        && gActionsByTurnOrder[i] != B_ACTION_THROW_BALL
                        && gActionsByTurnOrder[j] != B_ACTION_THROW_BALL)
                    {
                        if (GetWhoStrikesFirst(battler1, battler2, FALSE))
                            SwapTurnOrder(i, j);
                    }
                }
            }
        }
    }
    gBattleMainFunc = CheckMegaEvolutionBeforeTurn;
    gBattleStruct->mega.battlerId = 0;
}

static void TurnValuesCleanUp(bool8 var0)
{
    s32 i;

    for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
    {
        if (var0)
        {
            gProtectStructs[gActiveBattler].protected = 0;
            gProtectStructs[gActiveBattler].spikyShielded = 0;
            gProtectStructs[gActiveBattler].kingsShielded = 0;
            gProtectStructs[gActiveBattler].banefulBunkered = 0;
        }
        else
        {
            memset(&gProtectStructs[gActiveBattler], 0, sizeof(struct ProtectStruct));

            if (gDisableStructs[gActiveBattler].isFirstTurn)
                gDisableStructs[gActiveBattler].isFirstTurn--;

            if (gDisableStructs[gActiveBattler].rechargeTimer)
            {
                gDisableStructs[gActiveBattler].rechargeTimer--;
                if (gDisableStructs[gActiveBattler].rechargeTimer == 0)
                    gBattleMons[gActiveBattler].status2 &= ~(STATUS2_RECHARGE);
            }
        }

        if (gDisableStructs[gActiveBattler].substituteHP == 0)
            gBattleMons[gActiveBattler].status2 &= ~(STATUS2_SUBSTITUTE);
    }

    gSideStatuses[0] &= ~(SIDE_STATUS_QUICK_GUARD | SIDE_STATUS_WIDE_GUARD | SIDE_STATUS_CRAFTY_SHIELD | SIDE_STATUS_MAT_BLOCK);
    gSideStatuses[1] &= ~(SIDE_STATUS_QUICK_GUARD | SIDE_STATUS_WIDE_GUARD | SIDE_STATUS_CRAFTY_SHIELD | SIDE_STATUS_MAT_BLOCK);
    gSideTimers[0].followmeTimer = 0;
    gSideTimers[1].followmeTimer = 0;
}

void SpecialStatusesClear(void)
{
    memset(&gSpecialStatuses, 0, sizeof(gSpecialStatuses));
}

static void CheckMegaEvolutionBeforeTurn(void)
{
    if (!(gHitMarker & HITMARKER_RUN))
    {
        while (gBattleStruct->mega.battlerId < gBattlersCount)
        {
            gActiveBattler = gBattlerAttacker = gBattleStruct->mega.battlerId;
            gBattleStruct->mega.battlerId++;
            if (gBattleStruct->mega.toEvolve & gBitTable[gActiveBattler]
                && !(gProtectStructs[gActiveBattler].noValidMoves))
            {
                gBattleStruct->mega.toEvolve &= ~(gBitTable[gActiveBattler]);
                gLastUsedItem = gBattleMons[gActiveBattler].item;
                if (gBattleStruct->mega.isWishMegaEvo == TRUE)
                    BattleScriptExecute(BattleScript_WishMegaEvolution);
                else
                    BattleScriptExecute(BattleScript_MegaEvolution);
                return;
            }
        }
    }

    gBattleMainFunc = CheckFocusPunch_ClearVarsBeforeTurnStarts;
    gBattleStruct->focusPunchBattlerId = 0;
}

static void CheckFocusPunch_ClearVarsBeforeTurnStarts(void)
{
    u32 i;

    if (!(gHitMarker & HITMARKER_RUN))
    {
        while (gBattleStruct->focusPunchBattlerId < gBattlersCount)
        {
            gActiveBattler = gBattlerAttacker = gBattleStruct->focusPunchBattlerId;
            gBattleStruct->focusPunchBattlerId++;
            if (!(gBattleMons[gActiveBattler].status1 & STATUS1_SLEEP)
                && !(gDisableStructs[gBattlerAttacker].truantCounter)
                && !(gProtectStructs[gActiveBattler].noValidMoves))
            {
                switch(gChosenMoveByBattler[gActiveBattler])
                {
                case MOVE_FOCUS_PUNCH:
                    BattleScriptExecute(BattleScript_FocusPunchSetUp);
                    return;
                case MOVE_BEAK_BLAST:
                    BattleScriptExecute(BattleScript_BeakBlastSetUp);
                    return;
                case MOVE_SHELL_TRAP:
                    BattleScriptExecute(BattleScript_ShellTrapSetUp);
                    return;
                }
            }
        }
    }

    gBattleMainFunc = CheckQuickClaw_CustapBerryActivation;
    gBattleStruct->quickClawBattlerId = 0;
}

static void CheckQuickClaw_CustapBerryActivation(void)
{
    u32 i;

    if (!(gHitMarker & HITMARKER_RUN))
    {
        while (gBattleStruct->quickClawBattlerId < gBattlersCount)
        {
            gActiveBattler = gBattlerAttacker = gBattleStruct->quickClawBattlerId;
            gBattleStruct->quickClawBattlerId++;
            if (gChosenActionByBattler[gActiveBattler] == B_ACTION_USE_MOVE
             && gChosenMoveByBattler[gActiveBattler] != MOVE_FOCUS_PUNCH   // quick claw message doesn't need to activate here
             && gProtectStructs[gActiveBattler].custap
             && !(gBattleMons[gActiveBattler].status1 & STATUS1_SLEEP)
             && !(gDisableStructs[gBattlerAttacker].truantCounter)
             && !(gProtectStructs[gActiveBattler].noValidMoves))
            {
                gProtectStructs[gActiveBattler].custap = FALSE;
                gLastUsedItem = gBattleMons[gActiveBattler].item;
                if (GetBattlerHoldEffect(gActiveBattler, FALSE) == HOLD_EFFECT_CUSTAP_BERRY)
                {
                    // don't record berry since its gone now
                    BattleScriptExecute(BattleScript_CustapBerryActivation);
                }
                else
                {
                    RecordItemEffectBattle(gActiveBattler, GetBattlerHoldEffect(gActiveBattler, FALSE));
                    BattleScriptExecute(BattleScript_QuickClawActivation);
                }
                return;
            }
        }
    }
    
    // setup stuff before turns/actions
    TryClearRageAndFuryCutter();
    gCurrentTurnActionNumber = 0;
    gCurrentActionFuncId = gActionsByTurnOrder[0];
    gBattleStruct->dynamicMoveType = 0;
    for (i = 0; i < MAX_BATTLERS_COUNT; i++)
    {
        gBattleStruct->ateBoost[i] = FALSE;
        gSpecialStatuses[i].gemBoost = FALSE;
    }

    gBattleMainFunc = RunTurnActionsFunctions;
    gBattleCommunication[3] = 0;
    gBattleCommunication[4] = 0;
    gBattleScripting.multihitMoveEffect = 0;
    gBattleResources->battleScriptsStack->size = 0;
}

static void RunTurnActionsFunctions(void)
{
    if (gBattleOutcome != 0)
        gCurrentActionFuncId = B_ACTION_FINISHED;

    *(&gBattleStruct->savedTurnActionNumber) = gCurrentTurnActionNumber;
    sTurnActionsFuncsTable[gCurrentActionFuncId]();

    if (gCurrentTurnActionNumber >= gBattlersCount) // everyone did their actions, turn finished
    {
        gHitMarker &= ~(HITMARKER_x100000);
        gBattleMainFunc = sEndTurnFuncsTable[gBattleOutcome & 0x7F];
    }
    else
    {
        if (gBattleStruct->savedTurnActionNumber != gCurrentTurnActionNumber) // action turn has been done, clear hitmarker bits for another battlerId
        {
            gHitMarker &= ~(HITMARKER_NO_ATTACKSTRING);
            gHitMarker &= ~(HITMARKER_UNABLE_TO_USE_MOVE);
        }
    }
}

static void HandleEndTurn_BattleWon(void)
{
    gCurrentActionFuncId = 0;

    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
    {
        gSpecialVar_Result = gBattleOutcome;
        gBattleTextBuff1[0] = gBattleOutcome;
        gBattlerAttacker = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
        gBattlescriptCurrInstr = BattleScript_LinkBattleWonOrLost;
        gBattleOutcome &= ~(B_OUTCOME_LINK_BATTLE_RAN);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER
            && gBattleTypeFlags & (BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_EREADER_TRAINER))
    {
        BattleStopLowHpSound();
        gBattlescriptCurrInstr = BattleScript_FrontierTrainerBattleWon;

        if (gTrainerBattleOpponent_A == TRAINER_FRONTIER_BRAIN)
            PlayBGM(MUS_VICTORY_GYM_LEADER);
        else
            PlayBGM(MUS_VICTORY_TRAINER);
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER && !(gBattleTypeFlags & BATTLE_TYPE_LINK))
    {
        BattleStopLowHpSound();
        gBattlescriptCurrInstr = BattleScript_LocalTrainerBattleWon;

        switch (gTrainers[gTrainerBattleOpponent_A].trainerClass)
        {
        case TRAINER_CLASS_ELITE_FOUR:
        case TRAINER_CLASS_CHAMPION:
            PlayBGM(MUS_VICTORY_LEAGUE);
            break;
        case TRAINER_CLASS_TEAM_AQUA:
        case TRAINER_CLASS_TEAM_MAGMA:
        case TRAINER_CLASS_AQUA_ADMIN:
        case TRAINER_CLASS_AQUA_LEADER:
        case TRAINER_CLASS_MAGMA_ADMIN:
        case TRAINER_CLASS_MAGMA_LEADER:
            PlayBGM(MUS_VICTORY_AQUA_MAGMA);
            break;
        case TRAINER_CLASS_LEADER:
            PlayBGM(MUS_VICTORY_GYM_LEADER);
            break;
        default:
            PlayBGM(MUS_VICTORY_TRAINER);
            break;
        }
    }
    else
    {
        gBattlescriptCurrInstr = BattleScript_PayDayMoneyAndPickUpItems;
    }

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_BattleLost(void)
{
    gCurrentActionFuncId = 0;

    if (gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED_LINK))
    {
        if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER)
        {
            if (gBattleOutcome & B_OUTCOME_LINK_BATTLE_RAN)
            {
                gBattlescriptCurrInstr = BattleScript_PrintPlayerForfeitedLinkBattle;
                gBattleOutcome &= ~(B_OUTCOME_LINK_BATTLE_RAN);
                gSaveBlock2Ptr->frontier.disableRecordBattle = TRUE;
            }
            else
            {
                gBattlescriptCurrInstr = BattleScript_FrontierLinkBattleLost;
                gBattleOutcome &= ~(B_OUTCOME_LINK_BATTLE_RAN);
            }
        }
        else
        {
            gBattleTextBuff1[0] = gBattleOutcome;
            gBattlerAttacker = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
            gBattlescriptCurrInstr = BattleScript_LinkBattleWonOrLost;
            gBattleOutcome &= ~(B_OUTCOME_LINK_BATTLE_RAN);
        }
    }
    else
    {
        gBattlescriptCurrInstr = BattleScript_LocalBattleLost;
    }

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_RanFromBattle(void)
{
    gCurrentActionFuncId = 0;

    if (gBattleTypeFlags & BATTLE_TYPE_FRONTIER && gBattleTypeFlags & BATTLE_TYPE_TRAINER)
    {
        gBattlescriptCurrInstr = BattleScript_PrintPlayerForfeited;
        gBattleOutcome = B_OUTCOME_FORFEITED;
        gSaveBlock2Ptr->frontier.disableRecordBattle = TRUE;
    }
    else if (gBattleTypeFlags & BATTLE_TYPE_TRAINER_HILL)
    {
        gBattlescriptCurrInstr = BattleScript_PrintPlayerForfeited;
        gBattleOutcome = B_OUTCOME_FORFEITED;
    }
    else
    {
        switch (gProtectStructs[gBattlerAttacker].fleeFlag)
        {
        default:
            gBattlescriptCurrInstr = BattleScript_GotAwaySafely;
            break;
        case 1:
            gBattlescriptCurrInstr = BattleScript_SmokeBallEscape;
            break;
        case 2:
            gBattlescriptCurrInstr = BattleScript_RanAwayUsingMonAbility;
            break;
        }
    }

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_MonFled(void)
{
    gCurrentActionFuncId = 0;

    PREPARE_MON_NICK_BUFFER(gBattleTextBuff1, gBattlerAttacker, gBattlerPartyIndexes[gBattlerAttacker]);
    gBattlescriptCurrInstr = BattleScript_WildMonFled;

    gBattleMainFunc = HandleEndTurn_FinishBattle;
}

static void HandleEndTurn_FinishBattle(void)
{
    u32 i;

    if (gCurrentActionFuncId == B_ACTION_TRY_FINISH || gCurrentActionFuncId == B_ACTION_FINISHED)
    {
        if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK
                                  | BATTLE_TYPE_RECORDED_LINK
                                  | BATTLE_TYPE_FIRST_BATTLE
                                  | BATTLE_TYPE_SAFARI
                                  | BATTLE_TYPE_EREADER_TRAINER
                                  | BATTLE_TYPE_WALLY_TUTORIAL
                                  | BATTLE_TYPE_FRONTIER)))
        {
            for (gActiveBattler = 0; gActiveBattler < gBattlersCount; gActiveBattler++)
            {
                if (GetBattlerSide(gActiveBattler) == B_SIDE_PLAYER)
                {
                    if (gBattleResults.playerMon1Species == SPECIES_NONE)
                    {
                        gBattleResults.playerMon1Species = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_SPECIES, NULL);
                        GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_NICKNAME, gBattleResults.playerMon1Name);
                    }
                    else
                    {
                        gBattleResults.playerMon2Species = GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_SPECIES, NULL);
                        GetMonData(&gPlayerParty[gBattlerPartyIndexes[gActiveBattler]], MON_DATA_NICKNAME, gBattleResults.playerMon2Name);
                    }
                }
            }
            PutPokemonTodayCaughtOnAir();
        }

        if (!(gBattleTypeFlags & (BATTLE_TYPE_LINK
                                  | BATTLE_TYPE_RECORDED_LINK
                                  | BATTLE_TYPE_TRAINER
                                  | BATTLE_TYPE_FIRST_BATTLE
                                  | BATTLE_TYPE_SAFARI
                                  | BATTLE_TYPE_FRONTIER
                                  | BATTLE_TYPE_EREADER_TRAINER
                                  | BATTLE_TYPE_WALLY_TUTORIAL))
            && gBattleResults.shinyWildMon)
        {
            sub_80EE184();
        }

        sub_8186444();
        BeginFastPaletteFade(3);
        FadeOutMapMusic(5);
        for (i = 0; i < PARTY_SIZE; i++)
        {
            UndoMegaEvolution(i);
            UndoFormChange(i, B_SIDE_PLAYER);
        }
        gBattleMainFunc = FreeResetData_ReturnToOvOrDoEvolutions;
        gCB2_AfterEvolution = BattleMainCB2;
    }
    else
    {
        if (gBattleControllerExecFlags == 0)
            gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
    }
}

static void FreeResetData_ReturnToOvOrDoEvolutions(void)
{
    if (!gPaletteFade.active)
    {
        gIsFishingEncounter = FALSE;
        gIsSurfingEncounter = FALSE;
        ResetSpriteData();
        if (gLeveledUpInBattle && (gBattleOutcome == B_OUTCOME_WON || gBattleOutcome == B_OUTCOME_CAUGHT))
        {
            gBattleMainFunc = TryEvolvePokemon;
        }
        else
        {
            gBattleMainFunc = ReturnFromBattleToOverworld;
            return;
        }
    }

    FreeAllWindowBuffers();
    if (!(gBattleTypeFlags & BATTLE_TYPE_LINK))
    {
        FreeMonSpritesGfx();
        FreeBattleResources();
        FreeBattleSpritesData();
    }
}

static void TryEvolvePokemon(void)
{
    s32 i;
    bool8 canStopEvo = TRUE;

    while (gLeveledUpInBattle != 0)
    {
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (gLeveledUpInBattle & gBitTable[i])
            {
                u16 species;
                u8 levelUpBits = gLeveledUpInBattle;

                levelUpBits &= ~(gBitTable[i]);
                gLeveledUpInBattle = levelUpBits;

                species = GetEvolutionTargetSpecies(&gPlayerParty[i], EVO_MODE_NORMAL, levelUpBits, SPECIES_NONE);

                // handle forcing evo for speedchoice.
                if(CheckSpeedchoiceOption(EVO_EVERY_LEVEL, EVO_EV_OFF) == FALSE)
                    canStopEvo = FALSE;
                else
                    canStopEvo = TRUE;

                if (species != SPECIES_NONE)
                {
                    FreeAllWindowBuffers();
                    gBattleMainFunc = WaitForEvoSceneToFinish;
                    EvolutionScene(&gPlayerParty[i], species, canStopEvo, i);
                    return;
                }
            }
        }
    }

    gBattleMainFunc = ReturnFromBattleToOverworld;
}

static void WaitForEvoSceneToFinish(void)
{
    if (gMain.callback2 == BattleMainCB2)
        gBattleMainFunc = TryEvolvePokemon;
}

static void ReturnFromBattleToOverworld(void)
{
    if (!(gBattleTypeFlags & BATTLE_TYPE_LINK))
    {
        RandomlyGivePartyPokerus(gPlayerParty);
        PartySpreadPokerus(gPlayerParty);
    }

    if (gBattleTypeFlags & BATTLE_TYPE_LINK && gReceivedRemoteLinkPlayers != 0)
        return;

    gSpecialVar_Result = gBattleOutcome;
    gMain.inBattle = 0;
    gMain.callback1 = gPreBattleCallback1;

    if (gBattleTypeFlags & BATTLE_TYPE_ROAMER)
    {
        UpdateRoamerHPStatus(&gEnemyParty[0]);
        if ((gBattleOutcome & B_OUTCOME_WON) || gBattleOutcome == B_OUTCOME_CAUGHT)
            SetRoamerInactive();
    }

    m4aSongNumStop(SE_LOW_HEALTH);
    SetMainCallback2(gMain.savedCallback);
}

void RunBattleScriptCommands_PopCallbacksStack(void)
{
    if (gCurrentActionFuncId == B_ACTION_TRY_FINISH || gCurrentActionFuncId == B_ACTION_FINISHED)
    {
        if (gBattleResources->battleCallbackStack->size != 0)
            gBattleResources->battleCallbackStack->size--;
        gBattleMainFunc = gBattleResources->battleCallbackStack->function[gBattleResources->battleCallbackStack->size];
    }
    else
    {
        if (gBattleControllerExecFlags == 0)
            gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
    }
}

void RunBattleScriptCommands(void)
{
    if (gBattleControllerExecFlags == 0)
        gBattleScriptingCommandsTable[gBattlescriptCurrInstr[0]]();
}

void SetTypeBeforeUsingMove(u16 move, u8 battlerAtk)
{
    u32 moveType, ateType, attackerAbility;

    if (move == MOVE_STRUGGLE)
        return;

    gBattleStruct->dynamicMoveType = 0;
    gBattleStruct->ateBoost[battlerAtk] = 0;
    gSpecialStatuses[battlerAtk].gemBoost = 0;

    if (gBattleMoves[move].effect == EFFECT_WEATHER_BALL)
    {
        if (WEATHER_HAS_EFFECT)
        {
            if (gBattleWeather & WEATHER_RAIN_ANY)
                gBattleStruct->dynamicMoveType = TYPE_WATER | 0x80;
            else if (gBattleWeather & WEATHER_SANDSTORM_ANY)
                gBattleStruct->dynamicMoveType = TYPE_ROCK | 0x80;
            else if (gBattleWeather & WEATHER_SUN_ANY)
                gBattleStruct->dynamicMoveType = TYPE_FIRE | 0x80;
            else if (gBattleWeather & WEATHER_HAIL_ANY)
                gBattleStruct->dynamicMoveType = TYPE_ICE | 0x80;
            else
                gBattleStruct->dynamicMoveType = TYPE_NORMAL | 0x80;
        }
    }
    else if (gBattleMoves[move].effect == EFFECT_HIDDEN_POWER)
    {
        u8 typeBits  = ((gBattleMons[battlerAtk].hpIV & 1) << 0)
                     | ((gBattleMons[battlerAtk].attackIV & 1) << 1)
                     | ((gBattleMons[battlerAtk].defenseIV & 1) << 2)
                     | ((gBattleMons[battlerAtk].speedIV & 1) << 3)
                     | ((gBattleMons[battlerAtk].spAttackIV & 1) << 4)
                     | ((gBattleMons[battlerAtk].spDefenseIV & 1) << 5);

        gBattleStruct->dynamicMoveType = (15 * typeBits) / 63 + 1;
        if (gBattleStruct->dynamicMoveType >= TYPE_MYSTERY)
            gBattleStruct->dynamicMoveType++;
        gBattleStruct->dynamicMoveType |= 0xC0;
    }
    else if (gBattleMoves[move].effect == EFFECT_CHANGE_TYPE_ON_ITEM)
    {
        if (GetBattlerHoldEffect(battlerAtk, TRUE) == gBattleMoves[move].argument)
            gBattleStruct->dynamicMoveType = ItemId_GetSecondaryId(gBattleMons[battlerAtk].item) | 0x80;
    }
    else if (gBattleMoves[move].effect == EFFECT_REVELATION_DANCE)
    {
        if (gBattleMons[battlerAtk].type1 != TYPE_MYSTERY)
            gBattleStruct->dynamicMoveType = gBattleMons[battlerAtk].type1 | 0x80;
        else if (gBattleMons[battlerAtk].type2 != TYPE_MYSTERY)
            gBattleStruct->dynamicMoveType = gBattleMons[battlerAtk].type2 | 0x80;
        else if (gBattleMons[battlerAtk].type3 != TYPE_MYSTERY)
            gBattleStruct->dynamicMoveType = gBattleMons[battlerAtk].type3 | 0x80;
    }
    else if (gBattleMoves[move].effect == EFFECT_NATURAL_GIFT)
    {
        if (ItemId_GetPocket(gBattleMons[battlerAtk].item) == POCKET_BERRIES)
            gBattleStruct->dynamicMoveType = gNaturalGiftTable[ITEM_TO_BERRY(gBattleMons[battlerAtk].item)].type;
    }
    else if (gBattleMoves[move].effect == EFFECT_TERRAIN_PULSE)
    {
        if (gFieldStatuses & STATUS_FIELD_MISTY_TERRAIN)
            gBattleStruct->dynamicMoveType = TYPE_FAIRY | 0x80;
        else if (gFieldStatuses & STATUS_FIELD_ELECTRIC_TERRAIN)
            gBattleStruct->dynamicMoveType = TYPE_ELECTRIC | 0x80;
        else if (gFieldStatuses & STATUS_FIELD_PSYCHIC_TERRAIN)
            gBattleStruct->dynamicMoveType = TYPE_PSYCHIC | 0x80;
        else if (gFieldStatuses & STATUS_FIELD_GRASSY_TERRAIN)
            gBattleStruct->dynamicMoveType = TYPE_GRASS | 0x80;
    }

    attackerAbility = GetBattlerAbility(battlerAtk);
    GET_MOVE_TYPE(move, moveType);
    if ((gFieldStatuses & STATUS_FIELD_ION_DELUGE && moveType == TYPE_NORMAL)
        || gStatuses3[battlerAtk] & STATUS3_ELECTRIFIED)
    {
        gBattleStruct->dynamicMoveType = 0x80 | TYPE_ELECTRIC;
    }
    else if (gBattleMoves[move].type == TYPE_NORMAL
             && gBattleMoves[move].effect != EFFECT_HIDDEN_POWER
             && gBattleMoves[move].effect != EFFECT_WEATHER_BALL
             && gBattleMoves[move].effect != EFFECT_CHANGE_TYPE_ON_ITEM
             && gBattleMoves[move].effect != EFFECT_NATURAL_GIFT
             && ((attackerAbility == ABILITY_PIXILATE && (ateType = TYPE_FAIRY))
                 || (attackerAbility == ABILITY_REFRIGERATE && (ateType = TYPE_ICE))
                 || (attackerAbility == ABILITY_AERILATE && (ateType = TYPE_FLYING))
                 || ((attackerAbility == ABILITY_GALVANIZE) && (ateType = TYPE_ELECTRIC))
                )
             )
    {
        gBattleStruct->dynamicMoveType = 0x80 | ateType;
        gBattleStruct->ateBoost[battlerAtk] = 1;
    }
    else if (gBattleMoves[move].type != TYPE_NORMAL
             && gBattleMoves[move].effect != EFFECT_HIDDEN_POWER
             && gBattleMoves[move].effect != EFFECT_WEATHER_BALL
             && attackerAbility == ABILITY_NORMALIZE)
    {
        gBattleStruct->dynamicMoveType = 0x80 | TYPE_NORMAL;
        gBattleStruct->ateBoost[battlerAtk] = 1;
    }
    else if (gBattleMoves[move].flags & FLAG_SOUND
             && attackerAbility == ABILITY_LIQUID_VOICE)
    {
        gBattleStruct->dynamicMoveType = 0x80 | TYPE_WATER;
    }

    // Check if a gem should activate.
    GET_MOVE_TYPE(move, moveType);
    if (GetBattlerHoldEffect(battlerAtk, TRUE) == HOLD_EFFECT_GEMS
        && moveType == ItemId_GetSecondaryId(gBattleMons[battlerAtk].item))
    {
        gSpecialStatuses[battlerAtk].gemParam = GetBattlerHoldEffectParam(battlerAtk);
        gSpecialStatuses[battlerAtk].gemBoost = 1;
    }
}

// special to set a field's totem boost(s)
// inputs:
//  var8000: battlerId
//  var8001 - var8007: stat changes
void SetTotemBoost(void)
{
    u8 battlerId = gSpecialVar_0x8000;
    u8 i;

    for (i = 0; i < (NUM_BATTLE_STATS - 1); i++)
    {
        if (*(&gSpecialVar_0x8001 + i))
        {
            gTotemBoosts[battlerId].stats |= (1 << i);
            gTotemBoosts[battlerId].statChanges[i] = *(&gSpecialVar_0x8001 + i);
            gTotemBoosts[battlerId].stats |= 0x80;  // used as a flag for the "totem flared to life" script
        }
    }
}
