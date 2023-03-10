SlateportCity_PokemonFanClub_EventScript_SwaggerTutor:: @ 82C7F16
	lock
	faceplayer
	checkspeedchoice EASY_FALSE_SWIPE, EASY_FALSE_SWIPE_TUTOR
	goto_if TRUE, SpeedchoiceSlateportEasyFalseSwipe
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_SWAGGER, MoveTutor_EventScript_SwaggerTaught
gMoveTutorText_20::
	msgbox MoveTutor_Text_SwaggerTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_SwaggerDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_SwaggerDeclined
gMoveTutorText_21::
	msgbox MoveTutor_Text_SwaggerWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_SWAGGER
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_SwaggerDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_SWAGGER
	goto MoveTutor_EventScript_SwaggerTaught
	end

SpeedchoiceSlateportEasyFalseSwipe:: @ SPEEDCHOICE
	msgbox SlateportCity_PokemonFanClub_Text_SPEEDCHOICE_FALSE_SWIPE, MSGBOX_YESNO
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_SwaggerDeclined
	msgbox SlateportCity_PokemonFanClub_Text_SPEEDCHOICE_FALSE_SWIPE2, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_FALSE_SWIPE
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_SwaggerDeclined
	msgbox SlateportCity_PokemonFanClub_Text_SPEEDCHOICE_FALSE_SWIPE3, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_SwaggerDeclined:: @ 82C7F6A
	msgbox MoveTutor_Text_SwaggerDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_SwaggerTaught:: @ 82C7F74
gMoveTutorText_22::
	msgbox MoveTutor_Text_SwaggerTaught, MSGBOX_DEFAULT
	release
	end

MauvilleCity_EventScript_RolloutTutor:: @ 82C7F7E
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_ROLLOUT, MoveTutor_EventScript_RolloutTaught
gMoveTutorText_14::
	msgbox MoveTutor_Text_RolloutTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_RolloutDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_RolloutDeclined
	msgbox MoveTutor_Text_RolloutWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_ROLLOUT
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_RolloutDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_ROLLOUT
	goto MoveTutor_EventScript_RolloutTaught
	end

MoveTutor_EventScript_RolloutDeclined:: @ 82C7FD2
	msgbox MoveTutor_Text_RolloutDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_RolloutTaught:: @ 82C7FDC
	msgbox MoveTutor_Text_RolloutTaught, MSGBOX_DEFAULT
	release
	end

VerdanturfTown_PokemonCenter_1F_EventScript_FuryCutterTutor:: @ 82C7FE6
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_FURY_CUTTER, MoveTutor_EventScript_FuryCutterTaught
gMoveTutorText_8::
	msgbox MoveTutor_Text_FuryCutterTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_FuryCutterDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_FuryCutterDeclined
	msgbox MoveTutor_Text_FuryCutterWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_FURY_CUTTER
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_FuryCutterDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_FURY_CUTTER
	goto MoveTutor_EventScript_FuryCutterTaught
	end

MoveTutor_EventScript_FuryCutterDeclined:: @ 82C803A
	msgbox MoveTutor_Text_FuryCutterDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_FuryCutterTaught:: @ 82C8044
	msgbox MoveTutor_Text_FuryCutterTaught, MSGBOX_DEFAULT
	release
	end

LavaridgeTown_House_EventScript_MimicTutor:: @ 82C804E
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_MIMIC, MoveTutor_EventScript_MimicTaught
gMoveTutorText_11::
	msgbox MoveTutor_MimicTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_MimicDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_MimicDeclined
	msgbox MoveTutor_Text_MimicWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_MIMIC
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_MimicDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_MIMIC
	goto MoveTutor_EventScript_MimicTaught
	end

MoveTutor_EventScript_MimicDeclined:: @ 82C80A2
gMoveTutorText_13::
	msgbox MoveTutor_MimicDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_MimicTaught:: @ 82C80AC
gMoveTutorText_12::
	msgbox MoveTutor_Text_MimicTaught, MSGBOX_DEFAULT
	release
	end

FallarborTown_Mart_EventScript_MetronomeTutor:: @ 82C80B6
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_METRONOME, MoveTutor_EventScript_MetronomeTaught
gMoveTutorText_9::
	msgbox MoveTutor_Text_MetronomeTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_MetronomeDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_MetronomeDeclined
	msgbox MoveTutor_Text_MetronomeWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_METRONOME
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_MetronomeDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_METRONOME
	goto MoveTutor_EventScript_MetronomeTaught
	end

MoveTutor_EventScript_MetronomeDeclined:: @ 82C810A
	msgbox MoveTutor_Text_MetronomeDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_MetronomeTaught:: @ 82C8114
gMoveTutorText_10::
	msgbox MoveTutor_Text_MetronomeTaught, MSGBOX_DEFAULT
	release
	end

FortreeCity_House2_EventScript_SleepTalkTutor:: @ 82C811E
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_SLEEP_TALK, MoveTutor_EventScript_SleepTalkTaught
gMoveTutorText_15::
	msgbox MoveTutor_Text_SleepTalkTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_SleepTalkDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_SleepTalkDeclined
	msgbox MoveTutor_Text_SleepTalkWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_SLEEP_TALK
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_SleepTalkDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_SLEEP_TALK
	goto MoveTutor_EventScript_SleepTalkTaught
	end

