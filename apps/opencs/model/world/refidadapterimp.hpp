#ifndef CSM_WOLRD_REFIDADAPTERIMP_H
#define CSM_WOLRD_REFIDADAPTERIMP_H

#include <map>

#include <QVariant>

#include <components/esm/loadalch.hpp>
#include <components/esm/loadench.hpp>
#include <components/esm/loadappa.hpp>
#include <components/esm/loadnpc.hpp>
#include <components/esm/loadcrea.hpp>
#include <components/esm/loadcont.hpp>

#include "record.hpp"
#include "refiddata.hpp"
#include "universalid.hpp"
#include "refidadapter.hpp"
#include "nestedadapters.hpp"

namespace CSMWorld
{
    struct NestedTableWrapperBase;

    struct BaseColumns
    {
        const RefIdColumn *mId;
        const RefIdColumn *mModified;
        const RefIdColumn *mType;
    };

    /// \brief Base adapter for all refereceable record types
    /// Adapters that can handle nested tables, needs to return valid qvariant for parent columns
    template<typename RecordT>
    class BaseRefIdAdapter : public RefIdAdapter
    {
            UniversalId::Type mType;
            BaseColumns mBase;

        public:

            BaseRefIdAdapter (UniversalId::Type type, const BaseColumns& base);

            virtual std::string getId (const RecordBase& record) const;

            virtual void setId (RecordBase& record, const std::string& id);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.

            UniversalId::Type getType() const;
    };

    template<typename RecordT>
    BaseRefIdAdapter<RecordT>::BaseRefIdAdapter (UniversalId::Type type, const BaseColumns& base)
    : mType (type), mBase (base)
    {}

    template<typename RecordT>
    void BaseRefIdAdapter<RecordT>::setId (RecordBase& record, const std::string& id)
    {
        (dynamic_cast<Record<RecordT>&> (record).get().mId) = id;
    }

    template<typename RecordT>
    std::string BaseRefIdAdapter<RecordT>::getId (const RecordBase& record) const
    {
        return dynamic_cast<const Record<RecordT>&> (record).get().mId;
    }

