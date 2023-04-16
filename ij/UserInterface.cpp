#include "UserInterface.h"
#include "Bot.h"
#include "DrawWorld.h"
#include "Input.h"
#include "LogicEntity.h"
#include "ObjectAnimation.h"
#include <imgui.h>

void ij::UpdateUserInterface(LogicEntity &player, const World &world, const Input &input, Debugging &debugging)
{
    ImGui::Begin("Character");
    {
        ImGui::Text("Health");
        ImGui::SameLine();
        ImGui::ProgressBar(AssertCast<float>(player.GetCurrentHealth()) / AssertCast<float>(player.GetMaximumHealth()));
    }
    ImGui::End();

    if (Object *const selectedEnemy = input.selectedEnemy)
    {
        ImGui::Begin("Enemy");
        {
            ImGui::Text("Health");
            ImGui::SameLine();
            ImGui::ProgressBar(AssertCast<float>(selectedEnemy->Logic.GetCurrentHealth()) /
                               AssertCast<float>(selectedEnemy->Logic.GetMaximumHealth()));
            ImGui::BeginDisabled();
            ImGui::Checkbox("Bumped", &selectedEnemy->Logic.HasBumpedIntoWall);
            ImGui::EndDisabled();
            ImGui::LabelText("Animation", "%s", GetObjectAnimationName(selectedEnemy->Visuals.Animation));
            ImGui::LabelText("Direction", "%f %f", AssertCast<double>(selectedEnemy->Logic.Direction.x),
                             AssertCast<double>(selectedEnemy->Logic.Direction.y));
            if (const Bot *const bot = dynamic_cast<const Bot *>(selectedEnemy->Logic.Behavior.get()))
            {
                ImGui::LabelText("State", "%s", Bot::GetStateName(bot->GetState()));
                ImGui::BeginDisabled();
                bool hasTarget = (bot->GetTarget() != nullptr);
                ImGui::Checkbox("Has target", &hasTarget);
                ImGui::EndDisabled();
            }
        }
        ImGui::End();
    }

    ImGui::Begin("Debug");
    ImGui::LabelText("Enemies in the world", "%zu", world.enemies.size());
    ImGui::LabelText("Enemies drawn", "%zu", debugging.enemiesDrawnLastFrame);
    ImGui::LabelText("Tiles in the world", "%zu", world.map.Tiles.size());
    ImGui::LabelText("Tiles drawn", "%zu", debugging.tilesDrawnLastFrame);
    ImGui::LabelText("Floating texts in the world", "%zu", world.FloatingTexts.size());
    ImGui::Checkbox("Player/wall collision", &player.HasCollisionWithWalls);
    ImGui::PlotHistogram("Frame times (ms)", debugging.FrameTimes.data(), AssertCast<int>(debugging.FrameTimes.size()),
                         AssertCast<int>(debugging.NextFrameTime), nullptr, 0.0f, 100.0f, ImVec2(300, 100));
    ImGui::Checkbox("Zoom out", &debugging.IsZoomedOut);
    ImGui::End();
}
