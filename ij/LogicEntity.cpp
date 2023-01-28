#include "LogicEntity.h"

bool ij::isDead(const LogicEntity &entity)
{
    return (entity.GetCurrentHealth() == 0);
}
