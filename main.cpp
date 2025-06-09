#include <kamek.h>

extern "C" void give_item(void);
extern "C" void post_swap_code(void);
extern "C" void check_moongate_open(void);
extern "C" void open_moongate(void);
extern "C" void unlock_door(void);
extern "C" void door_exitfunc(void);
extern "C" void draw_crystal(void);
extern "C" void draw_maiden(void);
extern "C" void no_quest_item(void);
extern "C" void has_quest_item(void);
extern "C" void has_spellbook(void);
extern "C" void ow_maiden_saved(void);
extern "C" void ow_maiden_not_saved(void);
extern "C" void current_world_in_level(void);
extern "C" void current_selected_world(void);
extern "C" void current_selected_stage(void);
extern "C" void select_stage(void);
extern "C" void play_sfx(void);
extern "C" void end_select(void);
extern "C" void value_for_sound_play(void);
extern "C" void total_maidens_required(void);

int ItemGotFromServer;
int ItemToGivePlayer;
int HasInvItem = 0xFFFF;
int HasInvL2 = 0xFFFF;
int HasItemSpecial; //Moon pearl, letter, spellbook, power bracelet
int CurLevelNum;
int SavedMaidens;
int UnlockedWorlds = 0xFF;
int TotalHearts = 0x0C;
int TotalGems;


char KeysPerLevel[24] = { //Set to 04 testing purposes; Reset to zero for full release. High bits = doors opened
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
};


kmBranchDefAsm(0x80241790, NULL) {  // Give Remote Item to Player
	CheckItemSwap:
	lis r4, 0x8053
	lbz r4, 0xea5f(r4)
	cmpwi r4, 0x10
	beq ItemSwap
	b GetItemFromServer
SwapFinish:
	stw r4, 0x0C10 (r3)
	b CheckIfItemShouldLevel
ItemSwap:
	lwz r4,0x0C10 (r3)
CheckNext:
	addi r4, r4, 1
	cmpwi r4, 11
	beq RollOver
	li r6, 1
	slw r6, r6, r4
	lis r5, HasInvItem@ha
	addi r5, r5, HasInvItem@l
	lwz r5, 0(r5)
	and r6, r6, r5
	cmpwi r6, 0
	bne SwapFinish
	b CheckNext
RollOver:
	li r4, 0
	b SwapFinish
CheckIfItemShouldLevel:
	lwz r4, 0x0C10(r3)
	cmpwi r4, 0
	beq GetItemFromServer
	li r6, 1
	slw r6, r6, r4
	lis r5, HasInvL2@ha
	addi r5, r5, HasInvL2@l
	lwz r5, 0(r5)
	and r6, r6, r5
	cmpwi r6, 0
	beq Level1
	li r4, 2
	stw r4, 0x0C14 (r3)
	b GetItemFromServer
Level1:
	li r4, 0
	stw r4, 0x0C14(r3)
GetItemFromServer:
	lwz r3, 0x1240(r31)
	cmpwi r3, 0x04
	bge noItem
	lwz r3, 0x0454(r31)
	cmpwi r3, 0
	bne noItem
	lis r3, ItemGotFromServer@ha
	lwz r4, ItemGotFromServer@l(r3)
	cmpwi r4, 0
	beq noItem
	addi r4, r4, -1
	stw r4, ItemToGivePlayer@l(r3)
	lis r3, ItemToGivePlayer@ha
	addi r3, r3, ItemToGivePlayer@l
	addi r3, r3, -0x0234
	bl give_item
	lis r3, ItemToGivePlayer@ha
	li r4, 0
	stw r4, ItemGotFromServer@l(r3)
noItem:
}

kmCallDefAsm(0x802EB7DC) {  // Override the Chest effect if server item	
	lis r4, ItemGotFromServer@ha
	lwz r0, ItemGotFromServer@l(r4)
	cmpwi r0, 0
	beq LoadNormal
	li r0, 1
	blr

	LoadNormal:
	lwz r0, 0x0238
}

kmBranchDefAsm(0x802E8894, NULL) {  // Check the Global Inv variable for the Moon Pearl's presence
	lis r3, HasItemSpecial@ha
	addi r3, r3, HasItemSpecial@l
	lwz r3, 0(r3)
	andi. r3, r3, 0x01
	cmpwi r3, 0
	beq MoongateClosed
	b open_moongate
MoongateClosed:
	b check_moongate_open
}

