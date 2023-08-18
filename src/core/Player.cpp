#include "Player.h"

#include "sys/Renderer.h"
#include "Engine.h"

Player::Player(Engine& engine)
    : engine{ engine },
      thing{ engine, ThingType::Player, THING_FLAG_SOLID, phys::Box{ .center = {}, .width = 10.0f, .height = 10.0f, .velocity = {} } },
      tankBody{ engine, "models/tank/tank_body.txt" },
      tankTurret{ engine, "models/tank/tank_turret.txt" }
{
}

Player::~Player() = default;
