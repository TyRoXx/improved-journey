#include "DrawWorld.h"
#include "Camera.h"
#include "Input.h"

namespace ij
{
    namespace
    {
        Vector2i findTileByCoordinates(const Vector2f &position)
        {
            return Vector2i(RoundDown<Int32>(position.x / TileSize), RoundDown<Int32>(position.y / TileSize));
        }

        float bottomOfSprite(const Sprite &sprite)
        {
            return (AssertCast<float>(sprite.Position.y) + AssertCast<float>(sprite.TextureSize.y));
        }

        void drawHealthBar(Canvas &canvas, const Object &object)
        {
            if (object.Logic.GetCurrentHealth() == object.Logic.GetMaximumHealth())
            {
                return;
            }
            constexpr UInt32 width = 24;
            constexpr UInt32 height = 4;
            const Int32 x = RoundDown<Int32>(object.Logic.Position.x) - AssertCast<Int32>(width / 2);
            const Int32 y = RoundDown<Int32>(object.Logic.Position.y) -
                            AssertCast<Int32>(object.Visuals.GetTextureRect(object.Logic.Direction).Size.y);
            const UInt32 greenPortion =
                RoundDown<UInt32>(AssertCast<float>(object.Logic.GetCurrentHealth()) /
                                  AssertCast<float>(object.Logic.GetMaximumHealth()) * AssertCast<float>(width));
            const Color green(0, 255, 0, 255);
            canvas.DrawRectangle(Vector2i(x, y), Vector2u(greenPortion, height), green, green, 1);
            const Color red(255, 0, 0, 255);
            canvas.DrawRectangle(Vector2i(x + AssertCast<Int32>(greenPortion), y),
                                 Vector2u((width - greenPortion), height), red, red, 1);
        }
    } // namespace
} // namespace ij

void ij::DrawWorld(Canvas &canvas, const Camera &camera, const Input &input, Debugging &debugging, World &world,
                   Object &player, const TextureId grassTexture, const TimeSpan timeSinceLastDraw)
{
    const Vector2u windowSize = canvas.GetSize();
    const Vector2i topLeft = findTileByCoordinates(camera.getWorldFromScreenCoordinates(windowSize, Vector2i(0, 0)));
    const Vector2i bottomRight =
        findTileByCoordinates(camera.getWorldFromScreenCoordinates(windowSize, AssertCastVector<Int32>(windowSize)));

    debugging.tilesDrawnLastFrame = 0;
    for (size_t y = AssertCast<size_t>((std::max)(0, topLeft.y)),
                yStop = AssertCast<size_t>(
                    (std::min<ptrdiff_t>)(bottomRight.y, AssertCast<ptrdiff_t>(world.map.GetHeight() - 1)));
         y <= yStop; ++y)
    {
        for (size_t x = AssertCast<size_t>((std::max)(0, topLeft.x)),
                    xStop = AssertCast<size_t>(
                        (std::min<ptrdiff_t>)(bottomRight.x, AssertCast<ptrdiff_t>(world.map.Width - 1)));
             x <= xStop; ++x)
        {
            const int tile = world.map.GetTileAt(x, y);
            if (tile == NoTile)
            {
                continue;
            }
            canvas.DrawSprite(Sprite(grassTexture,
                                     Vector2i(AssertCast<Int32>(x) * TileSize, AssertCast<Int32>(y) * TileSize),
                                     Color(255, 255, 255, 255), Vector2u(AssertCast<UInt32>(tile) * TileSize, 160),
                                     Vector2u(TileSize, TileSize)));
            ++debugging.tilesDrawnLastFrame;
        }
    }

    std::vector<const Object *> visibleEnemies;
    std::vector<Sprite> spritesToDrawInZOrder;
    updateVisuals(player.Logic, player.Visuals, timeSinceLastDraw);
    spritesToDrawInZOrder.emplace_back(CreateSpriteForVisualEntity(player.Logic, player.Visuals));

    debugging.enemiesDrawnLastFrame = 0;
    for (Object &enemy : world.enemies)
    {
        updateVisuals(enemy.Logic, enemy.Visuals, timeSinceLastDraw);
        Sprite sprite = CreateSpriteForVisualEntity(enemy.Logic, enemy.Visuals);
        if (camera.canSee(windowSize, enemy.Logic.Position, enemy.Visuals))
        {
            visibleEnemies.push_back(&enemy);
            spritesToDrawInZOrder.emplace_back(sprite);
            ++debugging.enemiesDrawnLastFrame;
        }
    }

    for (size_t i = 0; i < world.FloatingTexts.size();)
    {
        world.FloatingTexts[i].Update(timeSinceLastDraw);
        if (world.FloatingTexts[i].HasExpired())
        {
            if ((i + 1) < world.FloatingTexts.size())
            {
                world.FloatingTexts[i] = std::move(world.FloatingTexts.back());
            }
            world.FloatingTexts.pop_back();
        }
        else
        {
            ++i;
        }
    }

    std::ranges::sort(spritesToDrawInZOrder, [](const Sprite &left, const Sprite &right) -> bool {
        return (bottomOfSprite(left) < bottomOfSprite(right));
    });
    for (const Sprite &sprite : spritesToDrawInZOrder)
    {
        canvas.DrawSprite(sprite);
    }

    for (FloatingText &floatingText : world.FloatingTexts)
    {
        floatingText.VisualItem.Draw();
    }

    drawHealthBar(canvas, player);
    for (const Object *const enemy : visibleEnemies)
    {
        drawHealthBar(canvas, *enemy);
    }

    canvas.DrawDot(RoundDown<Int32>(player.Logic.Position), Color(0, 255, 0, 255));

    for (const Object *const enemy : visibleEnemies)
    {
        canvas.DrawDot(RoundDown<Int32>(enemy->Logic.Position), Color(255, 0, 0, 255));

        if (enemy == input.selectedEnemy)
        {
            canvas.DrawRectangle(RoundDown<Int32>(enemy->Visuals.GetTopLeftPosition(enemy->Logic.Position)),
                                 enemy->Visuals.SpriteSize, Color(255, 255, 255, 255), Color(0, 0, 0, 0), 1);
        }
    }
}
