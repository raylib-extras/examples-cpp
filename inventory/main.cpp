/**********************************************************************************************
*
*   raylib-extras, Inventory example
*
*   LICENSE: MIT
*
*   Copyright (c) 2024 Jeffery Myers
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
#include "rlgl.h"

#include <vector>
#include <unordered_map>
#include <list>

struct Size2i
{
    int x = 0;
    int y = 0;
};

struct Item
{
public:
    Size2i Size;
    Color Tint = WHITE;
    Size2i BackPackLocation;
    Rectangle SourceRect = { 0 };
    int Id = 0;
};

class Inventory
{
protected:
    std::unordered_map<int, Item*>  Items;
    std::vector<int>    BackpackContents;
    Size2i              BackpackSize;

    size_t GetBackpackIndex(int x, int y)
    {
        return y * BackpackSize.x + x;
    }

public:
    Inventory(const Size2i& backpackSize)
        : BackpackSize(backpackSize)
    {
        BackpackContents.resize(BackpackSize.x * BackpackSize.y);

        for (auto& i : BackpackContents)
            i = -1;
    }

    const Size2i& GetBackpackSize() const { return BackpackSize; }

    const std::unordered_map<int, Item*>& GetItems() { return Items; }

    Item* FindItem(int id)
    {
        auto itr = Items.find(id);
        if (itr == Items.end())
            return nullptr;

        return itr->second;
    }

    bool ItemCanFit(Size2i position, Item* item)
    {
        for (int y = position.y; y < position.y + item->Size.y; y++)
        {
            if (y < 0 || y >= BackpackSize.y)
                return false;

            for (int x = position.x; x < position.x + item->Size.x; x++)
            {
                if (x < 0 || x >= BackpackSize.x)
                    return false;

                size_t index = GetBackpackIndex(x, y);
                if (BackpackContents[index] != -1)
                    return false;
            }
        }

        return true;
    }

    bool InsertItem(Size2i position, Item* item)
    {
        if (!ItemCanFit(position, item))
            return false;

        int nextId = 0;
        for (const auto& [id,item] : Items)
        {
            if (id > nextId)
                nextId = id;
        }
        nextId++;
        item->Id = nextId;

        for (int y = position.y; y < position.y + item->Size.y; y++)
        {
            for (int x = position.x; x < position.x + item->Size.x; x++)
            {
                size_t index = GetBackpackIndex(x, y);
                BackpackContents[index] = item->Id;
            }
        }
        item->BackPackLocation = position;
        Items.insert_or_assign(item->Id, item);

        return true;
    }

    Item* RemoveItem(int id)
    {
        auto itr = Items.find(id);
        if (itr == Items.end())
            return nullptr;

        Item* item = itr->second;

        Items.erase(itr);

        for (auto& slot : BackpackContents)
        {
            if (slot == id)
                slot = -1;
        }

        return item;
    }

    Size2i FindAvailableSlot(Item* item)
    {
        for (int y = 0; y < BackpackSize.y; y++)
        {
            for (int x = 0; x < BackpackSize.x; x++)
            {
                Size2i pos = { x,y };
                if (ItemCanFit(pos, item))
                {
                    return pos;
                }
            }
        }

        return Size2i{ -1,-1 };
    }
};

Inventory PlayerInventory(Size2i{ 8,4 });

Rectangle InventoryBounds = { 50, 50, 600,300 };

Texture ItemSheet = { 0 };

Item* DraggedItem = nullptr;

void DrawItem(Item* item, Rectangle rect, float gutter)
{
    if (item == nullptr)
        return;

    rect.x += gutter * 2;
    rect.y += gutter * 2;
    rect.width -= gutter * 4;
    rect.height -= gutter * 4;
    DrawRectangleRec(rect, ColorAlpha(item->Tint, 0.9f));
    float alpha = 1.0f;
    if (item == DraggedItem)
        alpha = 0.75f;

    DrawTexturePro(ItemSheet, item->SourceRect, rect, Vector2Zero(), 0, ColorAlpha(WHITE,alpha));
    DrawText(TextFormat("%d", item->Id), (int)rect.x + 2, (int)rect.y + 2, 20, BLACK);
}

Rectangle ComputeBackpackRect(Size2i position, const Rectangle& bounds, float gutter)
{
    float itemWidth = ((bounds.width - gutter * 2) / PlayerInventory.GetBackpackSize().x);
    float itemHeight = ((bounds.height - gutter * 2) / PlayerInventory.GetBackpackSize().y);

    Rectangle itemRect = Rectangle{ bounds.x + gutter + position.x * itemWidth, bounds.y + gutter + position.y * itemHeight, itemWidth - gutter, itemHeight - gutter };
    return itemRect;
}

Rectangle ComputeItemRect(Size2i position, Item* item, const Rectangle& bounds, float gutter)
{
    float itemWidth = ((bounds.width - gutter * 2) / PlayerInventory.GetBackpackSize().x);
    float itemHeight = ((bounds.height - gutter * 2) / PlayerInventory.GetBackpackSize().y);

    Rectangle itemRect;
    itemRect.x = bounds.x + gutter + position.x * itemWidth;
    itemRect.y = bounds.y + gutter + position.y * itemHeight;
    itemRect.width = (item->Size.x) * itemWidth - gutter;
    itemRect.height = (item->Size.y) * itemHeight - gutter;

    return itemRect;
}

Rectangle ComputeItemRect(Item* item, const Rectangle& bounds, float gutter)
{
    return ComputeItemRect(item->BackPackLocation, item, bounds, gutter);
}

void DrawInventory(const Rectangle& bounds)
{
    DrawText("Inventory: right click item to remove it, Drag to move or drop", (int)bounds.x, (int)bounds.y - 20, 20, BLACK);
    DrawRectangleRec(bounds, BROWN);
    DrawRectangleLinesEx(bounds, 2, DARKBROWN);

    float gutter = 2;
 
    for (int y = 0; y < PlayerInventory.GetBackpackSize().y; y++)
    {
        for (int x = 0; x < PlayerInventory.GetBackpackSize().x; x++)
        {
            Rectangle itemRect = ComputeBackpackRect(Size2i{ x,y }, bounds, gutter);

            DrawRectangleRec(itemRect, LIGHTGRAY);
            DrawRectangleLinesEx(itemRect, 2, DARKGRAY);
        }
    }

    int toRemove = -1;

    for (auto&[id, item] : PlayerInventory.GetItems())
    {
        Rectangle itemRect = ComputeItemRect(item, bounds, gutter);
        DrawItem(item, itemRect, gutter);

        if (CheckCollisionPointRec(GetMousePosition(), itemRect))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                toRemove = id;
            }
            else if (!DraggedItem && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                DraggedItem = item;
            }
        }
    }

    if (toRemove != -1)
    {
        auto* item = PlayerInventory.RemoveItem(toRemove);
        delete(item);
    }
}

std::vector<Item> BaseItems;

bool AddItem(Item& baseItem)
{
    Item* item = new Item();
    item->Size.x = baseItem.Size.x;
    item->Size.y = baseItem.Size.y;
    item->Tint = baseItem.Tint;
    item->SourceRect = baseItem.SourceRect;

    auto pos = PlayerInventory.FindAvailableSlot(item);
    if (!PlayerInventory.InsertItem(pos, item))
    {
        delete(item);
        return false;
    }
    return true;
}

void DrawItemList()
{
    for (auto& baseItem : BaseItems)
    {
        Rectangle rec = { (float)baseItem.BackPackLocation.x, (float)baseItem.BackPackLocation.y, baseItem.Size.x * 45.0f, baseItem.Size.y * 45.0f };
        DrawRectangleRec(rec, baseItem.Tint);
        DrawTexturePro(ItemSheet, baseItem.SourceRect, rec, Vector2Zero(), 0, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), rec))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                AddItem(baseItem);
            }
            else if (DraggedItem == nullptr && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                DraggedItem = new Item();
                DraggedItem->Size = baseItem.Size;
                DraggedItem->SourceRect = baseItem.SourceRect;
                DraggedItem->Id = -1;
                DraggedItem->Tint = baseItem.Tint;
            }
        }
    }
    DrawText("Right Click to add to inventory, or drag", 600, 380, 20, BLACK);
}

Size2i GetMouseSlot()
{
    Size2i slot = { -1,-1 };
    Vector2 mouse = GetMousePosition();
    if (mouse.x < InventoryBounds.x || mouse.x > InventoryBounds.x + InventoryBounds.width)
        return slot;

    if (mouse.y < InventoryBounds.y || mouse.y > InventoryBounds.y + InventoryBounds.height)
        return slot;

    float itemWidth = ((InventoryBounds.width) / PlayerInventory.GetBackpackSize().x);
    float itemHeight = ((InventoryBounds.height) / PlayerInventory.GetBackpackSize().y);

    mouse.x -= InventoryBounds.x;
    mouse.y -= InventoryBounds.y;

    slot.x = (int)(mouse.x / itemWidth);
    slot.y = (int)(mouse.y / itemHeight);

    return slot;
}

void GameInit()
{
    float grid = 256.0f / 3.0f;

    ItemSheet = LoadTexture("resources/Items.png");
    BaseItems.push_back(Item{ Size2i{1,1}, DARKGREEN, Size2i{600,400}, Rectangle{0, 0 ,grid,grid} });
    BaseItems.push_back(Item{ Size2i{1,2}, PURPLE   , Size2i{650,400}, Rectangle{0,grid,grid,grid *2} });
    BaseItems.push_back(Item{ Size2i{2,1}, MAROON   , Size2i{700,400}, Rectangle{grid,0,grid*2,grid} });
    BaseItems.push_back(Item{ Size2i{2,2}, ORANGE   , Size2i{700,450}, Rectangle{grid,grid,grid * 2,grid * 2} });

    AddItem(BaseItems[1]);
}

bool GameUpdate()
{
    if (DraggedItem != nullptr)
    {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Size2i slot = GetMouseSlot();
            if (slot.x >= 0)
            {
                if (PlayerInventory.ItemCanFit(slot, DraggedItem))
                {
                    PlayerInventory.RemoveItem(DraggedItem->Id);
                    PlayerInventory.InsertItem(slot, DraggedItem);
                }
            }
            else
            {
                if (DraggedItem->Id >= 0)
                    PlayerInventory.RemoveItem(DraggedItem->Id);
                delete(DraggedItem);
            }

            DraggedItem = nullptr;
        }
            
    }
    return true;
}

void DrawDraggedItem()
{
    if (DraggedItem != nullptr)
    {
        Rectangle mouseRect = { GetMousePosition().x - DraggedItem->SourceRect.width * 0.25f, GetMousePosition().y - DraggedItem->SourceRect.height * 0.15f, DraggedItem->SourceRect.width, DraggedItem->SourceRect.height };

        Color tint = RED;
        Size2i slot = GetMouseSlot();
        if (slot.x >= 0)
        {
            if (PlayerInventory.ItemCanFit(slot, DraggedItem))
            {
                tint = WHITE;

                Rectangle itemRect = ComputeItemRect(slot, DraggedItem, InventoryBounds, 0);
                DrawRectangleRec(itemRect, ColorAlpha(GREEN, 0.5f));
            }
            else
            {
                tint = ORANGE;
            }
        }

        DrawTexturePro(ItemSheet, DraggedItem->SourceRect, mouseRect, Vector2Zero(), 0, ColorAlpha(tint, 0.8f));
    }
}

void Draw2D()
{
    DrawInventory(InventoryBounds);
    DrawItemList();
    DrawDraggedItem();
}

void GameDraw()
{
    BeginDrawing();
    ClearBackground(DARKGRAY);
    Draw2D();
    DrawFPS(0, GetScreenHeight() - 20);
    EndDrawing();
}

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 800, "InventoryExample");
    SetTargetFPS(144);

    GameInit();

    while (!WindowShouldClose())
    {
        if (!GameUpdate())
            break;

        GameDraw();
    }

    CloseWindow();
    return 0;
}
