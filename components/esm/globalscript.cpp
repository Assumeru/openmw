#include "globalscript.hpp"

#include "esmreader.hpp"
#include "esmwriter.hpp"

namespace
{
    class TargetVisitor : public boost::static_visitor<void>
    {
        ESM::ESMWriter& mWriter;

    public:
        TargetVisitor(ESM::ESMWriter &esm) : mWriter(esm) {}

        void operator()(const ESM::RefNum &refNum) const
        {
            if (refNum.hasContentFile())
                refNum.save (mWriter, false, "FRMR");
        }

        void operator()(const std::string &id) const
        {
            mWriter.writeHNOString ("TARG", id);
        }
    };
}

void ESM::GlobalScript::load (ESMReader &esm)
{
    mId = esm.getHNString ("NAME");

    mLocals.load (esm);

    mRunning = 0;
    esm.getHNOT (mRunning, "RUN_");

    if (esm.isNextSub("TARG"))
    {
        std::string target = esm.getHNString ("TARG");
        mTarget = target;
    }
    else
    {
        RefNum refNum;
        if (esm.isNextSub("FRMR"))
            refNum.load(esm, false, "FRMR");
        else
            refNum.unset();
        mTarget = refNum;
    }
}

void ESM::GlobalScript::save (ESMWriter &esm) const
{
    esm.writeHNString ("NAME", mId);

    mLocals.save (esm);

    if (mRunning)
        esm.writeHNT ("RUN_", mRunning);

    boost::apply_visitor (TargetVisitor(esm), mTarget);
}
