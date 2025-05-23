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

int ItemGotFromServer;
int ItemToGivePlayer;
int HasInvItem;
int HasInvL2;
int HasItemSpecial; //Moon pearl, letter, spellbook, power bracelet
int CurLevelNum;
int SavedMaidens;


char KeysPerLevel[24] = { //Set to 04 testing purposes; Reset to zero for full release. High bits = doors opened
	0x04, 0x00, 0x00,
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
	lis r6, 1
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


kmWrite32(0x80410170, 0x38000006); // Pull up save dialogue when exiting a level

kmWrite32(0x80288A94, 0x38000001); // Disable pressing the Z button to close gameboy windows

kmWrite32(0x8041409C, 0x60000000); // Open all level paths

kmWrite32(0x802C67B0, 0x60000000); //Make doors not check for the key's presence?

kmWrite32(0x803EC450, 0x60000000); //Skip the intro

// kmWrite32(0x80005530, 0x380000ff); //DOESNT WORK FIX SAVING