MoveTutor_EventScript_SleepTalkDeclined:: @ 82C8172
	msgbox MoveTutor_Text_SleepTalkDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_SleepTalkTaught:: @ 82C817C
gMoveTutorText_16::
	msgbox MoveTutor_Text_SleepTalkTaught, MSGBOX_DEFAULT
	release
	end

LilycoveCity_DepartmentStoreRooftop_EventScript_SubstituteTutor:: @ 82C8186
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_SUBSTITUTE, MoveTutor_EventScript_SubstituteTaught
gMoveTutorText_17::
	msgbox MoveTutor_Text_SubstituteTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_SubstituteDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_SubstituteDeclined
gMoveTutorText_18::
	msgbox MoveTutor_Text_SubstituteWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_SUBSTITUTE
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_SubstituteDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_SUBSTITUTE
	goto MoveTutor_EventScript_SubstituteTaught
	end

MoveTutor_EventScript_SubstituteDeclined:: @ 82C81DA
gMoveTutorText_19::
	msgbox MoveTutor_Text_SubstituteDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_SubstituteTaught:: @ 82C81E4
	msgbox MoveTutor_Text_SubstituteTaught, MSGBOX_DEFAULT
	release
	end

MossdeepCity_EventScript_DynamicPunchTutor:: @ 82C81EE
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_DYNAMICPUNCH, MoveTutor_EventScript_DynamicPunchTaught
gMoveTutorText_3::
	msgbox MoveTutor_Text_DynamicPunchTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_DynamicPunchDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_DynamicPunchDeclined
	msgbox MoveTutor_Text_DynamicPunchWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_DYNAMIC_PUNCH
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_DynamicPunchDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_DYNAMICPUNCH
	goto MoveTutor_EventScript_DynamicPunchTaught
	end

MoveTutor_EventScript_DynamicPunchDeclined:: @ 82C8242
	msgbox MoveTutor_Text_DynamicPunchDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_DynamicPunchTaught:: @ 82C824C
gMoveTutorText_4::
	msgbox MoveTutor_Text_DynamicPunchTaught, MSGBOX_DEFAULT
	release
	end

SootopolisCity_PokemonCenter_1F_EventScript_DoubleEdgeTutor:: @ 82C8256
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_DOUBLE_EDGE, MoveTutor_EventScript_DoubleEdgeTaught
gMoveTutorText_1::
	msgbox MoveTutor_Text_DoubleEdgeTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_DoubleEdgeDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_DoubleEdgeDeclined
gMoveTutorText_2::
	msgbox MoveTutor_Text_DoubleEdgeWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_DOUBLE_EDGE
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_DoubleEdgeDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_DOUBLE_EDGE
	goto MoveTutor_EventScript_DoubleEdgeTaught
	end

MoveTutor_EventScript_DoubleEdgeDeclined:: @ 82C82AA
	msgbox MoveTutor_Text_DoubleEdgeDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_DoubleEdgeTaught:: @ 82C82B4
	msgbox MoveTutor_Text_DoubleEdgeTaught, MSGBOX_DEFAULT
	release
	end

PacifidlogTown_PokemonCenter_1F_EventScript_ExplosionTutor:: @ 82C82BE
	lock
	faceplayer
	goto_if_set FLAG_MOVE_TUTOR_TAUGHT_EXPLOSION, MoveTutor_EventScript_ExplosionTaught
gMoveTutorText_5::
	msgbox MoveTutor_Text_ExplosionTeach, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_ExplosionDeclined
	call MoveTutor_EventScript_CanOnlyBeLearnedOnce
	compare VAR_RESULT, NO
	goto_if_eq MoveTutor_EventScript_ExplosionDeclined
gMoveTutorText_6::
	msgbox MoveTutor_Text_ExplosionWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8005, TUTOR_MOVE_EXPLOSION
	call MoveTutor_EventScript_OpenPartyMenu
	compare VAR_RESULT, 0
	goto_if_eq MoveTutor_EventScript_ExplosionDeclined
	setflag FLAG_MOVE_TUTOR_TAUGHT_EXPLOSION
	goto MoveTutor_EventScript_ExplosionTaught
	end

MoveTutor_EventScript_ExplosionDeclined:: @ 82C8312
	msgbox MoveTutor_Text_ExplosionDeclined, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_ExplosionTaught:: @ 82C831C
gMoveTutorText_7::
	msgbox MoveTutor_Text_ExplosionTaught, MSGBOX_DEFAULT
	release
	end

MoveTutor_EventScript_OpenPartyMenu:: @ 82C8326
	special ChooseMonForMoveTutor
	waitstate
	lock
	faceplayer
	return

MoveTutor_EventScript_CanOnlyBeLearnedOnce:: @ 82C832D
	msgbox MoveTutor_Text_ThisMoveCanOnlyBeLearnedOnce, MSGBOX_YESNO
	return

SlateportCity_PokemonFanClub_Text_SPEEDCHOICE_FALSE_SWIPE: @ SPEEDCHOICE
	.string "Want me to teach you FALSE SWIPE?$"

SlateportCity_PokemonFanClub_Text_SPEEDCHOICE_FALSE_SWIPE2: @ SPEEDCHOICE
	.string "All right, which POK??MON wants to\n"
	.string "learn how to FALSE SWIPE?$"

SlateportCity_PokemonFanClub_Text_SPEEDCHOICE_FALSE_SWIPE3: @ SPEECHOICE
	.string "Time for some easy catching.$"