kmCallDefAsm(0x802C66C4) {  // Check for keys
	lwz r3, 0x0248(r31)
	cmpwi r3, 0xFFFF
	beq NobodyAtDoor
	lis r4, CurLevelNum@ha
	addi r4, r4, CurLevelNum@l
	lwz r5, 0(r4)// Get the ID of the level we're in
	lis r3, KeysPerLevel@ha
	addi r3, r3, KeysPerLevel@l
	lbzx r3, r3, r5
	mr r4, r3
	andi. r4, r4, 0xFFFFFFF0
	andi. r3, r3, 0x0000000F
	cmpwi r3, 0
	beq NoKeys
	addi r3, r3, -1
	add r3, r3, r4
	lis r4, KeysPerLevel@ha
	addi r4, r4, KeysPerLevel@l
	stbx r3, r4, r5
	b unlock_door
NoKeys:
NobodyAtDoor:
b door_exitfunc
}

kmBranchDefAsm(0x8044EA8C, NULL) {  // Check Current Maidens
	nofralloc
	li r30, 1
	slw r30, r30, r23
	lis r3, SavedMaidens@ha
	addi r3, r3, SavedMaidens@l
	lwz r3, 0(r3)
	and r3, r3, r30
	cmpwi r3, 0
	beq DrawCrystal
	b draw_maiden
DrawCrystal:
	b draw_crystal
}

kmBranchDefAsm(0x803840D4, NULL) {  // Check Quest Item
	nofralloc
	lis r3, HasItemSpecial@ha
	addi r3, r3, HasItemSpecial@l
	lwz r3, 0(r3)
	cmpwi r21, 0x01d8
	beq CheckSpellbook
	andi. r3, r3, 0x02
	cmpwi r3, 0
	beq ItemNotSet
	b has_quest_item
CheckSpellbook:
	andi. r3, r3, 0x04
	cmpwi r3, 0
	beq ItemNotSet
	b has_spellbook
ItemNotSet:
	subis r0, r3, 19269
	b no_quest_item
}

kmCallDefAsm(0x802544EC) {  // Give the player the Blue or Power Bracelet on map init
	lis r4, HasItemSpecial@ha
	addi r4, r4, HasItemSpecial@l
	lwz r0, 0(r4)
	andi. r0, r0, 0x08 // Check Blue Bracelet
	cmpwi r0, 0
	beq NoBB
	li r0, 1
	b HasBB
NoBB:
	li r0, 0
HasBB:
	stb r0, 0x0B7B (r31)
CheckPowerBracelet:
	lwz r0, 0(r4)
	andi. r0, r0, 0x10 // Check Power Bracelet
	cmpwi r0, 0
	beq NoPB
	li r0, 1
	b WritePB
NoPB:
	li r0, 0
WritePB:
	stb r0, 0x0B7A (r31)
}

kmBranchDefAsm(0x804112B4, NULL) {  // Display the crystal on the overworld based on maidens owned
	lis r4, 0x8113
	ori r4, r4, 0xB4DC
	lwz r4, 0(r4)
	li r5, 1
	slw r4, r5, r4
	lis r5, SavedMaidens@ha
	addi r5, r5, SavedMaidens@l
	lwz r5, 0(r5)
	and r4, r4, r5
	cmpwi r4, 0
	beq NoMaiden
	b ow_maiden_saved
NoMaiden:
	b ow_maiden_not_saved
}

kmCallDefAsm(0x8040DF8C) {  // Lock world movement based on Worlds Unlocked
CheckNext:
	li r5, 1
	slw r4, r5, r0 //Get the bit for the current level
	lis r5, UnlockedWorlds@ha
	addi r5, r5, UnlockedWorlds@l
	lwz r5, 0(r5)
	and r4, r4, r5
	cmpwi r4, 0
	beq CheckNextWorld
Exit:
	stw r0, 0x0340(r30)
	blr
CheckNextWorld:
	cmpwi r0, 9
	beq Exit
	mr r5, r0
	addi r5, r5, 1
	mr r0, r5
	b CheckNext
}

