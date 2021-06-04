#include "Player.h"
#include "Board.h"
#include "InputIncludes.h"


IMPLEMENT_CLASS_INFO(Player)

Player::Player(const kstl::string& name, CLASS_NAME_TREE_ARG) : CharacterBase(name, PASS_CLASS_NAME_TREE_ARG)
{

}

void	Player::InitModifiable()
{
	ParentClassType::InitModifiable();
	if (IsInit())
	{
		// graphic representation first
		mGraphicRepresentation = KigsCore::GetInstanceOf("pacman", "UIImage");
		mGraphicRepresentation->setValue("TextureName", "Pacman.json");
		mGraphicRepresentation->setValue("Anchor", v2f(0.5f, 0.5f));
		mGraphicRepresentation->setValue("Priority", 25);
		mGraphicRepresentation->setValue("CurrentAnimation", "pacman");
		mGraphicRepresentation->setValue("Loop", "true");
		mGraphicRepresentation->setValue("FramePerSecond", "3");
		mBoard->getGraphicInterface()->addItem(mGraphicRepresentation);
		mGraphicRepresentation->Init();

		setCurrentPos(v2f(13.0f, 23.0f));

		auto theInputModule = KigsCore::GetModule<ModuleInput>();
		KeyboardDevice* theKeyboard = theInputModule->GetKeyboard();
		KigsCore::Connect(theKeyboard, "KeyboardEvent", this, "UpdateKeyboard");
	}
}

void Player::UpdateKeyboard(std::vector<KeyEvent>& keys)
{
	if (!keys.empty())
	{
		for (auto& key : keys)
		{
			if (key.Action != key.ACTION_DOWN)
				continue;
			
			if (key.KeyCode == VK_LEFT)//Left
			{
				printf("");
			}
			else if (key.KeyCode == VK_RIGHT)//Right
			{
				printf("");
			}
			else if (key.KeyCode == VK_UP)//Left
			{
				printf("");
			}
			else if (key.KeyCode == VK_DOWN)//Right
			{
				printf("");
			}
			
		}
		
	}
}
