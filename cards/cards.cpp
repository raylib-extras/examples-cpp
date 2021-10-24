/**********************************************************************************************
*
*   raylib-extras, examples-cpp * examples for Raylib in C++
*
*   pew * an example of movement and shots
*
*   LICENSE: MIT
*
*   Copyright (c) 2021 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <list>

Texture CardBack;

enum class CardSuit
{
	Circle,
	Square,
	Diamond,
	Cross,
};

// represents a single unique card
class Card
{
public:
	Vector2 Position;
	CardSuit Suit = CardSuit::Circle;
	int Value = 0;

	bool FaceUp = false;

	Color GetCardColor()
	{
		switch (Suit)
		{
		case CardSuit::Circle:
			return RED;
		case CardSuit::Square:
			return BLUE;
		default:
			return BLACK;
		}
	}

	Rectangle GetScreenRect()
	{
		return Rectangle{ Position.x , Position.y, (float)CardBack.width, (float)CardBack.height };
	}

	void Draw()
	{
		if (FaceUp)
		{
			Rectangle baseRect = Rectangle{ Position.x-1 , Position.y-1, (float)CardBack.width+2, (float)CardBack.height+2 };
			DrawRectangleRec(baseRect, BLACK);
			DrawRectangleRec(GetScreenRect(), RAYWHITE);

			Vector2 center = { Position.x + CardBack.width / 2, Position.y + CardBack.height / 2 };

			DrawText(TextFormat("%d", Value), (int)center.x, (int)center.y + 55, 40, GetCardColor());

			switch (Suit)
			{
			case CardSuit::Circle:
				DrawCircle((int)center.x, (int)center.y - 40, 40, GetCardColor());
				break;
			case CardSuit::Square:
				DrawRectangle((int)center.x-20, (int)center.y - 60, 40, 40, GetCardColor());
				break;
			}
		}
		else
		{
			DrawTexture(CardBack, (int)Position.x, (int)Position.y, WHITE);
		}
	}

	bool PointIn(const Vector2& pos)
	{
		return CheckCollisionPointRec(pos, GetScreenRect());
	}
};

// owns a set of unique cards
class Deck
{
public:
	std::vector<Card> Cards;

	Deck()
	{
		for (int i = 0; i < 20; i++)
		{
			Cards.emplace_back(Card{ Vector2Zero(),CardSuit::Circle,i + 1, false });
			Cards.emplace_back(Card{ Vector2Zero(),CardSuit::Square,i + 1, false });
		}
	}
};

// a stack of card pointers, used for things like draw decks and things that draw as card stack.
class Stack
{
public:
	Vector2 Pos = { 0 };

	std::vector<Card*> Cards;

	void FromDeck(Deck& deck)
	{
		Cards.clear();
		for (auto& card : deck.Cards)
			Cards.push_back(&card);
	}

	void Swap(size_t a, size_t b)
	{
		Card* t = Cards[a];
		Cards[a] = Cards[b];
		Cards[b] = t;
	}

	void Shuffle( size_t factor = 4)
	{
		size_t count = Cards.size() * factor;

		for (size_t i = 0; i < count; i++)
		{
			size_t a = (size_t)GetRandomValue(0, (int)Cards.size() - 1);
			size_t b = (size_t)GetRandomValue(0, (int)Cards.size() - 1);

			Swap(a, b);
		}
	}

	Card* PeekTop()
	{
		Card* topCard = nullptr;
		if (!Cards.empty())
			topCard = Cards[Cards.size() - 1];

		return topCard;
	}

	Card* PopTop()
	{
		if (Cards.empty())
			return nullptr;

		Card* topCard = Cards[Cards.size() - 1];
		Cards.erase(Cards.begin() + (Cards.size()-1));

		return topCard;
	}

	void Draw()
	{
		Card* topCard = PeekTop();

		Rectangle baseRect = Rectangle{ Pos.x,Pos.y, (float)CardBack.width, (float)CardBack.height };

		DrawRectangleRec(baseRect, DARKGRAY);

		if (topCard != nullptr)
		{
			topCard->Position.x = baseRect.x + 6;
			topCard->Position.y = baseRect.y - 6;
			topCard->Draw();
		}
	}
};

class Hand : public Stack
{
public:
	std::list<Card*> Cards;

	Card* SelectedCard = nullptr;

	void AddCard(Card* card, bool selected = true)
	{
		Cards.emplace_back(card);
		if (selected)
			Select(card);
	}

	void Select(Card* card)
	{
		Deselect();
		if (card == nullptr)
			return;

		SelectedCard = card;

		// make it look like we picked up the card
		SelectedCard->Position.x -= 5;
		SelectedCard->Position.y -= 5;
	}

	void Deselect()
	{
		if (SelectedCard != nullptr)
		{
			SelectedCard->Position.x += 10;
			SelectedCard->Position.y += 10;
		}
		SelectedCard = nullptr;
	}

	void Draw()
	{
		for (Card* card : Cards)
		{
			if (card != SelectedCard)
				card->Draw();
		}

		if (SelectedCard != nullptr)
		{

			Rectangle shadow = SelectedCard->GetScreenRect();
			shadow.x += 10;
			shadow.y += 10;

			DrawRectangleRec(shadow, ColorAlpha(BLACK, 0.5f));

			SelectedCard->Draw();
		}
	}
};

void main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Card Sample");
	SetTargetFPS(144);

	Image img = GenImageChecked(100, 200, 25, 25, RED, MAROON);
	CardBack = LoadTextureFromImage(img);
	UnloadImage(img);

	Deck cards;

	Stack DrawDeck{ 30,20 };

	DrawDeck.FromDeck(cards);
	DrawDeck.Shuffle();

	Hand PlayerHand;

	while (!WindowShouldClose())
	{
		bool handled = false;

		// check to see if we are dragging a card
		if (PlayerHand.SelectedCard != nullptr)
		{
			if (PlayerHand.SelectedCard->PointIn(GetMousePosition()))
			{
				if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
					PlayerHand.SelectedCard->Position = Vector2Add(PlayerHand.SelectedCard->Position, GetMouseDelta());
				else
					PlayerHand.Deselect();
				handled = true;
			}
		}
		else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) // check to see if we are selecting a card from the hand
		{ 
			for (Card* card : PlayerHand.Cards)
			{
				if (card->PointIn(GetMousePosition()))
				{
					PlayerHand.Select(card);
					break;
				}
			}
		}

		// check to see if we are interacting with the draw deck
		if (!handled)
		{
			Card* deckTop = DrawDeck.PeekTop();
			if (deckTop != nullptr)
			{
				if (deckTop->PointIn(GetMousePosition()))
				{
					if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
					{
						// show the top card
						deckTop->FaceUp = true;
					}
					else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && PlayerHand.SelectedCard == nullptr)
					{
						// take the card into a hand and start dragging it.
						PlayerHand.AddCard(DrawDeck.PopTop());

						// always look at drawn cards
						PlayerHand.SelectedCard->FaceUp = true;
					}
				}
			}
			
		}

		BeginDrawing();
		ClearBackground(DARKGREEN);

		DrawDeck.Draw();
		PlayerHand.Draw();

		EndDrawing();
	}

	CloseWindow();
}