kmCallDefAsm(0x80450890) {  // Set collected Maiden as active
	lis r4, current_world_in_level@ha  
	addi r4, r4, current_world_in_level@l
	lwz r4, 0(r4)
	li r5, 1
	slw r4, r5, r4
	lis r5, SavedMaidens@ha
	addi r5, r5, SavedMaidens@l
	lwz r5, 0(r5)
	or r4, r4, r5
	lis r5, SavedMaidens@ha
	stw r4, SavedMaidens@l(r5)
	}

kmBranchDefAsm(0x8040AF14, NULL) {  // Block the Palace of Winds until the maiden requirement is clear
	nofralloc
	lis r3, current_selected_world@ha
	addi r3, r3, current_selected_world@l
	lwz r3, 0(r3)
	lis r4, current_selected_stage@ha
	addi r4, r4, current_selected_stage@l
	lwz r4, 0(r4)
	mulli r3, r3, 3
	add r3, r3, r4
	cmpwi r3, 0x17
	beq PalaceMaidenCheck
EnterPalace:
	lis r5, CurLevelNum@ha
	addi r5, r5, CurLevelNum@l
	stw r3, 0(r5)
	stfs  f31, 0x0394(r30)
	b select_stage
PalaceMaidenCheck:
	mr r0, r3
	lis r3, SavedMaidens@ha
	addi r3, r3, SavedMaidens@l
	lwz r3, 0(r3) // Load the Bitfield of Maidens the player has saved
	li r4, 0
	li r7, 0
BitCountLoop:
	li r5, 1
	slw r6, r5, r4 //Get the current bit in 6
	and r5, r6, r3 // 3 has the bitfield, 6 has the current bit
	cmpwi r5, 0
	beq MaidenBitNotSet
	addi r7, r7, 1 // Add 1 to the total maidens saved
MaidenBitNotSet:
	cmpwi r4, 6 // Have we checked all 7 bits?
	beq ExitBitLoop //If yes, end
	addi r4, r4, 1 //If no, add 1 and check again
	b BitCountLoop
ExitBitLoop:
	lis r5, total_maidens_required@ha
	addi r5, r5, total_maidens_required@l
	lwz r5, 0(r5)
	cmpw r7, r5
	blt PlayErrorAndCancel
	mr r3, r0
	b EnterPalace
PlayErrorAndCancel:
	li r4, 6
	lis r5, value_for_sound_play@ha
	addi r5, r5, value_for_sound_play@l
	mr r3, r5
	li r5, -1 // for reasons yet unknown the sound only plays if r3 and r5 have these values
	bl play_sfx
	b end_select
	}

kmCallDefAsm(0x802C3DE8) {  // Delete Key/Pearl when touched
	stb r0, 0x02B5(r25)
	li r4, 0
	stw r4, 0x011C(r25)
	}

kmCallDefAsm(0x802C4A60) {  // Delete Key/Pearl when picked up
	stb r0, 0x02B5(r31)
	li r0, 0
	stw r0, 0x011C(r31)
	}

kmCallDefAsm(0x802545AC) {  // Load Hearts and total Gems
	lis r4, TotalGems@ha
	addi r4, r4, TotalGems@l
	lwz r4, 0(r4)
	stw r4, 0x0C04(r31)
GetHearts:
	lis r4, TotalHearts@ha
	addi r4, r4, TotalHearts@l
	lwz r4, 0(r4)
	stw r4, 0x0BF8(r31)
	stw r4, 0x0BFC(r31) //set current hearts to max
	}

kmWrite32(0x80410170, 0x38000006); // Pull up save dialogue when exiting a level

kmWrite32(0x80288A94, 0x38000001); // Disable pressing the Z button to close gameboy windows (this is done since Z is the item swap now)

kmWrite32(0x8041409C, 0x60000000); // Open all level paths

kmWrite32(0x802C67B0, 0x60000000); //Make doors not check for the key's presence?

kmWrite32(0x803EC450, 0x60000000); //Skip the intro

kmWrite32(0x802C67C4, 0x60000000); //Prevent held item from getting deleted when unlocking something

kmWrite32(0x8038DF9C, 0x60000000); //Disable great fairy upgrades

kmWrite32(0x8040DF90, 0x38600008); //Force world map movement to assume 8 worlds; overwritten by unlock status

kmWrite32(0x8040DC98, 0x38000008); //Force world map movement to assume 8 worlds; overwritten by unlock status

// kmWrite32(0x80005530, 0x380000ff); //DOESNT WORK FIX SAVING
