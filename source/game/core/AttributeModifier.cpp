#include "game/core/AttributeModifier.hpp"

#include "game/world/generation/Player.hpp"

class LivingEntity;

#include <wchar.h>

#include "game/item/EnchantmentHelper.hpp"
#include "game/core/L10N.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

void AttributeModifier::_init(eMODIFIER_ID id, std::wstring name, double amount, int operation)
{
    this->operation = operation;
    this->amount = amount;
    this->name = name;
    this->id = id;
    this->serialize = true;
}

AttributeModifier::AttributeModifier(double amount, int operation)
{
    _init(eMODIFIER_ID_None, std::wstring(), amount, operation);
}

AttributeModifier::AttributeModifier(eMODIFIER_ID id, double amount, int operation)
{
    _init(id, std::wstring(), amount, operation);
}

AttributeModifier::AttributeModifier(eMODIFIER_ID id, double amount, int operation, bool serialize)
{
    _init(id, std::wstring(), amount, operation);

    this->serialize = serialize;
}

AttributeModifier::AttributeModifier(const AttributeModifier& other)
    : amount(other.amount),
      operation(other.operation),
      name(other.name),
      id(other.id),
      serialize(other.serialize)
{
}

eMODIFIER_ID AttributeModifier::getId()
{
    return id;
}

int AttributeModifier::getOperation()
{
    return operation;
}

double AttributeModifier::getAmount()
{
    return amount;
}

bool AttributeModifier::isSerializable()
{
    return serialize;
}

AttributeModifier* AttributeModifier::setSerialize(bool serialize)
{
    this->serialize = serialize;
    return this;
}

bool AttributeModifier::equals(AttributeModifier* other)
{
    if (this != other && (other == NULL || id != other->id))
    {
        return false;
    }

    return true;
}

HtmlString AttributeModifier::getHoverText(eATTRIBUTE_ID attributeId, not_null_ptr<ItemInstance> item, boost::shared_ptr<Player> player)
{
    double amount = getAmount();
    bool baseValue = false;

    eMODIFIER_ID modifierId = getId();

    if (modifierId == eMODIFIER_ID_BaseAttackDamage)
    {
        amount += EnchantmentHelper::getDamageBonus(item, boost::shared_ptr<LivingEntity>());
    }
    else if (modifierId == eMODIFIER_ID_BaseAttackSpeed)
    {
        if (player.get() != NULL)
        {
            amount += player->getAttribute(SharedMonsterAttributes::ATTACK_SPEED)->getBaseValue();
        }

        baseValue = true;
    }

    int operation = getOperation();

    double value;
    bool percentage;

    if (operation == 1 || operation == 2)
    {
        value = amount * 100.0;
        percentage = true;
    }
    else
    {
        value = amount;
        percentage = false;
    }

    eMinecraftColour colour = eMinecraftColour_NOTSET;
    std::wstring sign;

    if (baseValue)
    {
        sign = L"";
    }
    else
    {
        if (amount > 0.0)
        {
            colour = eMinecraftColour_HTMLColor_9;
        }
        else if (amount < 0.0)
        {
            value = -value;
            colour = eMinecraftColour_HTMLColor_c;
        }

        sign = amount < 0.0 ? L"-" : L"+";
    }

    wchar_t text[256];

    if (percentage)
    {
        swprintf(text, 256, L" %ls%d%ls %ls", sign.c_str(), (int)value, L"%", L10N::GetString(Attribute::getName(attributeId)));
    }
    else
    {
        swprintf(text, 256, L" %ls%.1f %ls", sign.c_str(), value, L10N::GetString(Attribute::getName(attributeId)));
    }

    return HtmlString(text, colour, false, false, 0);
}
