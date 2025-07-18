#pragma once

enum ItemType : char {
	Hppotion = 0,
	Mppotion,
	Atkbuf,
	Defbuf
};
class ITEM
{
	ITEM(ItemType type);
public:
	void Use(float& hp);
private:
	ItemType itemtype;


};