    template<typename RecordT>
    QVariant BaseRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, mType)));

        if (column==mBase.mId)
            return QString::fromUtf8 (record.get().mId.c_str());

        if (column==mBase.mModified)
        {
            if (record.mState==Record<RecordT>::State_Erased)
                return static_cast<int> (Record<RecordT>::State_Deleted);

            return static_cast<int> (record.mState);
        }

        if (column==mBase.mType)
            return static_cast<int> (mType);

        return QVariant();
    }

    template<typename RecordT>
    void BaseRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data, int index,
        const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, mType)));

        if (column==mBase.mModified)
            record.mState = static_cast<RecordBase::State> (value.toInt());
    }

    template<typename RecordT>
    UniversalId::Type BaseRefIdAdapter<RecordT>::getType() const
    {
        return mType;
    }


    struct ModelColumns : public BaseColumns
    {
        const RefIdColumn *mModel;

        ModelColumns (const BaseColumns& base) : BaseColumns (base) {}
    };

    /// \brief Adapter for IDs with models (all but levelled lists)
    template<typename RecordT>
    class ModelRefIdAdapter : public BaseRefIdAdapter<RecordT>
    {
            ModelColumns mModel;

        public:

            ModelRefIdAdapter (UniversalId::Type type, const ModelColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    template<typename RecordT>
    ModelRefIdAdapter<RecordT>::ModelRefIdAdapter (UniversalId::Type type, const ModelColumns& columns)
    : BaseRefIdAdapter<RecordT> (type, columns), mModel (columns)
    {}

    template<typename RecordT>
    QVariant ModelRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        if (column==mModel.mModel)
            return QString::fromUtf8 (record.get().mModel.c_str());

        return BaseRefIdAdapter<RecordT>::getData (column, data, index);
    }

    template<typename RecordT>
    void ModelRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data, int index,
        const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        RecordT record2 = record.get();
        if (column==mModel.mModel)
            record2.mModel = value.toString().toUtf8().constData();
        else
        {
            BaseRefIdAdapter<RecordT>::setData (column, data, index, value);
            return;
        }

        record.setModified(record2);
    }

    struct NameColumns : public ModelColumns
    {
        const RefIdColumn *mName;
        const RefIdColumn *mScript;

        NameColumns (const ModelColumns& base) : ModelColumns (base) {}
    };

    /// \brief Adapter for IDs with names (all but levelled lists and statics)
    template<typename RecordT>
    class NameRefIdAdapter : public ModelRefIdAdapter<RecordT>
    {
            NameColumns mName;

        public:

            NameRefIdAdapter (UniversalId::Type type, const NameColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    template<typename RecordT>
    NameRefIdAdapter<RecordT>::NameRefIdAdapter (UniversalId::Type type, const NameColumns& columns)
    : ModelRefIdAdapter<RecordT> (type, columns), mName (columns)
    {}

    template<typename RecordT>
    QVariant NameRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        if (column==mName.mName)
            return QString::fromUtf8 (record.get().mName.c_str());

        if (column==mName.mScript)
            return QString::fromUtf8 (record.get().mScript.c_str());

        return ModelRefIdAdapter<RecordT>::getData (column, data, index);
    }

    template<typename RecordT>
    void NameRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data, int index,
        const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        RecordT record2 = record.get();
        if (column==mName.mName)
            record2.mName = value.toString().toUtf8().constData();
        else if (column==mName.mScript)
            record2.mScript = value.toString().toUtf8().constData();
        else
        {
            ModelRefIdAdapter<RecordT>::setData (column, data, index, value);
            return;
        }

        record.setModified(record2);
    }

    struct InventoryColumns : public NameColumns
    {
        const RefIdColumn *mIcon;
        const RefIdColumn *mWeight;
        const RefIdColumn *mValue;

        InventoryColumns (const NameColumns& base) : NameColumns (base) {}
    };

    /// \brief Adapter for IDs that can go into an inventory
    template<typename RecordT>
    class InventoryRefIdAdapter : public NameRefIdAdapter<RecordT>
    {
            InventoryColumns mInventory;

        public:

            InventoryRefIdAdapter (UniversalId::Type type, const InventoryColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    template<typename RecordT>
    InventoryRefIdAdapter<RecordT>::InventoryRefIdAdapter (UniversalId::Type type,
        const InventoryColumns& columns)
    : NameRefIdAdapter<RecordT> (type, columns), mInventory (columns)
    {}

    template<typename RecordT>
    QVariant InventoryRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        if (column==mInventory.mIcon)
            return QString::fromUtf8 (record.get().mIcon.c_str());

        if (column==mInventory.mWeight)
            return record.get().mData.mWeight;

        if (column==mInventory.mValue)
            return record.get().mData.mValue;

        return NameRefIdAdapter<RecordT>::getData (column, data, index);
    }

    template<typename RecordT>
    void InventoryRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data, int index,
        const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        RecordT record2 = record.get();
        if (column==mInventory.mIcon)
            record2.mIcon = value.toString().toUtf8().constData();
        else if (column==mInventory.mWeight)
            record2.mData.mWeight = value.toFloat();
        else if (column==mInventory.mValue)
            record2.mData.mValue = value.toInt();
        else
        {
            NameRefIdAdapter<RecordT>::setData (column, data, index, value);
            return;
        }

        record.setModified(record2);
    }

    struct PotionColumns : public InventoryColumns
    {
        const RefIdColumn *mEffects;

        PotionColumns (const InventoryColumns& columns);
    };

    class PotionRefIdAdapter : public InventoryRefIdAdapter<ESM::Potion>
    {
            PotionColumns mColumns;
            const RefIdColumn *mAutoCalc;

        public:

            PotionRefIdAdapter (const PotionColumns& columns, const RefIdColumn *autoCalc);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    struct EnchantableColumns : public InventoryColumns
    {
        const RefIdColumn *mEnchantment;
        const RefIdColumn *mEnchantmentPoints;

        EnchantableColumns (const InventoryColumns& base) : InventoryColumns (base) {}
    };

    /// \brief Adapter for enchantable IDs
    template<typename RecordT>
    class EnchantableRefIdAdapter : public InventoryRefIdAdapter<RecordT>
    {
            EnchantableColumns mEnchantable;

        public:

            EnchantableRefIdAdapter (UniversalId::Type type, const EnchantableColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    template<typename RecordT>
    EnchantableRefIdAdapter<RecordT>::EnchantableRefIdAdapter (UniversalId::Type type,
        const EnchantableColumns& columns)
    : InventoryRefIdAdapter<RecordT> (type, columns), mEnchantable (columns)
    {}

    template<typename RecordT>
    QVariant EnchantableRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        if (column==mEnchantable.mEnchantment)
            return QString::fromUtf8 (record.get().mEnchant.c_str());

        if (column==mEnchantable.mEnchantmentPoints)
            return static_cast<int> (record.get().mData.mEnchant);

        return InventoryRefIdAdapter<RecordT>::getData (column, data, index);
    }

    template<typename RecordT>
    void EnchantableRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data,
        int index, const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        RecordT record2 = record.get();
        if (column==mEnchantable.mEnchantment)
            record2.mEnchant = value.toString().toUtf8().constData();
        else if (column==mEnchantable.mEnchantmentPoints)
            record2.mData.mEnchant = value.toInt();
        else
        {
            InventoryRefIdAdapter<RecordT>::setData (column, data, index, value);
            return;
        }

        record.setModified(record2);
    }

    struct ToolColumns : public InventoryColumns
    {
        const RefIdColumn *mQuality;
        const RefIdColumn *mUses;

        ToolColumns (const InventoryColumns& base) : InventoryColumns (base) {}
    };

    /// \brief Adapter for tools with limited uses IDs (lockpick, repair, probes)
    template<typename RecordT>
    class ToolRefIdAdapter : public InventoryRefIdAdapter<RecordT>
    {
            ToolColumns mTools;

        public:

            ToolRefIdAdapter (UniversalId::Type type, const ToolColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    template<typename RecordT>
    ToolRefIdAdapter<RecordT>::ToolRefIdAdapter (UniversalId::Type type, const ToolColumns& columns)
    : InventoryRefIdAdapter<RecordT> (type, columns), mTools (columns)
    {}

    template<typename RecordT>
    QVariant ToolRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        if (column==mTools.mQuality)
            return record.get().mData.mQuality;

        if (column==mTools.mUses)
            return record.get().mData.mUses;

        return InventoryRefIdAdapter<RecordT>::getData (column, data, index);
    }

    template<typename RecordT>
    void ToolRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data,
        int index, const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        RecordT record2 = record.get();
        if (column==mTools.mQuality)
            record2.mData.mQuality = value.toFloat();
        else if (column==mTools.mUses)
            record2.mData.mUses = value.toInt();
        else
        {
            InventoryRefIdAdapter<RecordT>::setData (column, data, index, value);
            return;
        }

        record.setModified(record2);
    }

    struct ActorColumns : public NameColumns
    {
        const RefIdColumn *mHasAi;
        const RefIdColumn *mHello;
        const RefIdColumn *mFlee;
        const RefIdColumn *mFight;
        const RefIdColumn *mAlarm;
        const RefIdColumn *mInventory;
        const RefIdColumn *mSpells;
        std::map<const RefIdColumn *, unsigned int> mServices;

        ActorColumns (const NameColumns& base) : NameColumns (base) {}
    };

    /// \brief Adapter for actor IDs (handles common AI functionality)
    template<typename RecordT>
    class ActorRefIdAdapter : public NameRefIdAdapter<RecordT>, public NestedRefIdAdapter
    {
            ActorColumns mActors;

        public:

            ActorRefIdAdapter (UniversalId::Type type, const ActorColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    template<typename RecordT>
    ActorRefIdAdapter<RecordT>::ActorRefIdAdapter (UniversalId::Type type,
        const ActorColumns& columns)
    : NameRefIdAdapter<RecordT> (type, columns), mActors (columns)
    {
        std::vector<std::pair <const RefIdColumn*, HelperBase*> > assoCol;

        assoCol.push_back(std::make_pair(mActors.mInventory, new InventoryHelper<RecordT>(type)));
        assoCol.push_back(std::make_pair(mActors.mSpells, new SpellsHelper<RecordT>(type)));

        setAssocColumns(assoCol);
    }

    template<typename RecordT>
    QVariant ActorRefIdAdapter<RecordT>::getData (const RefIdColumn *column, const RefIdData& data,
        int index) const
    {
        const Record<RecordT>& record = static_cast<const Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        if (column==mActors.mHasAi)
            return record.get().mHasAI!=0;

        if (column==mActors.mHello)
            return record.get().mAiData.mHello;

        if (column==mActors.mFlee)
            return record.get().mAiData.mFlee;

        if (column==mActors.mFight)
            return record.get().mAiData.mFight;

        if (column==mActors.mAlarm)
            return record.get().mAiData.mAlarm;

        if (column==mActors.mInventory)
            return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

        if (column==mActors.mSpells)
            return true; // to show nested tables in dialogue subview, see IdTree::hasChildren()

        std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
            mActors.mServices.find (column);

        if (iter!=mActors.mServices.end())
            return (record.get().mAiData.mServices & iter->second)!=0;

        return NameRefIdAdapter<RecordT>::getData (column, data, index);
    }

    template<typename RecordT>
    void ActorRefIdAdapter<RecordT>::setData (const RefIdColumn *column, RefIdData& data, int index,
        const QVariant& value) const
    {
        Record<RecordT>& record = static_cast<Record<RecordT>&> (
            data.getRecord (RefIdData::LocalIndex (index, BaseRefIdAdapter<RecordT>::getType())));

        RecordT record2 = record.get();
        if (column==mActors.mHasAi)
            record2.mHasAI = value.toInt();
        else if (column==mActors.mHello)
            record2.mAiData.mHello = value.toInt();
        else if (column==mActors.mFlee)
            record2.mAiData.mFlee = value.toInt();
        else if (column==mActors.mFight)
            record2.mAiData.mFight = value.toInt();
        else if (column==mActors.mAlarm)
            record2.mAiData.mAlarm = value.toInt();
        else
        {
            typename std::map<const RefIdColumn *, unsigned int>::const_iterator iter =
                mActors.mServices.find (column);
            if (iter!=mActors.mServices.end())
            {
                if (value.toInt()!=0)
                    record2.mAiData.mServices |= iter->second;
                else
                    record2.mAiData.mServices &= ~iter->second;
            }
            else
            {
                NameRefIdAdapter<RecordT>::setData (column, data, index, value);
                return;
            }
        }

        record.setModified(record2);
    }

    class ApparatusRefIdAdapter : public InventoryRefIdAdapter<ESM::Apparatus>
    {
            const RefIdColumn *mType;
            const RefIdColumn *mQuality;

        public:

            ApparatusRefIdAdapter (const InventoryColumns& columns, const RefIdColumn *type,
                const RefIdColumn *quality);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class ArmorRefIdAdapter : public EnchantableRefIdAdapter<ESM::Armor>
    {
            const RefIdColumn *mType;
            const RefIdColumn *mHealth;
            const RefIdColumn *mArmor;

        public:

            ArmorRefIdAdapter (const EnchantableColumns& columns, const RefIdColumn *type,
                const RefIdColumn *health, const RefIdColumn *armor);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class BookRefIdAdapter : public EnchantableRefIdAdapter<ESM::Book>
    {
            const RefIdColumn *mScroll;
            const RefIdColumn *mSkill;

        public:

            BookRefIdAdapter (const EnchantableColumns& columns, const RefIdColumn *scroll,
                const RefIdColumn *skill);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class ClothingRefIdAdapter : public EnchantableRefIdAdapter<ESM::Clothing>
    {
            const RefIdColumn *mType;

        public:

            ClothingRefIdAdapter (const EnchantableColumns& columns, const RefIdColumn *type);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class ContainerRefIdAdapter : public NameRefIdAdapter<ESM::Container>, public NestedRefIdAdapter
    {
            const RefIdColumn *mWeight;
            const RefIdColumn *mOrganic;
            const RefIdColumn *mRespawn;
            const RefIdColumn *mContent;

        public:

            ContainerRefIdAdapter (const NameColumns& columns, const RefIdColumn *weight,
                                   const RefIdColumn *organic, const RefIdColumn *respawn, const RefIdColumn *content);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index) const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                                  const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    struct CreatureColumns : public ActorColumns
    {
        std::map<const RefIdColumn *, unsigned int> mFlags;
        const RefIdColumn *mType;
        const RefIdColumn *mSoul;
        const RefIdColumn *mScale;
        const RefIdColumn *mOriginal;
        const RefIdColumn *mCombat;
        const RefIdColumn *mMagic;
        const RefIdColumn *mStealth;

        CreatureColumns (const ActorColumns& actorColumns);
    };

    class CreatureRefIdAdapter : public ActorRefIdAdapter<ESM::Creature>
    {
            CreatureColumns mColumns;

        public:

            CreatureRefIdAdapter (const CreatureColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class DoorRefIdAdapter : public NameRefIdAdapter<ESM::Door>
    {
            const RefIdColumn *mOpenSound;
            const RefIdColumn *mCloseSound;

        public:

            DoorRefIdAdapter (const NameColumns& columns, const RefIdColumn *openSound,
                const RefIdColumn *closeSound);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    struct LightColumns : public InventoryColumns
    {
        const RefIdColumn *mTime;
        const RefIdColumn *mRadius;
        const RefIdColumn *mColor;
        const RefIdColumn *mSound;
        std::map<const RefIdColumn *, unsigned int> mFlags;

        LightColumns (const InventoryColumns& columns);
    };

    class LightRefIdAdapter : public InventoryRefIdAdapter<ESM::Light>
    {
            LightColumns mColumns;

        public:

            LightRefIdAdapter (const LightColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class MiscRefIdAdapter : public InventoryRefIdAdapter<ESM::Miscellaneous>
    {
            const RefIdColumn *mKey;

        public:

            MiscRefIdAdapter (const InventoryColumns& columns, const RefIdColumn *key);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    struct NpcColumns : public ActorColumns
    {
        std::map<const RefIdColumn *, unsigned int> mFlags;
        const RefIdColumn *mRace;
        const RefIdColumn *mClass;
        const RefIdColumn *mFaction;
        const RefIdColumn *mHair;
        const RefIdColumn *mHead;
        const RefIdColumn *mDestinations;

        NpcColumns (const ActorColumns& actorColumns);
    };

    class NpcRefIdAdapter : public ActorRefIdAdapter<ESM::NPC>
    {
            NpcColumns mColumns;

        public:

            NpcRefIdAdapter (const NpcColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    struct WeaponColumns : public EnchantableColumns
    {
        const RefIdColumn *mType;
        const RefIdColumn *mHealth;
        const RefIdColumn *mSpeed;
        const RefIdColumn *mReach;
        const RefIdColumn *mChop[2];
        const RefIdColumn *mSlash[2];
        const RefIdColumn *mThrust[2];
        std::map<const RefIdColumn *, unsigned int> mFlags;

        WeaponColumns (const EnchantableColumns& columns);
    };

    class WeaponRefIdAdapter : public EnchantableRefIdAdapter<ESM::Weapon>
    {
            WeaponColumns mColumns;

        public:

            WeaponRefIdAdapter (const WeaponColumns& columns);

            virtual QVariant getData (const RefIdColumn *column, const RefIdData& data, int index)
                const;

            virtual void setData (const RefIdColumn *column, RefIdData& data, int index,
                const QVariant& value) const;
            ///< If the data type does not match an exception is thrown.
    };

    class NestedRefIdAdapterBase;

    template<typename ESXRecordT>
    class EffectsListAdapter;

    template<typename ESXRecordT>
    class EffectsRefIdAdapter : public EffectsListAdapter<ESXRecordT>, public NestedRefIdAdapterBase
    {
        UniversalId::Type mType;

        // not implemented
        EffectsRefIdAdapter (const EffectsRefIdAdapter&);
        EffectsRefIdAdapter& operator= (const EffectsRefIdAdapter&);

    public:

        EffectsRefIdAdapter(UniversalId::Type type) :mType(type) {}

        virtual ~EffectsRefIdAdapter() {}

        virtual void addNestedRow (const RefIdColumn *column,
                RefIdData& data, int index, int position) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            EffectsListAdapter<ESXRecordT>::addNestedRow(record, position);
        }

        virtual void removeNestedRow (const RefIdColumn *column,
                RefIdData& data, int index, int rowToRemove) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            EffectsListAdapter<ESXRecordT>::removeNestedRow(record, rowToRemove);
        }

        virtual void setNestedTable (const RefIdColumn* column,
                RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            EffectsListAdapter<ESXRecordT>::setNestedTable(record, nestedTable);
        }

        virtual NestedTableWrapperBase* nestedTable (const RefIdColumn* column,
                const RefIdData& data, int index) const
        {
            const Record<ESXRecordT>& record =
                static_cast<const Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            return EffectsListAdapter<ESXRecordT>::nestedTable(record);
        }

        virtual QVariant getNestedData (const RefIdColumn *column,
                const RefIdData& data, int index, int subRowIndex, int subColIndex) const
        {
            const Record<ESXRecordT>& record =
                static_cast<const Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            return EffectsListAdapter<ESXRecordT>::getNestedData(record, subRowIndex, subColIndex);
        }

        virtual void setNestedData (const RefIdColumn *column,
                RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (row, mType)));
            EffectsListAdapter<ESXRecordT>::setNestedData(record, value, subRowIndex, subColIndex);
        }

        virtual int getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
        {
            const Record<ESXRecordT> record; // not used, just a dummy
            return EffectsListAdapter<ESXRecordT>::getNestedColumnsCount(record);
        }

        virtual int getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
        {
            const Record<ESXRecordT>& record =
                static_cast<const Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            return EffectsListAdapter<ESXRecordT>::getNestedRowsCount(record);
        }
    };

    template <typename ESXRecordT>
    class NestedInventoryRefIdAdapter : public NestedRefIdAdapterBase
    {
        UniversalId::Type mType;

        // not implemented
        NestedInventoryRefIdAdapter (const NestedInventoryRefIdAdapter&);
        NestedInventoryRefIdAdapter& operator= (const NestedInventoryRefIdAdapter&);

    public:

        NestedInventoryRefIdAdapter(UniversalId::Type type) :mType(type) {}

        virtual ~NestedInventoryRefIdAdapter() {}

        virtual void addNestedRow (const RefIdColumn *column,
                RefIdData& data, int index, int position) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            ESXRecordT container = record.get();

            std::vector<ESM::ContItem>& list = container.mInventory.mList;

            ESM::ContItem newRow = {0, {""}};

            if (position >= (int)list.size())
                list.push_back(newRow);
            else
                list.insert(list.begin()+position, newRow);

            record.setModified (container);
        }

        virtual void removeNestedRow (const RefIdColumn *column,
                RefIdData& data, int index, int rowToRemove) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            ESXRecordT container = record.get();

            std::vector<ESM::ContItem>& list = container.mInventory.mList;

            if (rowToRemove < 0 || rowToRemove >= static_cast<int> (list.size()))
                throw std::runtime_error ("index out of range");

            list.erase (list.begin () + rowToRemove);

            record.setModified (container);
        }

        virtual void setNestedTable (const RefIdColumn* column,
                RefIdData& data, int index, const NestedTableWrapperBase& nestedTable) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));
            ESXRecordT container = record.get();

            container.mInventory.mList =
                static_cast<const NestedTableWrapper<std::vector<ESM::ContItem> >&>(nestedTable).mNestedTable;

            record.setModified (container);
        }

        virtual NestedTableWrapperBase* nestedTable (const RefIdColumn* column,
                const RefIdData& data, int index) const
        {
            const Record<ESXRecordT>& record =
                static_cast<const Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

            // deleted by dtor of NestedTableStoring
            return new NestedTableWrapper<std::vector<ESM::ContItem> >(record.get().mInventory.mList);
        }

        virtual QVariant getNestedData (const RefIdColumn *column,
                const RefIdData& data, int index, int subRowIndex, int subColIndex) const
        {
            const Record<ESXRecordT>& record =
                static_cast<const Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

            const std::vector<ESM::ContItem>& list = record.get().mInventory.mList;

            if (subRowIndex < 0 || subRowIndex >= static_cast<int> (list.size()))
                throw std::runtime_error ("index out of range");

            const ESM::ContItem& content = list.at(subRowIndex);

            switch (subColIndex)
            {
                case 0: return QString::fromUtf8(content.mItem.toString().c_str());
                case 1: return content.mCount;
                default:
                    throw std::runtime_error("Trying to access non-existing column in the nested table!");
            }
        }

        virtual void setNestedData (const RefIdColumn *column,
                RefIdData& data, int row, const QVariant& value, int subRowIndex, int subColIndex) const
        {
            Record<ESXRecordT>& record =
                static_cast<Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (row, mType)));
            ESXRecordT container = record.get();
            std::vector<ESM::ContItem>& list = container.mInventory.mList;

            if (subRowIndex < 0 || subRowIndex >= static_cast<int> (list.size()))
                throw std::runtime_error ("index out of range");

            switch(subColIndex)
            {
                case 0:
                    list.at(subRowIndex).mItem.assign(std::string(value.toString().toUtf8().constData()));
                    break;

                case 1:
                    list.at(subRowIndex).mCount = value.toInt();
                    break;

                default:
                    throw std::runtime_error("Trying to access non-existing column in the nested table!");
            }

            record.setModified (container);
        }

        virtual int getNestedColumnsCount(const RefIdColumn *column, const RefIdData& data) const
        {
            return 2;
        }

        virtual int getNestedRowsCount(const RefIdColumn *column, const RefIdData& data, int index) const
        {
            const Record<ESXRecordT>& record =
                static_cast<const Record<ESXRecordT>&> (data.getRecord (RefIdData::LocalIndex (index, mType)));

            return static_cast<int>(record.get().mInventory.mList.size());
        }
    };
}

#endif
